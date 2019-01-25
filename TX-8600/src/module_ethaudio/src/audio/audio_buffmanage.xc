#include "string.h"

#include "audio_buffmanage.h"
#include "eth_audio_config.h"
#include "gobal_val.h"
#include "eth_audio.h"
#include "adpcm.h"
#include "debug_print.h"

#include "src_fifo.h"

#include "audio_tx.h"


static struct
{
    uint8_t flag;
    uint16_t dst_ip[4];
    uint16_t dst_mask[4];
    uint8_t dst_mac[6];
}g_static_route;

static void debug_audio_devlist(eth_audio_dev_t audio_devlist[NUM_MEDIA_INPUTS], int num)
{
    debug_printf("\ndebug_audio_devlist\n");
    for(int i=0; i<num; i++) {
        if(audio_devlist[i].channel_num == 0) continue;
        debug_printf("[%d] IP:%d.%d.%d.%d  num:%d  media_list:", 
        i, audio_devlist[i].ip[0], audio_devlist[i].ip[1], audio_devlist[i].ip[2], audio_devlist[i].ip[3],audio_devlist[i].channel_num);
        for(int j=0; j<audio_devlist[i].channel_num; j++)
            debug_printf("[%d|%d]", audio_devlist[i].media_list[j].channel, audio_devlist[i].media_list[j].priority);
        debug_printf("\n\n");
    }
}
static void inset_media_list(media_info_t media_list[NUM_MEDIA_INPUTS], uint8_t &channel_num, media_info_t info)
{
    uint8_t i, j;

    // �׸�ͨ�������ֵ��ͬ, ���²��˳�
    if(media_list[0].channel && media_list[0].channel == info.channel) {
        media_list[0] = info;
        return;
    }
    // �����ͬͨ���ı�־λ 
    for(i=0; i<channel_num; i++) {
        if(media_list[i].channel == info.channel) {
            memmove(&media_list[i], &media_list[i+1], (channel_num-(i+1))*sizeof(media_info_t));
            channel_num--;
            break;// ȷ������û����ͬͨ��
        }
    }
    // ��ֵ����
    for(i=0; i<channel_num; i++) {
        if(media_list[i].priority <= info.priority) {
            for((NUM_MEDIA_INPUTS==channel_num)?(j=channel_num-1):(j=channel_num); j>i; j--) 
                media_list[j] = media_list[j-1];
            media_list[i] = info;
            channel_num++;
            break;
        }
    }
}

static void remove_media_list(media_info_t media_list[NUM_MEDIA_INPUTS], uint8_t &channel_num, uint8_t ch)
{
    // �����ͬͨ���ı�־λ 
    for(uint8_t i=0; i<channel_num; i++) {
        if(media_list[i].channel == ch) {
            memmove(&media_list[i], &media_list[i+1], (channel_num-(i+1))*sizeof(media_info_t));
            channel_num--;
            break;
        }
    }
}
//--------------------------------------------------------------------------------

void audio_buffmanage_process(client ethernet_cfg_if i_eth_cfg, int is_hp,
						 			 server ethaud_cfg_if i_ethaud_cfg[n_ethaud_cfg],
						  			 static const unsigned n_ethaud_cfg){
	//-----------------------------------------------------------------------
	// global val point get 
	//-----------------------------------------------------------------------
	volatile g_val_t *unsafe g_t_val;
	uint8_t i, j;
	
	// Reset global val point
	unsafe{
	uint32_t tmp;
	tmp = get_gval_point();
	g_t_val = (g_val_t *)tmp;
	//
	//g_t_val->audio_format= (SAMPLE_RATE_48K<<4)|AUDIOWIDTH_ADPCM;
	//g_t_val->audio_format= (SAMPLE_RATE_48K<<4)|AUDIOWIDTH_16BIT;
	g_t_val->audio_format= (SAMPLE_RATE_48K<<4)|AUDIOWIDTH_MP3;
	
	for(i=0; i<NUM_MEDIA_INPUTS; i++)
		g_t_val->audio_txvol[i] = MIX_VOL;

	//g_t_val->tx_flag = 0;
	//tx_flag = &g_t_val->tx_flag;
	}//unsafe


	// Add Eth TYPE Filter
	if(!is_hp) i_eth_cfg.add_ethertype_filter(0, 0x0800);   	//ETH Fun
	
	//-------------------------------------------------------------------------
	debug_printf("audio begin \n");
    memset(&g_static_route, 0, sizeof(g_static_route));
	//-------------------------------------------------------------------------
	// main loop
	//-------------------------------------------------------------------------
	unsafe{
	while(1){
		select{
			//======================================================================================================
			case i_ethaud_cfg[uint8_t a].set_audio_ethinfo(audio_ethinfo_t *t_audio_ethinfo):
				memcpy(g_t_val->macaddress,t_audio_ethinfo->macaddress,6);	
				memcpy(g_t_val->ipgate_macaddr,t_audio_ethinfo->ipgate_macaddr,6);
				memcpy(g_t_val->ipaddr,t_audio_ethinfo->ipaddr,4);
				memcpy(g_t_val->ipmask,t_audio_ethinfo->ipmask,4);
				memcpy(g_t_val->ipgate,t_audio_ethinfo->ipgate,4);
				//---------------------------------------------------------------------------------
				// Set Local Mac Address
				// i_eth_cfg.set_macaddr(0,g_t_val->macaddress);
				//---------------------------------------------------------------------------------
				// Add Eth Eth MAC Filter
#if 0
				{
				ethernet_macaddr_filter_t macaddr_filter;
				// Add Local MAC Address
				for(i=0;i<6;i++)
					macaddr_filter.addr[i]=g_t_val->macaddress[i];
				 i_eth_cfg.add_macaddr_filter(0, 0, macaddr_filter);
				}
#endif
				//---------------------------------------------------------------------------------		
				// Reset gobal val
				g_t_val->standby=1;
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_desip_infolist(audio_txlist_t *t_audio_txlist,uint8_t ch):
#if NEW_SEND_LIST_MODE_ENABLE
                timer tmr;
                uint32_t t1, t2;
                uint8_t priority = 0;
                media_info_t info;
                int audio_devlist_free_index = -1;
                tmr :> t1;
                //eth_audio_dev_t dev;
                for(i=0; i<t_audio_txlist->num_info; i++) { // ��ѵ�豸
                    audio_devlist_free_index = -1;
                    for(j=0; j<MAX_SENDCHAN_NUM; j++) {     // ��ѵ��Ƶ�豸�б�
                        // ��¼��һ�������豸
                        if(g_t_val->audio_devlist[j].channel_num==0 && audio_devlist_free_index==-1) {
                            audio_devlist_free_index = j;
                        }
                        // ������ͬIP���豸
                        if(memcmp(g_t_val->audio_devlist[j].ip, t_audio_txlist->t_des_info[i].ip, 4)==0) {
                            // ����MAC
                            if(!(ipaddr_maskcmp(g_t_val->ipaddr,g_t_val->audio_devlist[j].ip,g_t_val->ipmask)))
                                memcpy(g_t_val->audio_devlist[j].mac, g_t_val->ipgate_macaddr, 6);
                            else
                                memcpy(g_t_val->audio_devlist[j].mac, t_audio_txlist->t_des_info[i].mac, 6);

                            // ���ƽ���ͨ����Ϣ
                            info.channel = ch+1;
                            info.area_contorl = t_audio_txlist->t_des_info[i].area_contorl;
                            info.priority = priority;
                            
                            // ����ͨ����Ϣ: �����ͬͨ���ı�־λ ���ȼ��ж� ��ֵ���� 
                            inset_media_list(g_t_val->audio_devlist[j].media_list, g_t_val->audio_devlist[j].channel_num, info);
                            break;
                        }
                    }
                    // û���ҵ���ͬIP���豸, �����п����豸, ���и���
                    if(j==MAX_SENDCHAN_NUM && audio_devlist_free_index!=-1) {
                        j = audio_devlist_free_index;
                        // ����IP
                        memcpy(g_t_val->audio_devlist[j].ip, t_audio_txlist->t_des_info[i].ip, 4);
                        
                        // ����MAC
                        if(!(ipaddr_maskcmp(g_t_val->ipaddr,g_t_val->audio_devlist[j].ip,g_t_val->ipmask)))
                            memcpy(g_t_val->audio_devlist[j].mac, g_t_val->ipgate_macaddr, 6);
                        else
                            memcpy(g_t_val->audio_devlist[j].mac, t_audio_txlist->t_des_info[i].mac, 6);
                        
                        // ���ƽ���ͨ����Ϣ
                        g_t_val->audio_devlist[j].media_list[0].channel = ch+1;
                        g_t_val->audio_devlist[j].media_list[0].area_contorl = t_audio_txlist->t_des_info[i].area_contorl;
                        g_t_val->audio_devlist[j].media_list[0].priority = priority;
                        g_t_val->audio_devlist[j].channel_num++;
                    }
                }
                tmr :> t2;
                debug_audio_devlist(g_t_val->audio_devlist, MAX_SENDCHAN_NUM);
                debug_printf("audio_devlist tick %d\n", t2-t1);
#else
				memcpy(&g_t_val->t_audio_txlist[ch],t_audio_txlist,sizeof(audio_txlist_t));	
                for(i=0;i<g_t_val->t_audio_txlist[ch].num_info; i++){
                    if(g_static_route.flag&ipaddr_maskcmp(g_t_val->ipaddr, g_static_route.dst_ip, g_static_route.dst_mask)) {
                        /* Build an ethernet header. */
                        memcpy(g_t_val->t_audio_txlist[ch].t_des_info[i].mac, g_static_route.dst_mac, 6);
                    } else if(!(ipaddr_maskcmp(g_t_val->ipaddr,g_t_val->t_audio_txlist[ch].t_des_info[i].ip,g_t_val->ipmask)))
                        memcpy(g_t_val->t_audio_txlist[ch].t_des_info[i].mac,g_t_val->ipgate_macaddr,6);
                }
#endif
                g_t_val->sample_rate[ch] = 44100;
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_type(enum AUDIO_TYPE_E audio_type[NUM_MEDIA_INPUTS]):
				for(i=0; i<NUM_MEDIA_INPUTS; i++)
					g_t_val->audio_type[i] = audio_type[i];
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_txen(uint8_t audio_txen[NUM_MEDIA_INPUTS],unsigned timestamp[NUM_MEDIA_INPUTS]):
                for(i=0;i<NUM_MEDIA_INPUTS;i++){
#if NEW_SEND_LIST_MODE_ENABLE
                    if(g_t_val->audio_txen[i] && !audio_txen[i]) {
                        for(j=0; j<MAX_SENDCHAN_NUM; j++) {
                            if(g_t_val->audio_devlist[j].channel_num) {
                                // ����ͨ����Ϣ:�����ͬͨ���ı�־λ ǰ�� 
                                remove_media_list(g_t_val->audio_devlist[j].media_list, g_t_val->audio_devlist[j].channel_num, i+1);
                                // �������
                                if(g_t_val->audio_devlist[j].channel_num == 0) {
                                    memset(&g_t_val->audio_devlist[j], 0, sizeof(eth_audio_dev_t));
                                }
                            }
                        }
                    }
#endif                    
                    g_t_val->aux_timestamp[i] = timestamp[i];
                    g_t_val->audio_txen[i] = audio_txen[i];
                }
				debug_printf("set_audio_txen\n");
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_txvol(uint8_t audio_val[NUM_MEDIA_INPUTS]):
				//memcpy(g_t_val->audio_txvol,audio_val,NUM_MEDIA_INPUTS);
				for(i=0; i<NUM_MEDIA_INPUTS; i++)
					g_t_val->audio_txvol[i] = audio_val[i];
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_priolv(enum AUDIO_PRIOLV_E audio_priolv[NUM_MEDIA_INPUTS]):
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_silentlv(uint8_t audio_silentlv[NUM_MEDIA_INPUTS]):
				break;
            case i_ethaud_cfg[uint8_t a].set_static_route(uint8_t dst_ip[], uint8_t dst_mask[], uint8_t dst_mac[]):
                memcpy(g_static_route.dst_ip, dst_ip, 4);
                memcpy(g_static_route.dst_mask, dst_mask, 4);
                memcpy(g_static_route.dst_mac, dst_mac, 6);
                g_static_route.flag = 1;
                break;
#if 0
            case i_ethaud_cfg[unsigned a].send_text_en(uint8_t audio_txen[NUM_MEDIA_INPUTS],unsigned timestamp[NUM_MEDIA_INPUTS],
                                                                          unsigned max_send_page[NUM_MEDIA_INPUTS],
                                                                          unsigned have_send_num[NUM_MEDIA_INPUTS]):
                
                for(uint8_t i=0;i<NUM_MEDIA_INPUTS;i++){
                    g_t_val->audio_txen[i] = audio_txen[i];
                    g_t_val->send_text_en[i] = audio_txen[i];
                    g_t_val->aux_timestamp[i] = timestamp[i];
                    g_t_val->max_send_page[i] = max_send_page[i];
                    have_send_num[i] = g_t_val->have_send_num[i];
                    g_t_val->have_send_num[i] = 0; 
                }
                break;
#endif
        }//select
	} //while
	}//unsafe
}//main


