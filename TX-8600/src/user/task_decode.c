#include "task_decode.h"
#include "sys_config_dat.h"
#include "list_contorl.h"
#include "user_unti.h"
#include "fl_buff_decode.h"
#include "user_xccode.h"
#include "ack_build.h"
#include "protocol_adrbase.h"
#include "music_play.h"
#include "debug_print.h"
#include "user_messend.h"
#include "user_lcd.h"
#include "conn_process.h"
#include "sys_log.h"
#include "user_log.h"

extern uint8_t f_name[];

#ifndef null
#define null -1
#endif

//----------------------------------------------------------
// ��ʱ�����б��ʼ��
void task_fl_init(){
    g_tmp_union.task_allinfo_tmp.task_coninfo.task_id=0xFFFF;
    g_tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum=0x00;
    g_tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum = 0x00;
    for(uint16_t i=0; i<MAX_HOST_TASK; i++){
        fl_timertask_write(&g_tmp_union.task_allinfo_tmp,i);
    }
}
//---------------------------------------------------------
// ��ȡ��ʱ�����б������Ϣ
void timer_tasklist_read(){
    timetask_list.all_timetask_head=null;
    timetask_list.all_timetask_end=null;
    timetask_list.today_timetask_head=null;
    timetask_list.today_timetask_end=null;
    timetask_list.task_total=0;
    //
    for(uint16_t i=1; i<MAX_HOST_TASK; i++){
        timetask_list.timetask[i].all_next_p=null;
        timetask_list.timetask[i].today_next_p=null;
    }
    //
    for(uint16_t i=1;i<MAX_HOST_TASK;i++){
        fl_timertask_read(&g_tmp_union.task_allinfo_tmp,i);
        timetask_list.timetask[i].id = g_tmp_union.task_allinfo_tmp.task_coninfo.task_id;
        //�жϴ������� ��λ����        �жϷ���״̬
        if(timetask_list.timetask[i].id!=0xFFFF && 
           (timetask_list.timetask[i].id!=i || solution_list.solu_info[g_tmp_union.task_allinfo_tmp.task_coninfo.solution_sn].state==0xFF)
          ){
            //debug_printf("write task %d\n",timetask_list.timetask[i].id);
            g_tmp_union.task_allinfo_tmp.task_coninfo.task_id=0xFFFF;
            g_tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum=0x00;
            g_tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum = 0x00;
            //
            timetask_list.timetask[i].id = 0xFFFF;
            fl_timertask_write(&g_tmp_union.task_allinfo_tmp,i);
        }
        timetask_list.timetask[i].task_en = g_tmp_union.task_allinfo_tmp.task_coninfo.task_state;
        timetask_list.timetask[i].solu_id = g_tmp_union.task_allinfo_tmp.task_coninfo.solution_sn;
        timetask_list.timetask[i].repe_mode = g_tmp_union.task_allinfo_tmp.task_coninfo.task_repe_mode;
        timetask_list.timetask[i].time_info = g_tmp_union.task_allinfo_tmp.task_coninfo.time_info;
        timetask_list.timetask[i].week_repebit = g_tmp_union.task_allinfo_tmp.task_coninfo.week_repebit;
        //
        for(uint8_t j=0; j<MAX_TASK_DATE_NUM; j++)
            timetask_list.timetask[i].date_info[j] = g_tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[j];       
    }
    // ���������ʼ��
    for(uint8_t i=0;i<MAX_MUSIC_NUM;i++){
        timetask_now.ch_state[i] = 0xFF;
    }
    //       
}
//---------------------------------------------

//--------------------------------------------------------------------------------------------
// ���������б�����
void create_alltask_list(){
    for(uint16_t i=1; i<MAX_HOST_TASK; i++){
        if(timetask_list.timetask[i].id!=0xFFFF){
            // ���ӷ����ж�
            if((timetask_list.timetask[i].solu_id!=0xFF)&&(solution_list.solu_info[timetask_list.timetask[i].solu_id].state==0xFF)){
                timetask_list.timetask[i].id = 0xFFFF; //��λ����
                continue;
            }
            create_task_node_forid(timetask_list.timetask[i].id); //����һ������ڵ� 
        }
    }
    #if 0
    timetask_t *tmp_p;
    tmp_p = timetask_list.all_timetask_head;
    while(tmp_p!=null){
        xtcp_debug_printf("creat task id %d\n",tmp_p->id);
        tmp_p = tmp_p->all_next_p;
    }
    #endif
}

//---------------------------------------------------------------
// �ر���Ƶģ�鲥��
void task_music_config_stop(uint8_t ch){
    timetask_now.task_musicplay[ch].play_state=0;
    timetask_now.ch_state[ch] = 0xFF;
    task_music_stop(ch);
    user_disptask_refresh();
}

void task_music_stop_all(){
    for(uint8_t i=0; i<MAX_MUSIC_CH;i++){
        if(timetask_now.ch_state[i]!=0xFF){
            task_music_config_stop(i);
        }
    }
}

//-------------------------------------------------------------------------------------------
// ��Ŀ�¼��л�
uint8_t random_music_tab[MAX_RTMUSIC_NUM] = {17,8,3,0,16,12,19,15,4,18,7,13,2,6,1,10,5,9,11,14,25,49,46,33,32,21,48,47,29,43,30,22,23,31,37,
                                             26,34,44,24,45,39,42,36,38,20,35,41,28,40,27};
uint8_t random_inc[MAX_MUSIC_CH] = {0};

uint8_t random_play_inc(uint8_t ch,uint8_t mustol){
    for(uint8_t i=0;i<mustol;i++){
        random_inc[ch]++;
        if(random_inc[ch]>=mustol )
            random_inc[ch] =0;
        if(random_music_tab[random_inc[ch]] < timetask_now.task_musicplay[ch].music_tol){
            timetask_now.task_musicplay[ch].music_inc = random_music_tab[random_inc[ch]];
            return random_music_tab[random_inc[ch]];
        } 
    }
    return 0;
}

// ���ֲ����л�
void task_musicevent_change(uint8_t ch,char event,char data,uint8_t set_musinc_f){
    uint16_t id_tmp;
    task_music_info_t *p_music_info;
    static uint8_t music_inc[MAX_MUSIC_CH] = {0};
    //
    if(timetask_now.ch_state[ch]==0xFF)
        return;
    // �������ֳ���һ���������ر�����
    if(g_sys_val.play_error_inc[ch]>=timetask_now.task_musicplay[ch].music_tol){
        timetask_now.ch_state[ch]=0xFF;
        user_audio_send_dis(ch);
        user_disptask_refresh();
        return;
    }    
    // ָ����Ŀ
    if(set_musinc_f){
        music_inc[ch] = timetask_now.task_musicplay[ch].music_inc;
        goto plya_music_begin;
    }
    // ��һ����Ŀ
    if(timetask_now.task_musicplay[ch].play_mode!=ONCE_PLAY_M && timetask_now.task_musicplay[ch].play_mode!=ONCE_LOOPPLAY_M)
        timetask_now.task_musicplay[ch].music_inc++;
    //timetask_now.task_musicplay[ch].play_mode = LOOP_PLAY_M;
    //xtcp_debug_printf("play mode %d\n", timetask_now.task_musicplay[ch].play_mode);
    // �жϲ���ģʽ ˳��/ѭ��/��� ����
    switch(timetask_now.task_musicplay[ch].play_mode){
        // ˳�򲥷�
        case ORDER_PLAY_M:
            if(timetask_now.task_musicplay[ch].music_inc == timetask_now.task_musicplay[ch].music_tol){
                //��ʱ����ֹͣ����
                if(timetask_now.task_musicplay[ch].rttask_f){
                    timetask_now.task_musicplay[ch].music_inc=0;
                    if(data==0){
                        user_playstate_set(0,ch);                
                        timetask_now.task_musicplay[ch].play_state=0;
                    }
                }else{
                    timetask_now.ch_state[ch] = 0xFF;
                    user_audio_send_dis(ch);
                    user_disptask_refresh();
                    return;
                }
                //task_music_config_stop(ch);
            }
            // ������ֱ��
            music_inc[ch] = timetask_now.task_musicplay[ch].music_inc;
            //xtcp_debug_printf("play mode %d  music num %d\n",timetask_now.task_musicplay[ch].play_mode,
            //                                            music_inc[ch]);
            break;
        // ѭ������
        case LOOP_PLAY_M:
            //xtcp_debug_printf("loop mic tol %d  mic inc %d ch%d\n",timetask_now.task_musicplay[ch].music_tol,timetask_now.task_musicplay[ch].music_inc,ch);
            if(timetask_now.task_musicplay[ch].music_inc == timetask_now.task_musicplay[ch].music_tol){
                timetask_now.task_musicplay[ch].music_inc=0;
            }            
            // ������ֱ��
            music_inc[ch] = timetask_now.task_musicplay[ch].music_inc;
            break;
        // �������
        case RANDOM_PLAY_M:
            // ������ֱ��
            if(timetask_now.task_musicplay[ch].rttask_f){
                music_inc[ch] = random_play_inc(ch,MAX_RTMUSIC_NUM);
            }
            else{
                music_inc[ch] = random_play_inc(ch,MAX_MUSIC_CH);
            }
            break;
        // ��������
        case ONCE_PLAY_M:
            music_inc[ch] = timetask_now.task_musicplay[ch].music_inc;
            //��ʱ����ֹͣ����
            if(timetask_now.task_musicplay[ch].rttask_f){
                timetask_now.task_musicplay[ch].music_inc=0;
                if(data==0){
                    user_playstate_set(0,ch);                
                    timetask_now.task_musicplay[ch].play_state=0;
                }
            }else{
                timetask_now.ch_state[ch] = 0xFF;
                user_audio_send_dis(ch);
                user_disptask_refresh();
                return;
            }
            break;
        // ����ѭ��
        case ONCE_LOOPPLAY_M:
            music_inc[ch] = timetask_now.task_musicplay[ch].music_inc;
            break;
    }
    plya_music_begin:
    
    //xtcp_debug_printf("play inc m %d c%d %d %d\n",timetask_now.task_musicplay[ch].play_mode,timetask_now.task_musicplay[ch].music_inc,music_inc[ch],timetask_now.task_musicplay[ch].rttask_f);
    if(timetask_now.task_musicplay[ch].rttask_f){
        // ��ȡ��ʱ����
        fl_rttask_read(&g_tmp_union.rttask_dtinfo,timetask_now.task_musicplay[ch].task_id);
        id_tmp = g_tmp_union.rttask_dtinfo.rttask_id;   
        xtcp_debug_printf("play inc %d\n",music_inc[ch]);
        p_music_info = &g_tmp_union.rttask_dtinfo.music_info[music_inc[ch]];   
    }else{
        // ��ȡ��ʱ����
        fl_timertask_read(&g_tmp_union.task_allinfo_tmp,timetask_now.task_musicplay[ch].task_id);        
        id_tmp = g_tmp_union.task_allinfo_tmp.task_coninfo.task_id;
        p_music_info = &g_tmp_union.task_allinfo_tmp.task_musiclist.music_info[music_inc[ch]];
    }
    if(id_tmp!=0xFFFF)
        // ������һ������
        task_music_play(ch,music_inc[ch],p_music_info);
    else{
        // �������ǿյģ��ر����ֲ���
        timetask_now.ch_state[ch] = 0xFF;
        user_audio_send_dis(ch);
        user_disptask_refresh();
    }
}

//-------------------------------------------------------------------------------------------
// 1Shz ���ż�ʱ��ѯ
void timer_taskmusic_check(){
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(timetask_now.ch_state[i]!=0xFF && timetask_now.task_musicplay[i].dura_time!=0xFFFF){
            timetask_now.task_musicplay[i].time_inc++;
            if(timetask_now.task_musicplay[i].time_inc>=timetask_now.task_musicplay[i].dura_time){
                task_music_config_stop(i);
                user_disptask_refresh();
                // ��ʱ����ر�
                if(timetask_now.task_musicplay[i].rttask_f){
                    ; // ��ʱ����رղ������ﴦ������ֹֻͣ����
                }       
                // ��ʱ����ر�
                else{ 
                    // ������Ϣ����
                	g_sys_val.task_config_s = 2; //����༭
                	g_sys_val.task_con_id = timetask_now.task_musicplay[i].task_id;
                    mes_send_taskinfo_nopage(&g_tmp_union.task_allinfo_tmp);
                }
            }
        }
    }
}
//---------------------------------------------------------------------------------------------------------------
// ���÷���Ŀ�����������
void task_music_config_play(uint8_t ch,uint16_t id,uint8_t rttask_f,uint8_t set_musicinc,uint8_t set_vol){
    g_sys_val.play_error_inc[ch] = 0;
    // ��ͨ������״̬
    timetask_now.ch_state[ch]=ch;
    // ��ʼ����������״̬1
    timetask_now.task_musicplay[ch].task_id = id;
    timetask_now.task_musicplay[ch].time_inc = 0;
    if(set_musicinc==0)
        timetask_now.task_musicplay[ch].music_inc = 0;
    timetask_now.task_musicplay[ch].rttask_f = rttask_f;
    // ���ģʽ����
    uint8_t tmp_inc;
    tmp_inc = timetask_now.task_musicplay[ch].music_inc;
    if(timetask_now.task_musicplay[ch].play_mode == RANDOM_PLAY_M){
        random_inc[ch] = g_sys_val.time_info.second%20;
        tmp_inc = random_play_inc(ch,MAX_MUSIC_CH);
    }
    // ��ʼ����������״̬2

    // ��ʱ����
    if(rttask_f){
        fl_rttask_read(&g_tmp_union.rttask_dtinfo,id);
        
        // û�и��������ֲ�����
        if(g_tmp_union.rttask_dtinfo.music_tol==0){
            timetask_now.ch_state[ch]=0xFF;
            return;
        }
        xtcp_debug_printf("rt task play id %d,%d\n",id,ch);
        timetask_now.task_musicplay[ch].dura_time = g_tmp_union.rttask_dtinfo.dura_time; //��ó���ʱ��
        timetask_now.task_musicplay[ch].play_mode = g_tmp_union.rttask_dtinfo.play_mode; //��ò���ģʽ
        timetask_now.task_musicplay[ch].music_tol = g_tmp_union.rttask_dtinfo.music_tol;   //�������� 
        if(set_vol==0){
            timetask_now.task_musicplay[ch].task_vol = g_tmp_union.rttask_dtinfo.task_vol;
        }
        timetask_now.task_musicplay[ch].rttask_f=1;
        //
        timetask_now.task_musicplay[ch].sulo_id = D_RTTASK_SOULID;
        //memcpy(timetask_now.task_musicplay[ch].name,g_tmp_union.rttask_dtinfo.name,DIV_NAME_NUM);
        // ��������,ʹ�ܲ����б�
        task_music_send(ch,
                        g_tmp_union.rttask_dtinfo.des_info,
                        g_tmp_union.rttask_dtinfo.div_tol,
                        g_tmp_union.rttask_dtinfo.prio,
                        timetask_now.task_musicplay[ch].task_vol);

        task_music_play(ch,tmp_inc,&g_tmp_union.rttask_dtinfo.music_info[timetask_now.task_musicplay[ch].music_inc]);
        
        user_disptask_refresh();
    }
    // ��ʱ����
    else{
        fl_timertask_read(&g_tmp_union.task_allinfo_tmp,id);
        
        timetask_now.task_musicplay[ch].dura_time = g_tmp_union.task_allinfo_tmp.task_coninfo.dura_time; //��ó���ʱ��
        timetask_now.task_musicplay[ch].play_mode = g_tmp_union.task_allinfo_tmp.task_coninfo.play_mode; //��ò���ģʽ
        timetask_now.task_musicplay[ch].music_tol = g_tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum;   //�������� 
        timetask_now.task_musicplay[ch].sulo_id = g_tmp_union.task_allinfo_tmp.task_coninfo.solution_sn;
        timetask_now.task_musicplay[ch].rttask_f=0;
        //memcpy(timetask_now.task_musicplay[ch].name,g_tmp_union.task_allinfo_tmp.task_coninfo.task_name,DIV_NAME_NUM);
        // ��������,ʹ�ܲ����б�
        task_music_send(ch,
                        g_tmp_union.task_allinfo_tmp.task_maclist.taskmac_info,
                        g_tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum,
                        g_tmp_union.task_allinfo_tmp.task_coninfo.task_prio,
                        g_tmp_union.task_allinfo_tmp.task_coninfo.task_vol);

        task_music_play(ch,tmp_inc,&g_tmp_union.task_allinfo_tmp.task_musiclist.music_info[timetask_now.task_musicplay[ch].music_inc]);
        user_disptask_refresh();
    }
}

//-------------------------------------------------------------------------------------------
// ��ʱ/���� ����ʱ����ѯ�벥�� 
void task_check_and_play(){
    uint8_t have_task_playing=0;
    for(uint8_t i=0;i<MAX_HOST_TASK;i++){
        if(timetask_list.today_timetask_head==null)
            break;
        // ʱ�䵽 �ж����񲥷�
        if((timetask_list.today_timetask_head->time_info.hour == g_sys_val.time_info.hour)&&
           (timetask_list.today_timetask_head->time_info.minute == g_sys_val.time_info.minute)&&
           (timetask_list.today_timetask_head->time_info.second == g_sys_val.time_info.second)
        ){
            //--------------------------------------------------------------
            // �����Ƿ��������Ѿ��ڲ���
            for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
                if(g_sys_val.task_wait_state[i]!=0){
                    if(timetask_now.task_musicplay[i].task_id==timetask_list.today_timetask_head->id && timetask_now.task_musicplay[i].rttask_f==0){
                        goto next_task_play;
                    }
                }
            }            
            // �ҿ�����λ�ò���
            uint8_t play_num;
            for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
                if(g_sys_val.task_wait_state[i]==0){
                    play_num = i;
                    break;
                }
            }
            // �����ļ�ʱ�ɲ������� 
            //if(g_sys_val.file_bat_contorl_s==0){
                g_sys_val.music_task_id[play_num] = timetask_list.today_timetask_head->id;
                g_sys_val.task_wait_state[play_num] = 1;
                g_sys_val.play_rttask_f[play_num] = 0; // ��ʱ���񲥷�
                g_sys_val.play_ok = 1;
            //}
            //--------------------------------------------------------------
            // �л�   ��һ����������
            next_task_play:
            timetask_list.today_timetask_head = timetask_list.today_timetask_head->today_next_p;
            //if(g_sys_val.file_bat_contorl_s){
                user_disptask_refresh();
            //}
            continue;
        }
        // û������ִ��
        break;
    }//for
}

void task_10hz_mutich_play(){
    if(!g_sys_val.play_ok)
        return;
    g_sys_val.play_ok = 0;
    //
    for(uint8_t j=0;j<MAX_MUSIC_CH;j++){
        if(g_sys_val.task_wait_state[j]){
            g_sys_val.task_wait_state[j] = 0;
            // SD���γ� ����������
            if(g_sys_val.sd_state) continue;
            // ɨ��ò��ŵ�����
            for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
                if(timetask_now.ch_state[i]==0xFF){
                    xtcp_debug_printf("play task:%d\n",g_sys_val.music_task_id[j]);
                    //task_music_config_play(i,g_sys_val.music_task_id[j]);
                    task_music_config_play(i,g_sys_val.music_task_id[j],g_sys_val.play_rttask_f[j],0,0);
                    // ������Ϣ����
                    if(g_sys_val.play_rttask_f[j]==0){
                    	g_sys_val.task_config_s = 2; //����༭
                    	g_sys_val.task_con_id = g_sys_val.music_task_id[j];
                        mes_send_taskinfo_nopage(&g_tmp_union.task_allinfo_tmp);
                    }
                    //--------------------------------------------------------------------------------
                    break;
                }
            }
            break;
        }
    }
}


//-------------------------------------------------------------------------------------------
// ���������б�����
void create_todaytask_list(time_info_t time_info){
    timetask_t *task_p;
    timetask_t *today_t_p;
    timetask_t *last_today_p;
    unsigned beg_time;
    unsigned task_time;
    unsigned time_tmp;
    uint8_t tmp_f;
    //
    beg_time = (time_info.hour<<16)|(time_info.minute<<8)|time_info.second;
    task_p = timetask_list.all_timetask_head;
    timetask_list.today_timetask_head = null;
    //
    while(task_p!=null){
        tmp_f=0;
        if(task_p->repe_mode){//�ж�����
            for(uint8_t i=0;i<MAX_TASK_DATE_NUM;i++){
                #if 0
                xtcp_debug_printf("year %d %d month %d %d day %d %d \n", task_p->date_info[i].year,g_sys_val.date_info.year,
                                                                    task_p->date_info[i].month,g_sys_val.date_info.month,
                                                                    task_p->date_info[i].date,g_sys_val.date_info.date);
                #endif
                if((task_p->date_info[i].date==g_sys_val.today_date.date)&&
                   (task_p->date_info[i].month==g_sys_val.today_date.month)&&
                   (task_p->date_info[i].year==g_sys_val.today_date.year)){
                        tmp_f=1;
                        break;
                   }
            }
        }
        else if((task_p->week_repebit>>(g_sys_val.today_date.week-1))&0x01){ //�ж�����
            tmp_f=1;
        }
        // �жϷ���������ʹ�� 
        if((!task_p->task_en)||((task_p->solu_id!=0xFF)&&(!solution_list.solu_info[task_p->solu_id].data_en))){
            tmp_f=0;
        }
        task_time = (task_p->time_info.hour<<16)|(task_p->time_info.minute<<8)|task_p->time_info.second; //������
        //
        // �����������ҳ��������� ������������������������
        //if(tmp_f)
        //    xtcp_debug_printf("id %d,taskt %x,begt %x tasken %d  soluid %d soluen %d day mode %d today %d\n",task_p->id,task_time,beg_time,task_p->task_en,task_p->solu_id,solution_list.solu_info[task_p->solu_id].en,task_p->week_repebit,tmp_f);
        //for(uint8_t i=0;i<10;i++){
        //    xtcp_debug_printf("data %d,%d,%d\n",task_p->date_info[i].date,task_p->date_info[i].month,task_p->date_info[i].year);
        //}
        if((tmp_f)&&(task_time >= beg_time)){//�ж�ʱ��
            //xtcp_debug_printf("today time right\n");
            task_p->today_next_p=null;
            today_t_p = timetask_list.today_timetask_head;
            //��ȡ��һ������
            if(today_t_p==null){    
                timetask_list.today_timetask_head = task_p; 
                today_t_p = task_p;
            }
            else{
                while(today_t_p!=null){
                    time_tmp = (today_t_p->time_info.hour<<16)|(today_t_p->time_info.minute<<8)|today_t_p->time_info.second;
                    if(task_time<time_tmp){  //�������� ʱ��Ƚ�
                        if(today_t_p==timetask_list.today_timetask_head){ //�嵽��һ������
                            timetask_list.today_timetask_head = task_p; //����ͷ����
                            task_p->today_next_p = today_t_p;
                        }
                        else{ //�嵽�м�
                            last_today_p->today_next_p = task_p;
                            task_p->today_next_p = today_t_p;
                        }
                        break;//�˳�while����
                    }
                    last_today_p = today_t_p;
                    if(today_t_p->today_next_p == null){ // β������
                        today_t_p->today_next_p = task_p;
                        break;
                    }
                    today_t_p = today_t_p->today_next_p;
                }
            }
        }
        task_p = task_p->all_next_p;    
    }
    #if 1
    timetask_t *tmp_p = timetask_list.today_timetask_head;
    while(tmp_p!=null){
        xtcp_debug_printf("today task id %d\n",tmp_p->id);
        tmp_p = tmp_p->today_next_p;
    }
    #endif
    //-------------------------------------------------------------------------------------
    // ��ʼ����������
    task_check_and_play();
    user_disptask_refresh();
}

//===================================================================================================
// return 1 ʧ�� 0 �ɹ� 
uint8_t tasktime_conflictcmp(uint32_t src_begtime,uint32_t src_endtime,uint32_t begtime,uint32_t endtime){
    // 24Сʱ��
    if(begtime<src_begtime && endtime<=src_begtime){
        ;//�������ж�
    }
    else if(begtime>=src_endtime){
        return 0;// ���ڳ�ʱ��Χ
    }
    else{
        return 1;//��ͻ
    }
    // ����
    begtime+=24*3600;
    if(begtime>src_endtime){
        return 0;
    }
    // ���ճ�ͻ
    return 1;
}

// return 1 ʱ���ͻ 0 ����
uint8_t tasktime_decode(uint8_t hour,uint8_t minute,uint8_t second,uint32_t dura_time,uint8_t tasksolu_id,task_dateinfo_t dateinfo[]){
    unsigned next_tasktime,end_tasktime,beg_time,end_time;
    uint8_t over_state_inc=0;
    //
    timetask_t *task_p;
    taskconflict_info_s *p_taskconflict_info;
    // ʹ�ù���buff ���ʱ���ͻ�ṹ��
    p_taskconflict_info = (taskconflict_info_s *)(&g_tmp_union.buff[4*1024]);
    // ��ʼ���ṹ��
    memset(p_taskconflict_info,0xFF,sizeof(taskconflict_info_s));
    // ����ʱ���ʼ��
    beg_time = hour*3600+minute*60+second;
    end_time = beg_time+dura_time;    
    // 
    task_p = timetask_list.all_timetask_head;
    while(task_p!=null){
        fl_timertask_read(&g_tmp_union.task_allinfo_tmp,task_p->id);
        next_tasktime = (g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour*3600)+
                        (g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute*60)+
                         g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.second;
        end_tasktime = next_tasktime+g_tmp_union.task_allinfo_tmp.task_coninfo.dura_time;
        // ���ڳ�ͻ
        uint8_t data_right_flag=0;
        if(tasksolu_id!=0xFF){
            data_right_flag=1;
        }
        else{
            for(uint8_t i=0;i<MAX_TASK_DATE_NUM;i++){
                if(dateinfo[i].date==0xff)
                    continue;
                if((dateinfo[i].date==g_tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i].date && 
                    dateinfo[i].month==g_tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i].month &&
                    dateinfo[i].year==g_tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i].year) && 
                    g_tmp_union.task_allinfo_tmp.task_coninfo.solution_sn==0xFF && task_p->task_en){
                    data_right_flag=1;
                    /*
                    debug_printf("have data same %d %d m %d %d y %d %d\n",dateinfo[i].date,g_tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i].date,
                                dateinfo[i].month,g_tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i].month,
                                dateinfo[i].year,g_tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i].year);
                    */
                }
            }    
        }
        // ����������ɸѡ
        if((task_p->id!=g_sys_val.task_con_id)&&(tasksolu_id==task_p->solu_id)&&(task_p->task_en) && data_right_flag){
            // ������ʱ���� �༶�����ͻ�ж�
            if(tasktime_conflictcmp(beg_time,end_time,next_tasktime,end_tasktime)){
                uint8_t info_p;
                for(uint8_t i=0;i<SOLU_MAX_PLAYCH;i++){
                    //-------------------------------------------------------------------------------
                    // ��ʼ��ÿ��״̬
                    if(p_taskconflict_info->state_bg[i]==0xFF){
                        over_state_inc++;   // ��ͻ������1
                        // �ҿ�λ
                        for(uint8_t j=0;j<MAX_SOUL_HAVETASK;j++){
                            if(p_taskconflict_info->state[j]==0xFF){
                                p_taskconflict_info->state[j]=i; // ��ֵ��ǰ����
                                p_taskconflict_info->state_bg[i] = j; // ��ֵ��ǰ��ͷָ��
                                p_taskconflict_info->bt[j] = next_tasktime; 
                                p_taskconflict_info->et[j] = end_tasktime;
                                break;
                            }
                        }
                        // �ҵ���ǰ�ȼ�λ�� �˳�������ȼ���ͻ����
                        break;
                    }
                    //-------------------------------------------------------------------------------
                    // ͬ����ͻ�ж�
                    else{
                        uint8_t conflict_flag=0;
                        // ��ȡͷָ��
                        info_p = p_taskconflict_info->state_bg[i];
                        //-------------------------------------------------------------------
                        // ���������ͻ�ж�
                        while(1){
                            if(tasktime_conflictcmp(p_taskconflict_info->bt[info_p],p_taskconflict_info->et[info_p],next_tasktime,end_tasktime)){
                                conflict_flag=1;
                                break;//�˳��������� ������һ����ͻ�ж�
                            }
                            if(p_taskconflict_info->next_t[info_p]==0xFF){
                                //�����������ͻ      ����ͬ������ʱ��
                                for(uint8_t j=0;j<MAX_SOUL_HAVETASK;j++){ //�ҿ����񱣴�
                                    if(p_taskconflict_info->state[j]==0xFF){
                                        p_taskconflict_info->state[j]=i; // ��ֵ��ǰ����
                                        p_taskconflict_info->bt[j] = next_tasktime; 
                                        p_taskconflict_info->et[j] = end_tasktime;
                                        p_taskconflict_info->next_t[info_p]==j; //�½���
                                        break;
                                    }
                                }
                                break;// �˳����������ͻ�ж�
                            }
                            info_p = p_taskconflict_info->next_t[info_p]; // �¸�ͬ������ʱ���ж�
                        }
                        //-------------------------------------------------------------------
                        if(conflict_flag==0){
                            break; //�����������ͻ �˳����¸�����
                        }
                    }
                }
                if(over_state_inc>(SOLU_MAX_PLAYCH-1)){
                    xtcp_debug_printf("task time error\n",over_state_inc);
                    return 1;
                }
            }
        }
        task_p = task_p->all_next_p;
    }
    return 0;
}
//====================================================================================================
// ������ѯ
//====================================================================================================
void solution_check_recive(){
    user_sending_len = solution_list_ack_build(SOLUTION_CHECK_CMD,0);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    //xtcp_debug_printf("solution check\n");
}

//====================================================================================================
// �����б��ѯ
//====================================================================================================
void task_check_recive(){
    uint8_t list_num = list_sending_init(TASK_CHECK_CMD,TASK_LIST_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num == LIST_SEND_INIT)
        return;
    // ���⴦��
    t_list_connsend[list_num].list_info.tasklist.solu_en=0; //������������
    t_list_connsend[list_num].list_info.tasklist.solu_id=0;
    //
    t_list_connsend[list_num].list_info.tasklist.task_tol = timetask_list.task_total/MAX_TASK_ONCESEND;
    if(timetask_list.task_total%MAX_TASK_ONCESEND || t_list_connsend[list_num].list_info.tasklist.task_tol==0)
        t_list_connsend[list_num].list_info.tasklist.task_tol++;
    //
	if(g_sys_val.list_sending_f==0){
		g_sys_val.list_sending_f = 1;
	    user_sending_len = task_list_ack_build(TASK_CHECK_CMD,0,0,list_num);
	    user_xtcp_send(conn,t_list_connsend[list_num].could_s);
	}
}
//
//--------------------------------------------
// �����б���������
void tasklist_sending_decode(uint8_t list_num){
    user_sending_len = task_list_ack_build(t_list_connsend[list_num].list_info.tasklist.cmd,
                                           t_list_connsend[list_num].list_info.tasklist.solu_en,
                                           t_list_connsend[list_num].list_info.tasklist.solu_id,
                                           list_num);
    user_xtcp_send(t_list_connsend[list_num].conn,t_list_connsend[list_num].could_s);
    //xtcp_debug_printf("tasklist_sending\n");
}

//====================================================================================================
// ��ȡ��ʱ������ϸ��ϢЭ�� B304
//====================================================================================================
void task_dtinfo_check_recive(){
    uint8_t list_num = list_sending_init(TASK_DTINFO_CK_CMD,TASK_DTINFO_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num == LIST_SEND_INIT)
        return;
    //
    uint16_t id = xtcp_rx_buf[TASK_DTG_TASK_ID+1]<<8 |xtcp_rx_buf[TASK_DTG_TASK_ID];
    if(id>MAX_HOST_TASK)
        return;
    //
	if(g_sys_val.list_sending_f==0){
		g_sys_val.list_sending_f = 1;
	    fl_timertask_read(&g_tmp_union.task_allinfo_tmp,id);
	    //
	    user_sending_len = task_dtinfo_chk_build(list_num);
	    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
	}
    //xtcp_debug_printf("dtchk task %d\n",id);
}

//----------------------------------------------
// ��ϸ��Ϣ��������
void task_dtinfo_decode(uint8_t list_num){
    fl_timertask_read(&g_tmp_union.task_allinfo_tmp,t_list_connsend[list_num].list_info.task_dtinfo.task_id);
    user_sending_len = task_dtinfo_chk_build(list_num);
    user_xtcp_send(t_list_connsend[list_num].conn,t_list_connsend[list_num].could_s);
    //xtcp_debug_printf("task dtinfo send\n");
}

//====================================================================================================
// ���������ж�
// @solu_mode :0  �ж��Ƿ������� 
//            :1  �жϿ�����رշ���
//====================================================================================================
void solution_data_chk(uint8_t id){
    //
    unsigned solu_begdate = (solution_list.solu_info[id].begin_date.year<<16)|
                            (solution_list.solu_info[id].begin_date.month<<8)|
                            (solution_list.solu_info[id].begin_date.date);
    //
    unsigned solu_enddate = (solution_list.solu_info[id].end_date.year<<16)|
                            (solution_list.solu_info[id].end_date.month<<8)|
                            (solution_list.solu_info[id].end_date.date);
    //
    unsigned sys_date = (g_sys_val.date_info.year<<16)|
                        (g_sys_val.date_info.month<<8)|
                        (g_sys_val.date_info.date);
    
    solution_list.solu_info[id].data_en = 0;
    if((solution_list.solu_info[id].state!=0xFF)&&(solution_list.solu_info[id].en)){
        if((solu_begdate<=sys_date)&&(solu_enddate>=sys_date)){
            solution_list.solu_info[id].data_en = 1;
        }
    }
}


//====================================================================================================
// ���ö�ʱ������ϢЭ��   B305
//====================================================================================================
void solution_config_recive(){
    uint8_t state=0;
    //��ȡid
    uint8_t id = xtcp_rx_buf[SOLU_CFG_SOLU_ID];
    //xtcp_debug_printf("solution config %d\n",id);
    //��������
    if((xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL]==0)||(xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL]==3)){
        id = 0xFF;
        for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
            if(solution_list.solu_info[i].state==0xFF){
                solution_list.solu_info[i].state=i; 
                solution_list.solu_info[i].id=i; 
                xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT] = 0xFF;
                id = i;
                break;
            }
        }
    }
    if((id>MAX_TASK_SOULTION)){     
        state = 1; //��������
        goto solution_config_end;
    }
    //ɾ������
    if(xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL]==1){
        // ɾ������
        solution_list.solu_info[id].state=0xFF; 
        xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT] = 0xFF;
        // ɾ�����������������
        timetask_t *task_p;
        uint16_t tmp_id;
        task_p = timetask_list.all_timetask_head;
        // ɨ����������
        while(task_p!=null){
            if(task_p->solu_id == id){
                tmp_id = task_p->id;
                task_p = task_p->all_next_p;
                fl_timertask_read(&g_tmp_union.task_allinfo_tmp,tmp_id);
                delete_task_node(tmp_id);
                timetask_list.timetask[tmp_id].id=0xFFFF;
                g_tmp_union.task_allinfo_tmp.task_coninfo.task_id =0xFFFF;
                fl_timertask_write(&g_tmp_union.task_allinfo_tmp,tmp_id);
                continue;
            }
            task_p = task_p->all_next_p;
        }
    }
    timetask_t *task_p;
    timetask_t *task_end_p;
    //��¡����
    if(xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL]==3){
        //�����������ҳ���ǰ��������
        task_p = timetask_list.all_timetask_head;
        task_end_p = timetask_list.all_timetask_end;
        while((task_p!=null)){
            // �жϿ�¡����
            if(task_p->solu_id == xtcp_rx_buf[SOLU_CFG_SOLU_ID]){
                // ��ȡ���¡������
                fl_timertask_read(&g_tmp_union.task_allinfo_tmp,task_p->id);
                // �������񲢿�¡
                if(create_task_node()){ //������ӳɹ�   
                    g_tmp_union.task_allinfo_tmp.task_coninfo.task_id = timetask_list.all_timetask_end->id;
                    g_tmp_union.task_allinfo_tmp.task_coninfo.solution_sn = id; 
                    timetask_list.timetask[timetask_list.all_timetask_end->id].solu_id = id;
                    timetask_list.timetask[timetask_list.all_timetask_end->id].task_en = g_tmp_union.task_allinfo_tmp.task_coninfo.task_state;
                    timetask_list.timetask[timetask_list.all_timetask_end->id].week_repebit = g_tmp_union.task_allinfo_tmp.task_coninfo.week_repebit;
                    timetask_list.timetask[timetask_list.all_timetask_end->id].repe_mode = g_tmp_union.task_allinfo_tmp.task_coninfo.task_repe_mode;
                    memcpy(timetask_list.timetask[timetask_list.all_timetask_end->id].date_info,g_tmp_union.task_allinfo_tmp.task_coninfo.dateinfo,3*MAX_TASK_DATE_NUM);
                    timetask_list.timetask[timetask_list.all_timetask_end->id].time_info = g_tmp_union.task_allinfo_tmp.task_coninfo.time_info;
                    //
                    fl_timertask_write(&g_tmp_union.task_allinfo_tmp,timetask_list.all_timetask_end->id);
                }
                else{ // �������� ʧ��
                    state = 2;
                    break;
                }
            }
            if((task_p==task_end_p)){
                break;
            }
            task_p = task_p->all_next_p;
        }
    }
    // ��������
    //xtcp_debug_printf("con solu %x\n",xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT]);
    //
    //���÷�������
    if((xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT]>>1)&1)
        memcpy(&solution_list.solu_info[id].name,&xtcp_rx_buf[SOLU_CFG_SOLU_NAME],DIV_NAME_NUM);
    //���÷�������
    if((xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT]>>2)&1){
        memcpy(&solution_list.solu_info[id].begin_date,&xtcp_rx_buf[SOLU_CFG_SOLU_BEGDATE],3);
        memcpy(&solution_list.solu_info[id].end_date,&xtcp_rx_buf[SOLU_CFG_SOLU_ENDDATE],3);
    } 
    // ���÷������ȼ�
    solution_list.solu_info[id].prio = xtcp_rx_buf[SOLU_CFG_SOLU_PRIO]; 
    // �ı��������ȼ�
    
    task_p = timetask_list.all_timetask_head;
    while(task_p!=null){
        if(task_p->solu_id == id){
            fl_timertask_read(&g_tmp_union.task_allinfo_tmp,task_p->id);
            g_tmp_union.task_allinfo_tmp.task_coninfo.task_prio = solution_list.solu_info[id].prio;
            fl_timertask_write(&g_tmp_union.task_allinfo_tmp,task_p->id);
        }
        task_p = task_p->all_next_p;
    }

    // ���÷���ʹ��
    if(xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT]&1){ 
        if(xtcp_rx_buf[SOLU_CFG_SOLU_STATE]){
            uint8_t max_open_solu=0;
            for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
                if((solution_list.solu_info[i].state!=0xFF)&&(solution_list.solu_info[i].en))
                    max_open_solu++;
            }
            if(max_open_solu<2)
                solution_list.solu_info[id].en = 1;
        }
        else{
            solution_list.solu_info[id].en = 0;
        }
    }
    //
    if(xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL]==3)
        solution_list.solu_info[id].en = 0;
    //
    solution_data_chk(id);
    fl_solution_write();
    // ��־����
    log_solu_config();
solution_config_end:
    user_sending_len = solution_config_build(id,state,xtcp_rx_buf[SOLU_CFGACK_CONFIG]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    if(state==0){
        mes_send_suloinfo(id);
        taskview_page_messend();
    }
    create_todaytask_list(g_sys_val.time_info);
}

//====================================================================================================
// ���ö�ʱ������ϢЭ�� B306
//====================================================================================================
void task_config_recive(){
    uint16_t id;
    // �������ID
    id = (xtcp_rx_buf[TASK_CFG_TASK_ID+1]<<8)|xtcp_rx_buf[TASK_CFG_TASK_ID];
    //xtcp_debug_printf("B306 id %d\n",id);
    //
    if(id>MAX_HOST_TASK-1)
        return;
    // ��ֹ������ǰ����
    if((xtcp_rx_buf[TASK_CFG_TASK_STATE]==0)&&((xtcp_rx_buf[TASK_CFG_CONTORL]==1)||(xtcp_rx_buf[TASK_CFG_CONTORL]==2))){
        for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
            if((timetask_now.ch_state[i]!=0xFF)&&(timetask_now.task_musicplay[i].task_id==id)){
                task_music_config_stop(i);
                // �ض�����
                //fl_timertask_read(&tmp_union.task_allinfo_tmp,id);
            }
        }
    }
    //
    uint8_t config_bit = xtcp_rx_buf[TASK_CFG_CFGBIT];
    // ȡ��������Ϣ
    //fl_timertask_read(&tmp_union.task_allinfo_tmp,id);
    //
    //xtcp_debug_printf("task config state %d\n",xtcp_rx_buf[TASK_CFG_CONTORL]);
    if(xtcp_rx_buf[TASK_CFG_CONTORL]==1){ //ɾ������
        delete_task_node(id);
        timetask_list.timetask[id].id=0xFFFF;
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_id = 0xFFFF;
        xtcp_rx_buf[TASK_CFG_CFGBIT] = 0xFF;
        config_bit =0xFF;
        g_sys_val.task_delete_s = 1;
        g_sys_val.task_config_s = 1;
    }
    else if(xtcp_rx_buf[TASK_CFG_CONTORL]==0){ //�������
        g_sys_val.task_creat_s = 1;
        g_sys_val.task_config_s = 0;
        config_bit=0xFF;
        xtcp_rx_buf[TASK_CFG_CFGBIT] = 0xFF;
    }
    else if(xtcp_rx_buf[TASK_CFG_CONTORL]==3){ //��¡����
        g_sys_val.task_creat_s = 1;
        g_sys_val.task_config_s = 2;
        config_bit=0x0;
        xtcp_rx_buf[TASK_CFG_CFGBIT] = 0xFF;
    }
    // ���ø��ݱ�־λ������Ϣ
    //xtcp_debug_printf("task id %d\n",id);
    if(config_bit&1){    //��������
        //xtcp_debug_printf("con name\n");
        //for(uint8_t i=0;i<DIV_NAME_NUM;i++){
        //    xtcp_debug_printf("%x,",xtcp_rx_buf[TASK_CFG_TASK_NAME+i]);
        //}
        //xtcp_debug_printf("\n");
        memcpy(g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_name,&xtcp_rx_buf[TASK_CFG_TASK_NAME],DIV_NAME_NUM);
    }
    config_bit>>=1;     //����״̬
    if(config_bit&1){
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_state = xtcp_rx_buf[TASK_CFG_TASK_STATE];
        //xtcp_debug_printf("task state %d\n",g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_state);
    }
    config_bit>>=1;
    if(config_bit&1){   //��������
        //xtcp_debug_printf("vol %d\n",xtcp_rx_buf[TASK_CFG_TASK_VOL]);
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_vol = xtcp_rx_buf[TASK_CFG_TASK_VOL];
    }
    config_bit>>=1;
    if(config_bit&1){   //�����ظ�ģʽ
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_repe_mode = xtcp_rx_buf[TASK_CFG_REPE_MODE];
        //�������ظ�
        //xtcp_debug_printf("config day:\n");
        for(uint8_t i=0;i<MAX_TASK_DATE_NUM;i++){
            memcpy(&g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i],&xtcp_rx_buf[TASK_CFG_REPE_DATE+i*3],3);
        }
        //�����ظ�
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.week_repebit = xtcp_rx_buf[TASK_CFG_REPE_WEEK];
    }
    config_bit>>=1; //����ʼʱ��
    if(config_bit&1){
        memcpy(&g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.time_info,&xtcp_rx_buf[TASK_CFG_BEG_TIME],3);
        
    }
    config_bit>>=1; //�������ʱ��
    if(config_bit&1){
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dura_time = xtcp_rx_buf[TASK_CFG_DURA_TIME]*3600+xtcp_rx_buf[TASK_CFG_DURA_TIME+1]*60+xtcp_rx_buf[TASK_CFG_DURA_TIME+2];
    }
    config_bit>>=1; //��������
    if(config_bit&1){
        //xtcp_debug_printf("con play\n");
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.play_mode = xtcp_rx_buf[TASK_CFG_PLAY_MODE];
        //tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum = xtcp_rx_buf[TASK_CFG_PLAY_TOL];
    }
    config_bit>>=1; //��������
    if(config_bit&1){
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn = xtcp_rx_buf[TASK_CFG_SOLU_ID];
    }
    // �������ȼ�
    if(g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn==0xFF){
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_prio = xtcp_rx_buf[TASK_CFG_TASK_PRIO];
    }
    else{ //�������񰴷������ȼ�ѡ��
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_prio = AUX_I1;
        for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
            if((solution_list.solu_info[i].state!=0xFF)&&(solution_list.solu_info[i].id==xtcp_rx_buf[TASK_CFG_SOLU_ID])){
                g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_prio = solution_list.solu_info[i].prio;
            }
        }
    }
    //xtcp_debug_printf("task prio %d,%d\n", g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_prio,xtcp_rx_buf[TASK_CFG_SOLU_ID]);    
    
    // ����ʧ��
    if(xtcp_rx_buf[TASK_CFG_CFGBIT] == 0)
        g_sys_val.task_con_state |= 2;
    // 
}

//====================================================================================================
// ���ö�ʱ������ϸ��ϢЭ��                     B307
//====================================================================================================
void task_dtinfo_config_recive(){
    static uint8_t config_state;
    //����ͬһʱ��ֻ�ܴ���һ�ֶ����������
    //if(conn_sending_s.id!=null)
    //    goto dtinfo_send_end;
    //xtcp_debug_printf("B407 ID : %d,pack tol %d inc %d\n",((xtcp_rx_buf[TASK_DTCFG_ID+1]<<8)|xtcp_rx_buf[TASK_DTCFG_ID]),xtcp_rx_buf[TASK_DTCFG_TOLPACK],xtcp_rx_buf[TASK_DTCFG_PACKNUM]);
    // ������һ������
    //xtcp_debug_printf("rec pack inc %d\n",xtcp_rx_buf[TASK_DTCFG_PACKNUM]);
    
    if((g_sys_val.task_recid==0xFFFF)&&(xtcp_rx_buf[TASK_DTCFG_PACKNUM]==0)){
        g_sys_val.task_recid = (xtcp_rx_buf[TASK_DTCFG_ID+1]<<8)|xtcp_rx_buf[TASK_DTCFG_ID];
        g_sys_val.task_con_id = g_sys_val.task_recid;
        fl_timertask_read(&g_sys_val.tmp_union.task_allinfo_tmp,g_sys_val.task_recid);
        g_sys_val.task_music_inc = 0;
        g_sys_val.task_packrec_inc = 0;
        g_sys_val.task_con_state = 0;
        g_sys_val.task_delete_s = 0;
        g_sys_val.task_creat_s = 0;
        g_sys_val.task_dtinfo_setmusic_f = 0;
        g_sys_val.task_dtinfo_setdiv_f = 0;
        g_sys_val.task_creat_s = 0;
        g_sys_val.task_config_s = 2;
       // xtcp_debug_printf("rec id:%d %d,%d\n",g_sys_val.task_recid,g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum,g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum);
    }
    // ���հ�+1
    g_sys_val.task_packrec_inc ++;
    //����ְ�
    //xtcp_debug_printf("rec config byte %d\n",xtcp_rx_buf[TASK_DTCFG_PACKTYPE]);
    if(g_sys_val.task_recid == ((xtcp_rx_buf[TASK_DTCFG_ID+1]<<8)|xtcp_rx_buf[TASK_DTCFG_ID]) ){
        uint16_t data_base;
        //������Ϣ����
        if(xtcp_rx_buf[TASK_DTCFG_PACKTYPE]==2){
            task_config_recive();
            config_state = xtcp_rx_buf[TASK_CFG_CONTORL];
        }
        else if(xtcp_rx_buf[TASK_DTCFG_PACKTYPE]==0){ //��ȡ�����б�
            #if 0
            for(uint8_t i=0;i<80;i++){
                xtcp_debug_printf("%2x ",xtcp_rx_buf[i]);
                if(i!=0&&i%20==0)
                    xtcp_debug_printf("\n");
            }
            xtcp_debug_printf("\n");
            #endif
            g_sys_val.task_dtinfo_setdiv_f = 1;
            data_base = TASK_DTCFG_MAC_BASE;
            //����豸����
            g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum = xtcp_rx_buf[TASK_DTCFG_MACTOL];
            xtcp_debug_printf("div tol %d\n",xtcp_rx_buf[TASK_DTCFG_MACTOL]);
            for(uint8_t i=0;i<xtcp_rx_buf[TASK_DTCFG_MACTOL] ;i++){
                // ��÷�������λ
                g_sys_val.tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].zone_control = 
                                                            (xtcp_rx_buf[data_base+TASK_DTCFG_AREACFG+1]<<8)|xtcp_rx_buf[data_base+TASK_DTCFG_AREACFG];
                //xtcp_debug_printf("task area %x\n",g_sys_val.tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].zone_control);
                // ���MAC
                memcpy(g_sys_val.tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac,
                       &xtcp_rx_buf[data_base+TASK_DTCFG_MAC],6);
                //
                data_base += TASK_DTCFG_MAC_LEN;
            }
        }
        else if(xtcp_rx_buf[TASK_DTCFG_PACKTYPE]==1){ //��ȡ�����б�
            g_sys_val.task_dtinfo_setmusic_f = 1;
            data_base = TASK_DTCFG_MUSIC_BASE;
            for(uint8_t i=0; i<xtcp_rx_buf[TASK_DTCFG_MUSICTOL]; i++){
                // �������·����
                memcpy(g_sys_val.tmp_union.task_allinfo_tmp.task_musiclist.music_info[g_sys_val.task_music_inc].music_path,
                       &xtcp_rx_buf[data_base+TASK_DTCFG_MUSICPATCH],PATCH_NAME_NUM);
                #if 0
                for(uint8_t i=0;i<PATCH_NAME_NUM;i++){
                    if(xtcp_rx_buf[data_base+TASK_DTCFG_MUSICNAME+i]==0){
                        xtcp_debug_printf("\n");  
                        break;
                    }
                    xtcp_debug_printf("%c",xtcp_rx_buf[data_base+TASK_DTCFG_MUSICPATCH+i]);        
                }
                #endif
                // ���������
                memcpy(g_sys_val.tmp_union.task_allinfo_tmp.task_musiclist.music_info[g_sys_val.task_music_inc].music_name,
                          &xtcp_rx_buf[data_base+TASK_DTCFG_MUSICNAME],MUSIC_NAME_NUM);
                //
                #if 0
                for(uint8_t i=0;i<MUSIC_NAME_NUM;i++){
                    if(g_sys_val.tmp_union.task_allinfo_tmp.task_musiclist.music_info[g_sys_val.task_music_inc].music_name[i]==0){
                        xtcp_debug_printf("\n");  
                        break;
                    }
                    xtcp_debug_printf("%c",g_sys_val.tmp_union.task_allinfo_tmp.task_musiclist.music_info[g_sys_val.task_music_inc].music_name[i]);        
                }
                #endif
                g_sys_val.task_music_inc++;  
                g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum = g_sys_val.task_music_inc;
                //xtcp_debug_printf("music tol %d\n",g_sys_val.task_music_inc);
                data_base += TASK_DTCFG_MUSICLEN;
            }
        }
        // ����������
        if(((xtcp_rx_buf[TASK_DTCFG_PACKNUM]+1)==xtcp_rx_buf[TASK_DTCFG_TOLPACK])&&(g_sys_val.task_packrec_inc == xtcp_rx_buf[TASK_DTCFG_TOLPACK])){
            g_sys_val.task_packrec_inc=0;
            //�Ƿ����������
            if((xtcp_rx_buf[TASK_DTCFG_TOLPACK]==1)&&(xtcp_rx_buf[TASK_DTCFG_PACKTYPE]==2)){
                goto dtinfo_config_ok;
            }
            //�ж�д���Ƿ� 0�׸� ���� 0���豸
            if((((g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum==0)&&(g_sys_val.task_dtinfo_setmusic_f))||
                ((g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum==0)&&(g_sys_val.task_dtinfo_setdiv_f)))&&
                (g_sys_val.task_delete_s!=1)){
                //
                g_sys_val.task_con_state |= 0xC0;
            }
            //----------------------------------------------------------------------------------------------
            dtinfo_config_ok:
            if(g_sys_val.task_con_state) // ����ʧ��
                goto dtinfo_send_end;
            //----------------------------------------------------------------------------------------------
            // ���óɹ�
            if(g_sys_val.task_creat_s){
                //------------------------------------------------------------------------------------------------------
                //���������ж�
                uint16_t tasknum_cnt=0;
                timetask_t *task_p = timetask_list.all_timetask_head;
                //
                while(task_p!=null){
                    if(task_p->solu_id==g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn)
                        tasknum_cnt++;
                    task_p = task_p->all_next_p;
                }
                if((tasknum_cnt>=MAX_TIMED_TASK_NUM)&&(g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn==0xFF)){
                    g_sys_val.task_con_state |= 1;
                    goto dtinfo_send_end;
                }
                if(tasknum_cnt>=MAX_SOUL_HAVETASK){
                    g_sys_val.task_con_state |= 1;
                    goto dtinfo_send_end;
                }
                //-------------------------------------------------------------------------------------------------
                uint16_t id;
                if(create_task_node()){ //������ӳɹ�   
                    id = timetask_list.all_timetask_end->id;
                    g_sys_val.task_con_id = id;
                    g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_id =id;
                    //                        
                   // xtcp_debug_printf("task creat id:%d\n",id);
                }         
                else{
                    g_sys_val.task_con_state |= 1;
                    goto dtinfo_send_end;
                }
            }   
            //xtcp_debug_printf("task save %d\n",g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dura_time);
            //---------------------------------------------------------------
            // ����������Ϣ
            timetask_list.timetask[g_sys_val.task_con_id ].time_info = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.time_info;
            timetask_list.timetask[g_sys_val.task_con_id ].task_en = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_state;
            timetask_list.timetask[g_sys_val.task_con_id ].solu_id = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn;
            timetask_list.timetask[g_sys_val.task_con_id ].repe_mode = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_repe_mode;
            timetask_list.timetask[g_sys_val.task_con_id ].week_repebit = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.week_repebit;
            for(uint8_t i=0;i<MAX_TASK_DATE_NUM;i++){
                timetask_list.timetask[g_sys_val.task_con_id ].date_info[i] = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i];
            }
            //-------------------------------------------------------------
            // �ж�ʱ�� �Զ���ֹ 
            uint8_t hour = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour;
            uint8_t minute = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute;
            uint8_t second = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.time_info.second;

            if(tasktime_decode(hour,minute,second,
                               g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dura_time,
                               g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn,g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dateinfo)
                               &&(g_sys_val.task_delete_s==0)){
               if(g_sys_val.task_creat_s==0)
                    g_sys_val.task_con_state |= 16;
                g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_state = 0;
                timetask_list.timetask[g_sys_val.task_con_id].task_en=0;
            }
            //-------------------------------------------------------------
            //xtcp_debug_printf("rttask write id:%d buf id:%d\n",g_sys_val.task_con_id,g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_id);
            
            fl_timertask_write(&g_sys_val.tmp_union.task_allinfo_tmp,g_sys_val.task_con_id);
            //-----------------------------------------------------------------------------------------------------
            // ��ɴ���
            dtinfo_send_end:
            g_sys_val.task_recid=0xFFFF;
            user_sending_len = threebyte_ack_build(g_sys_val.task_con_id,g_sys_val.task_con_id>>8,g_sys_val.task_con_state,TASK_DIINFO_CONFIG_CMD);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
            xtcp_debug_printf("send id: %d\n state %x\n",g_sys_val.task_con_id,g_sys_val.task_con_state);
            create_todaytask_list(g_sys_val.time_info);
            //---------------------------------------------------------------------------------
            // ��Ϣ����
            if(g_sys_val.task_con_state == 0){
                // ��־��¼ 
                log_timetask_config(config_state);
                mes_send_taskinfo(&g_sys_val.tmp_union.task_allinfo_tmp);           
                if(g_sys_val.task_delete_s || g_sys_val.task_creat_s)
                    mes_send_suloinfo(g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn);
            }
            //--------------------------------------------------------------------------------
            xtcp_debug_printf("task dtinfo config over\n");
        }    
    }
    //    
}
// ��ʱ����ʧ��
void task_dtinfo_overtime_recive_close(){
    if(g_sys_val.task_recid!=0xFFFF){
        g_sys_val.task_rec_count++;
        if(g_sys_val.task_rec_count>8){
            g_sys_val.task_recid=0xFFFF;
            g_sys_val.task_rec_count = 0;
        }
    }
}

//====================================================================================================
// ��ʱ���������༭Э��                  B30D
//====================================================================================================
#define DAY_TIME_SECOND (24*60*60)
void task_bat_config_recive(){
    uint16_t dat_base = TASK_BAT_TASKID;
    uint16_t id;
    uint8_t beg_hour,beg_minute,beg_second;
    uint8_t solu_id;
    unsigned rectime,tasktime;

    rectime = (xtcp_rx_buf[TASK_BAT_BEGTIME]*3600)+(xtcp_rx_buf[TASK_BAT_BEGTIME+1]*60)+xtcp_rx_buf[TASK_BAT_BEGTIME+2];
    rectime = rectime%DAY_TIME_SECOND;
    
    beg_hour = xtcp_rx_buf[TASK_BAT_BEGTIME] %24;
    beg_minute = xtcp_rx_buf[TASK_BAT_BEGTIME+1] %60;
    beg_second = xtcp_rx_buf[TASK_BAT_BEGTIME+2] %60;
    //
    uint8_t state=01;
    //debug_printf("status %d\n",xtcp_rx_buf[TASK_BAT_CONFIG_S]);
    for(uint8_t i=0;i<xtcp_rx_buf[TASK_BAT_TASKTOL];i++){
        id = xtcp_rx_buf[dat_base]|(xtcp_rx_buf[dat_base+1]<<8);
        if(id>MAX_HOST_TASK){
            state = 0;
            break;
        }
        fl_timertask_read(&g_tmp_union.task_allinfo_tmp,id);
        solu_id = g_tmp_union.task_allinfo_tmp.task_coninfo.solution_sn;
        tasktime = (g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour*3600)+
                   (g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute*60)+g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.second;
        // ɾ������
        if(xtcp_rx_buf[TASK_BAT_CONFIG_S]==01){
            debug_printf("del id %d\n",id);
            delete_task_node(id);
            timetask_list.timetask[id].id=0xFFFF;
            g_tmp_union.task_allinfo_tmp.task_coninfo.task_id = 0xFFFF;
            fl_timertask_write(&g_tmp_union.task_allinfo_tmp,id);
            dat_base+=2;
            continue;    
        }
        // ��ǰʱ��
        if(xtcp_rx_buf[TASK_BAT_CONFIG_S]==02){
            if(rectime>tasktime){
                tasktime = DAY_TIME_SECOND - (rectime-tasktime);
            }
            else{
                tasktime -= rectime;
            }
            g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.second = tasktime % 60;
            g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute = (tasktime % 3600)/60;
            g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour = tasktime/3600;
            timetask_list.timetask[id].time_info = g_tmp_union.task_allinfo_tmp.task_coninfo.time_info;
        }
        // �Ӻ�ʱ��
        if(xtcp_rx_buf[TASK_BAT_CONFIG_S]==03){
            tasktime += rectime;
            tasktime = tasktime%DAY_TIME_SECOND;
            g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.second = tasktime % 60;
            g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute = (tasktime % 3600)/60;
            g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour = tasktime/3600;
            timetask_list.timetask[id].time_info = g_tmp_union.task_allinfo_tmp.task_coninfo.time_info;
        }
        // ����ʱ��
        if(xtcp_rx_buf[TASK_BAT_DURATIME_S]==01){
            g_tmp_union.task_allinfo_tmp.task_coninfo.dura_time = xtcp_rx_buf[TASK_BAT_DURATIME]*3600+xtcp_rx_buf[TASK_BAT_DURATIME+1]*60+xtcp_rx_buf[TASK_BAT_DURATIME+2];
        }
        fl_timertask_write(&g_tmp_union.task_allinfo_tmp,id);
        // ��һ��id
        dat_base+=2;
    }
    //-------------------------------------------------------------
    
    // �ж�ʱ�� �Զ���ֹ
    timetask_t *task_p;
    dat_base = TASK_BAT_TASKID;
    unsigned next_tasktime,beg_time,end_time;
    uint8_t tasksolu_id;
    for(uint8_t i=0;i<xtcp_rx_buf[TASK_BAT_TASKTOL];i++){
        uint8_t over_time_inc=0;
        task_p = timetask_list.all_timetask_head;
        id = xtcp_rx_buf[dat_base]|(xtcp_rx_buf[dat_base+1]<<8);
        fl_timertask_read(&g_tmp_union.task_allinfo_tmp,id);
        //
        if(tasktime_decode(g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour,
                           g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute,
                           g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.second,
                           g_tmp_union.task_allinfo_tmp.task_coninfo.dura_time,
                           g_tmp_union.task_allinfo_tmp.task_coninfo.solution_sn,
                           g_tmp_union.task_allinfo_tmp.task_coninfo.dateinfo)){
            // ��ֹ�뱣������
            fl_timertask_read(&g_tmp_union.task_allinfo_tmp,id);
            timetask_list.timetask[id].task_en=0;
            g_tmp_union.task_allinfo_tmp.task_coninfo.task_state=0;
            fl_timertask_write(&g_tmp_union.task_allinfo_tmp,id);
        }
        /*
        beg_time = (g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour*3600)+
                   (g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute*60)+
                   g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.second;
        end_time = beg_time+g_tmp_union.task_allinfo_tmp.task_coninfo.dura_time;
        //
        tasksolu_id = g_tmp_union.task_allinfo_tmp.task_coninfo.solution_sn;
        //
        while(task_p!=null){
            fl_timertask_read(&g_tmp_union.task_allinfo_tmp,task_p->id);
            next_tasktime = (g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour*3600)+
                            (g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute*60)+
                             g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.second;
            // ʱ�䲻��ȷ
            if((next_tasktime > beg_time)&&(next_tasktime<end_time)&&(task_p->id!=id)&&(tasksolu_id==task_p->solu_id)){
                over_time_inc++;
                if(over_time_inc>SOLU_MAX_PLAYCH){
                    fl_timertask_read(&g_tmp_union.task_allinfo_tmp,id);
                    g_tmp_union.task_allinfo_tmp.task_coninfo.task_state = 0;
                    timetask_list.timetask[id].task_en=0;
                    fl_timertask_write(&g_tmp_union.task_allinfo_tmp,id);
                }
                break;
            }
            task_p = task_p->all_next_p;
        }
        */
        // ��һ��id
        dat_base+=2;
    }
    
    //-------------------------------------------------------------
    create_todaytask_list(g_sys_val.time_info);
    user_sending_len = onebyte_ack_build(state,TASK_BAT_CONFIG_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]); 
    mes_send_listinfo(TIMETASKERROR_INFO_REFRESH,0);
    taskview_page_messend();
    
    mes_send_suloinfo(solu_id);

    // ��־��¼
    log_timetask_config(5);
}

//====================================================================================================
// ��ʱ���񲥷Ž�ֹ/����Э��                  B30E
//====================================================================================================
void task_en_recive(){ 
    uint16_t id;
    uint8_t state=0;
    id = (xtcp_rx_buf[POL_DAT_BASE+1]<<8)|xtcp_rx_buf[POL_DAT_BASE];
    g_sys_val.task_con_id = id;
    //
    if(id>MAX_HOST_TASK-1)
        goto task_en_end;
    // ��ֹ������ǰ����
    if(xtcp_rx_buf[POL_DAT_BASE+2]==0){
        for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
            if((timetask_now.ch_state[i]!=0xFF)&&(timetask_now.task_musicplay[i].task_id==id)){
                task_music_config_stop(i);
                // �ض�����
                //timer_task_read(&tmp_union.task_allinfo_tmp,id);
            }
        }
    }
    fl_timertask_read(&g_tmp_union.task_allinfo_tmp,id);
    //����ʱ�ж�����ʱ��
    task_dateinfo_t dateinfo[MAX_TASK_DATE_NUM];
    if(xtcp_rx_buf[POL_DAT_BASE+2]){
        for(uint8_t i=0;i<MAX_TASK_DATE_NUM;i++){
            dateinfo[i] = g_tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i];
        }
        if(tasktime_decode(g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour,
                           g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute,
                           g_tmp_union.task_allinfo_tmp.task_coninfo.time_info.second,
                           g_tmp_union.task_allinfo_tmp.task_coninfo.dura_time,
                           g_tmp_union.task_allinfo_tmp.task_coninfo.solution_sn,
                           dateinfo
                           )){
            user_sending_len = onebyte_ack_build(2,TASK_EN_CONFIG_CMD,&xtcp_rx_buf[POL_ID_BASE]);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);    
            return;
        }
        fl_timertask_read(&g_tmp_union.task_allinfo_tmp,id);
    }
    g_tmp_union.task_allinfo_tmp.task_coninfo.task_state = xtcp_rx_buf[POL_DAT_BASE+2];
    xtcp_debug_printf("B30E task en %d\n",xtcp_rx_buf[POL_DAT_BASE+2]);
    timetask_list.timetask[id].task_en = xtcp_rx_buf[POL_DAT_BASE+2];
    fl_timertask_write(&g_tmp_union.task_allinfo_tmp,id);
    create_todaytask_list(g_sys_val.time_info);
    state = 1;
    task_en_end:
    user_sending_len = onebyte_ack_build(state,TASK_EN_CONFIG_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);    
    //---------------------------------------------------------------------------------
    // ������Ϣ����
    if(state=1){
        //xtcp_debug_printf("\n\ntask updata\n\n");
		g_sys_val.task_config_s = 2; //����༭
        mes_send_taskinfo(&g_tmp_union.task_allinfo_tmp);
    }
    //--------------------------------------------------------------------------------
}

//====================================================================================================
// ��ʱ���񲥷ſ���Э��                  B308 
//====================================================================================================
void task_playtext_recive(){    
    uint16_t id = (xtcp_rx_buf[TASK_PLAY_ID+1]<<8)|xtcp_rx_buf[TASK_PLAY_ID];
    if(xtcp_rx_buf[TASK_PLAY_CONTORL]){// 1 ֹͣ
        for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
            //xtcp_debug_printf("text chk %d %d\n",timetask_now.ch_state[i],timetask_now.task_musicplay[i].task_id);
            if((timetask_now.ch_state[i]!=0xFF)&&(timetask_now.task_musicplay[i].task_id==id)&&(timetask_now.task_musicplay[i].rttask_f==0)){
                task_music_config_stop(i);
                // ������Ϣ����
            	g_sys_val.task_config_s = 2; //����༭
            	g_sys_val.task_con_id = id;
                mes_send_taskinfo(&g_tmp_union.task_allinfo_tmp);
                // ��־����
                log_timetask_contorl();
                goto play_text_sucess;
            }
        }
        goto play_text_sucess;
    }
    // ��������
    xtcp_debug_printf("play id %d\n",id);
    // ��ȡ����
    fl_timertask_read(&g_tmp_union.task_allinfo_tmp,id);
    // �ж��Ƿ������
    if(g_tmp_union.task_allinfo_tmp.task_coninfo.task_id==0xFFFF){
        goto play_text_fail;
    }
    //--------------------------------------------------------------
    //��������ͻ
    uint8_t cnt=0;
    for(uint8_t j=0;j<MAX_MUSIC_CH;j++){
        if(timetask_now.ch_state[j]!=0xFF && timetask_now.task_musicplay[j].sulo_id==g_tmp_union.task_allinfo_tmp.task_coninfo.solution_sn && timetask_now.task_musicplay[j].rttask_f==0){
            cnt++;
            if(cnt>=SOLU_MAX_PLAYCH){
                user_sending_len =  id_ack_build(id,2,TASK_PLAYTEXT_CMD);
                user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
                return;
            }
        }
    }
    // ���������Ƿ��Ѿ��ڲ���
    uint8_t play_flag=0;
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(timetask_now.ch_state[i]!=0xFF && timetask_now.task_musicplay[i].task_id==id && timetask_now.task_musicplay[i].rttask_f==0){
            play_flag=1;
            break;
        }
    }
    // ��־����
    log_timetask_contorl();
    // �������� 
    if(play_flag==0){
        for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
            if(g_sys_val.task_wait_state[i]==0){
                g_sys_val.music_task_id[i] = id;
                g_sys_val.task_wait_state[i] = 1;
                g_sys_val.play_rttask_f[i] = 0;
                g_sys_val.play_ok = 1;
                user_disptask_refresh();
                break;
            }
        }
    }
    //--------------------------------------------------------------
    //
    play_text_sucess:
    user_sending_len =  id_ack_build(id,1,TASK_PLAYTEXT_CMD);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    return;
    //---------------------------------------------------------------------------------
	//
    play_text_fail:
    user_sending_len =  id_ack_build(id,0,TASK_PLAYTEXT_CMD);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);    
    //
}

//====================================================================================================
// ���������ѯ                     B309
//====================================================================================================
void today_week_check_recive(){
    user_sending_len = todaytask_ack_build();
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    //xtcp_debug_printf("today week check\n");
}

//====================================================================================================
// ������������                     B30B
//====================================================================================================
void today_week_config_recive(){
    g_sys_val.today_date.week = xtcp_rx_buf[POL_DAT_BASE];
    g_sys_val.today_date.year = xtcp_rx_buf[POL_DAT_BASE+1];  
    g_sys_val.today_date.month = xtcp_rx_buf[POL_DAT_BASE+2];
    g_sys_val.today_date.date = xtcp_rx_buf[POL_DAT_BASE+3];
    //    
    user_sending_len = onebyte_ack_build(1,TASK_CONFIG_WEEK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    create_todaytask_list(g_sys_val.time_info);
    //
    mes_send_listinfo(TODAYTASK_INFO_REFRESH,0);
    //
    log_todaytask_config();
    //xtcp_debug_printf("today week config\n");
}

//==================================================================================================
// �����༭�ն�����                     B30F
//==================================================================================================
void bat_task_divset_recive(){
    uint16_t task_id,task_num;
    uint16_t dat_len=POL_DAT_BASE+2;
    uint16_t div_base,div_tol,adr_base;
    //
    #if 0
    xtcp_debug_printf("\n\nbat set \n");
    for(uint8_t i=0;i<200;i++){
        xtcp_debug_printf("%x ",xtcp_rx_buf[POL_DAT_BASE+i]);
    }
    xtcp_debug_printf("\n\n");
    #endif
    task_num = xtcp_rx_buf[POL_DAT_BASE]|(xtcp_rx_buf[POL_DAT_BASE+1]<<8);
    
    div_base = task_num*2+dat_len;
    div_tol = xtcp_rx_buf[div_base];
    div_base++;
    xtcp_debug_printf("div tol %d task tol %d\n",div_tol,task_num);
    for(uint16_t i=0;i<task_num;i++){
        adr_base = div_base;
        task_id = xtcp_rx_buf[dat_len]|(xtcp_rx_buf[dat_len+1]<<8);
        fl_timertask_read(&g_tmp_union.task_allinfo_tmp,task_id);
        // �༭�����ն��б�
        for(uint16_t div_num=0; div_num<div_tol; div_num++){
            g_tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum = div_tol;
            g_tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[div_num].zone_control = xtcp_rx_buf[adr_base]|(xtcp_rx_buf[adr_base+1]<<8);
            memcpy(g_tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[div_num].mac,&xtcp_rx_buf[adr_base+2],6);
            adr_base+=8;
        }    
        fl_timertask_write(&g_tmp_union.task_allinfo_tmp,task_id);
        dat_len +=2;
    }
    user_sending_len = onebyte_ack_build(1,TASK_BAT_DIVSET_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);   
}


//====================================================================================================
// ��ʱ�����б��ʼ��            
//====================================================================================================
void fl_rttask_dat_init(){
    rttask_lsit.all_head_p = null;
    rttask_lsit.run_head_p = null;
    rttask_lsit.all_end_p = null;
    rttask_lsit.rttask_tol = 0;
    g_tmp_union.rttask_dtinfo.rttask_id=0xFFFF;
    for(uint8_t i=0;i<MAX_RT_TASK_NUM;i++){
        rttask_lsit.rttask_info[i].rttask_id=0xFFFF;
        rttask_lsit.rttask_info[i].all_next_p=null;
        rttask_lsit.rttask_info[i].run_next_p=null;
        fl_rttask_write(&g_tmp_union.rttask_dtinfo,i);
    }
}

//====================================================================================================
// ��ʱ�����б� ��ȡ          
//====================================================================================================
void rt_task_list_read(){
    rttask_lsit.all_head_p = null;
    rttask_lsit.all_end_p = null;
    rttask_lsit.run_head_p = null;
    rttask_lsit.run_end_p = null;
    for(uint8_t i=1;i<MAX_RT_TASK_NUM;i++){
        rttask_lsit.rttask_info[i].all_next_p = null;
        rttask_lsit.rttask_info[i].run_next_p = null;
        rttask_lsit.rttask_info[i].rttask_id = 0xFFFF;
        fl_rttask_read(&g_tmp_union.rttask_dtinfo,i);
        // �жϴ������� ��λ����
        if((g_tmp_union.rttask_dtinfo.rttask_id!=0xFFFF)&&(g_tmp_union.rttask_dtinfo.rttask_id!=i)){
            g_tmp_union.rttask_dtinfo.rttask_id=0xFFFF;   
            fl_rttask_write(&g_tmp_union.rttask_dtinfo,i);
        }
        if(g_tmp_union.rttask_dtinfo.rttask_id!=0xFFFF){
            create_rttask_node_forid(i);
            xtcp_debug_printf("rt list %d\n",i);
        }
    }
}

//====================================================================================================
// ��ʱ�����б��ѯ                     B400
//====================================================================================================
void rttask_list_check_recive(){
    uint8_t list_num = list_sending_init(RTTASK_CHECK_CMD,RTTASK_LIST_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num==LIST_SEND_INIT)
        return;
    //
	if(g_sys_val.list_sending_f==0){
		g_sys_val.list_sending_f = 1;
	    user_sending_len = rttask_list_chk_build(list_num);
	    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
	}
    //xtcp_debug_printf("rttask list check\n");
}

void rttask_list_sending_decode(uint8_t list_num){
    user_sending_len = rttask_list_chk_build(list_num);
    user_xtcp_send(t_list_connsend[list_num].conn,t_list_connsend[list_num].could_s);
    //xtcp_debug_printf("rttask list sending\n");
}


//====================================================================================================
// ��ʱ������ϸ��Ϣ��ѯ                   B402
//====================================================================================================
void rttask_dtinfo_check_recive(){
    uint16_t id;
    id = (xtcp_rx_buf[RTTASK_DTCK_ID+1]<<8)|xtcp_rx_buf[RTTASK_DTCK_ID]; 
    user_sending_len = rttask_dtinfo_chk_build(id);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

void rttask_runningtask_stop_start(uint16_t id,uint8_t state,uint8_t del_en){
    div_node_t *div_tmp_p;
    conn_list_t *div_conn_p;
    // ���Ҹ�����id�Ƿ��Ѿ�������
    rttask_info_t *runtmp_p = rttask_lsit.run_head_p;
    while(runtmp_p!=null){
        //�Ƚ�����id
        if(runtmp_p->rttask_id == id){ 
            if(del_en){
                // ���������� ɾ��
                delete_rttask_run_node(id);
                //------------------------------------------------------------------------------------------------------------
                // �ر��豸
                fl_rttask_read(&g_tmp_union.rttask_dtinfo,runtmp_p->rttask_id);
            }
            // �ҵ�Դ�豸
            div_tmp_p = get_div_info_p(g_tmp_union.rttask_dtinfo.src_mas);
            if(div_tmp_p == null){
                break;
            }
            //------------------------------------------------------------------------------------------------------------
            // �ҵ�Դ����
            div_conn_p = get_conn_for_ip(div_tmp_p->div_info.ip);
            if(div_conn_p == null){
                break;
            }
            runtmp_p->dura_time = g_tmp_union.rttask_dtinfo.dura_time;
            runtmp_p->over_time = 0;
            
            user_sending_len = rttask_connect_build(state, // 0 �������� // 1 �ر�����
                                                    g_tmp_union.rttask_dtinfo.account_id,
                                                    g_tmp_union.rttask_dtinfo.rttask_id,
                                                    RTTASK_BUILD_CMD,00);
            user_xtcp_send(div_conn_p->conn,0);
            //������������ 
            xtcp_debug_printf("old rttask rebuild\n");
            break;
        }
        runtmp_p = runtmp_p->run_next_p;
    }


}

//====================================================================================================
// ��ʱ��������                    B403
//====================================================================================================
void rttask_config_recive(){
    //
    uint16_t id;
    uint8_t state=1;

    id = (xtcp_rx_buf[RTTASK_CFG_TASKID+1]<<8)|xtcp_rx_buf[RTTASK_CFG_TASKID];
    // �������
    if(xtcp_rx_buf[RTTASK_CFG_CONTORL]==0){
        if(!create_rttask_node()){
            state=0;
        }
        else{
            id = rttask_lsit.all_end_p->rttask_id;
            // ����½�����ID
            g_tmp_union.rttask_dtinfo.rttask_id = id;
            // ��ô����˻�ID
            g_tmp_union.rttask_dtinfo.account_id = xtcp_rx_buf[RTTASK_CFG_ACID];
            // �����½����0
            g_tmp_union.rttask_dtinfo.music_tol=0;
        }
        //
    }
    // ɾ������
    else if(xtcp_rx_buf[RTTASK_CFG_CONTORL]==1){
        // �ر����ֲ���
        close_rttask_musicplay(id);
        //ɾ������������ֹͣ����
        rttask_runningtask_stop_start(id,1,1);
        
        // ɾ����������ڵ�
        if(delete_rttask_node(id)){
            g_tmp_union.rttask_dtinfo.rttask_id = 0xFFFF;
            state=1;
        }
    }
    // �༭����
    else{
        fl_rttask_read(&g_tmp_union.rttask_dtinfo,id);
        //�༭�иı���Դֹͣ����
        if(!charncmp(g_tmp_union.rttask_dtinfo.src_mas,&xtcp_rx_buf[RTTASK_CFG_SRCMAC],6)){
            // �ر����ֲ���
            close_rttask_musicplay(id);
            // ֹͣ���м�ʱ����
            rttask_runningtask_stop_start(id,1,0);
        }
    }
    //-----------------------------------------------------------------------------------------
    #if 0
    rttask_info_t *tmp_p;
    tmp_p = rttask_lsit.all_head_p;
    while(tmp_p!=null){
        xtcp_debug_printf("rt task id %d\n",tmp_p->rttask_id);
        tmp_p = tmp_p->all_next_p;
    }
    #endif
    //-----------------------------------------------------------------------------------------
    // ��������
    memcpy(g_tmp_union.rttask_dtinfo.name,&xtcp_rx_buf[RTTASK_CFG_TASKNAME],DIV_NAME_NUM);
    // ���ò���Դ����
    memcpy(g_tmp_union.rttask_dtinfo.src_mas,&xtcp_rx_buf[RTTASK_CFG_SRCMAC],6);
    // ��������
    g_tmp_union.rttask_dtinfo.task_vol = xtcp_rx_buf[RTTASK_CFG_TASKVOL];
    // �������ʱ��
    if(xtcp_rx_buf[RTTASK_CFG_DURATIME]==0xFF && xtcp_rx_buf[RTTASK_CFG_DURATIME+1]==0xFF && xtcp_rx_buf[RTTASK_CFG_DURATIME+2]==0xFF){
        g_tmp_union.rttask_dtinfo.dura_time=0xFFFFFFFF;
    }
    else{
        g_tmp_union.rttask_dtinfo.dura_time = xtcp_rx_buf[RTTASK_CFG_DURATIME]*3600+xtcp_rx_buf[RTTASK_CFG_DURATIME+1]*60+xtcp_rx_buf[RTTASK_CFG_DURATIME+2];
    }
    //
    g_tmp_union.rttask_dtinfo.play_mode = LOOP_PLAY_M;
    // ң�ذ�������
    g_tmp_union.rttask_dtinfo.task_key = xtcp_rx_buf[RTTASK_CFG_KETINFO];
    // �������ȼ�
    g_tmp_union.rttask_dtinfo.prio = xtcp_rx_buf[RTTASK_CFG_TASKPRIO];
    // ���÷����ն�
    if(xtcp_rx_buf[RTTASK_CFG_DIVTOL] < MAX_DIV_LIST){
        g_tmp_union.rttask_dtinfo.div_tol = xtcp_rx_buf[RTTASK_CFG_DIVTOL];
        uint16_t data_base = RTTASK_CFG_DIV_BASE;
        for(uint8_t i=0; i<xtcp_rx_buf[RTTASK_CFG_DIVTOL]; i++){
            g_tmp_union.rttask_dtinfo.des_info[i].zone_control = (xtcp_rx_buf[data_base+RTTASK_CFG_AREACONTORL+1]<<8)|xtcp_rx_buf[data_base+RTTASK_CFG_AREACONTORL];
            //xtcp_debug_printf("rt t area %x\n",tmp_union.rttask_dtinfo.des_info[i].zone_control);
            memcpy(g_tmp_union.rttask_dtinfo.des_info[i].mac,&xtcp_rx_buf[data_base+RTTASK_CFG_MAC],6);
            data_base += RTTASK_CFG_LEN;
        }
    }    
    // ������Ϣ
    fl_rttask_write(&g_tmp_union.rttask_dtinfo,id);
    //----------------------------------------------------------------------------------------------------------------------------
    // ��ʱ���������У��ط�
    rttask_runningtask_stop_start(id,0,0);
    //----------------------------------------------------------------------------------------------------------------------------
    user_sending_len = rttask_config_ack_build(id,state);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    // ������Ϣ
    if(state){
        //��־����
        log_rttask_config();
        mes_send_rttaskinfo(id,xtcp_rx_buf[RTTASK_CFG_CONTORL],1);
        // ����ҳ�����֪ͨ
        taskview_page_messend();
    }
}
//====================================================================================================
// �ر������еļ�ʱ���� 
//====================================================================================================
void close_running_rttask(uint8_t *mac,uint16_t tid){
    // ����ͬһ��Դ�Ƿ��о����񲢹ر�
    rttask_info_t *runtmp_p = rttask_lsit.run_head_p;
    while(runtmp_p!=null){
        fl_rttask_read(&g_tmp_union.rttask_dtinfo,runtmp_p->rttask_id);       
        //�Ƚ�MAC
        if(charncmp(g_tmp_union.rttask_dtinfo.src_mas,mac,6) && (tid==runtmp_p->rttask_id)){
            delete_rttask_run_node(runtmp_p->rttask_id);
            //������������ 
            xtcp_debug_printf("del old rttask\n");
            break;
        }
        runtmp_p = runtmp_p->run_next_p;
    }
}

//====================================================================================================
// ��ʱ���񿪹ؿ���                    B404
//====================================================================================================
void rttask_contorl_recive(){
    xtcp_debug_printf("rttask contorl\n");
    uint16_t id;
    uint16_t user_id;
    div_node_t *div_tmp_p;
    conn_list_t *div_conn_p;
    // ��ȡ����ID
    id = (xtcp_rx_buf[RTTASK_PLAY_TASKID+1]<<8)|xtcp_rx_buf[RTTASK_PLAY_TASKID];
    user_id = xtcp_rx_buf[RTTASK_PLAY_USERID]|(xtcp_rx_buf[RTTASK_PLAY_USERID+1]<<8);
    //------------------------------------------------------------------------------------------
    // ���������Ƿ��ѱ�����
    //if((rttask_run_chk(id))&&(xtcp_rx_buf[RTTASK_PLAY_CONTORL]))
    //    goto rttaask_creat_fail;
    //-------------------------------------------------------------------------------------------
    // ��ʱ�����ȡ
    fl_rttask_read(&g_tmp_union.rttask_dtinfo,id);
    //---------------------------------------------------------------------------------------------------------------
    // ���豸�б���Դ�豸
    //xtcp_debug_printf("rt src mac %x,%x,%x,%x,%x,%x\n",tmp_union.rttask_dtinfo.src_mas[0],tmp_union.rttask_dtinfo.src_mas[1],tmp_union.rttask_dtinfo.src_mas[2],
    //                                             tmp_union.rttask_dtinfo.src_mas[3],tmp_union.rttask_dtinfo.src_mas[4],tmp_union.rttask_dtinfo.src_mas[5]);    
    //----------------------------------------------------------------------------------------------------------------
    // ����������Դ
    uint8_t state=0;             
    if(charncmp(g_tmp_union.rttask_dtinfo.src_mas,host_info.mac,6)){
        // ������ʱ����
        if(xtcp_rx_buf[RTTASK_PLAY_CONTORL]){
            // �ر������еļ�ʱ���� 
            close_running_rttask(&xtcp_rx_buf[POL_MAC_BASE],id);
            // �ر����ֲ���
            close_rttask_musicplay(id);
            //---------------------------------------------------------------------------------------------------------
            // ����������
            if(!(rttask_run_chk(id))){
                //��������ͻ
                uint8_t cnt=0;
                for(uint8_t j=0;j<MAX_MUSIC_CH;j++){
                    if(timetask_now.ch_state[j]!=0xFF && timetask_now.task_musicplay[j].rttask_f){
                        cnt++;
                        if(cnt>=SOLU_MAX_PLAYCH){
                            goto host_rttask_build_end;
                        }
                    }
                }
                // ���������Ƿ��Ѿ��ڲ���
                uint8_t play_flag=0;
                for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
                    if(timetask_now.ch_state[i]!=0xFF && timetask_now.task_musicplay[i].task_id==id && timetask_now.task_musicplay[i].rttask_f){
                        play_flag=1;
                        break;
                    }
                }
                // �������ֲ���                
                if(play_flag==0){
                    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
                        if(timetask_now.ch_state[i]==0xFF){
                            // ������ʱ�������нڵ�
                            if(create_rttask_run_node(id)==0){
                                goto host_rttask_build_end;
                            }  
                            //��ʼ�� ��������״̬
                            rttask_lsit.run_end_p->user_id = user_id;
                            rttask_lsit.run_end_p->dura_time = g_tmp_union.rttask_dtinfo.dura_time;
                            rttask_lsit.run_end_p->over_time = 0;
                            rttask_lsit.run_end_p->run_state = 01;
                            // 
                            g_sys_val.play_rttask_f[i] = 1;                            
                            g_sys_val.music_task_id[i] = id;
                            // ��������
                            task_music_config_play(i,g_sys_val.music_task_id[i],g_sys_val.play_rttask_f[i],0,0);
                            user_disptask_refresh();   
                            
                            state = 1;
                            break;
                        }
                    }
                }
            }
        }
        // �رռ�ʱ����
        else{
            // �ر����ֲ���
            close_rttask_musicplay(id);
            // �ر������еļ�ʱ���� 
            close_running_rttask(g_tmp_union.rttask_dtinfo.src_mas,id);
            
            // �رռ�ʱ������Ϣ����
            for(uint8_t i=0;i<MAX_SEND_RTTASKINFO_NUM;i++){
                if(conn.id == rttask_info_list[i].conn.id){
                    rttask_info_list[i].conn.id=0;
                    break;
                }
            }
           
            state =1;
        }
        // ��ʱ��������ɻظ�
        host_rttask_build_end:
        //xtcp_debug_printf("rtbuild end s %d id %d,uid %d\n",state,id,user_id);
        user_sending_len = rttask_creat_build(user_id,state,id);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        // ��ʱ������Ϣ���� 
        mes_send_rttaskinfo(id,2,0);
        return;
    }
    //----------------------------------------------------------------------------------------------------------------
    // �ⲿ��Դ
    div_tmp_p = get_div_info_p(g_tmp_union.rttask_dtinfo.src_mas);
    if(div_tmp_p == null){
        goto rttaask_creat_fail;
    }
    // �ж��豸�Ƿ�����
    if(div_tmp_p->div_info.div_state==0){
        goto rttaask_creat_fail;
    }
    //--------------------------------------------------------------------------
    // �Ҳ���Դ�豸���ӽڵ�
    xtcp_connection_t new_conn;
    div_conn_p = get_conn_for_ip(div_tmp_p->div_info.ip);
    //
    if(div_conn_p == null){
        if(user_xtcp_connect_udp(8805,div_tmp_p->div_info.ip,&new_conn))
            goto rttaask_creat_fail;
         create_conn_node(&new_conn);    //�½�һ��conn�ڵ�
    }
    else{
        new_conn = div_conn_p->conn;
    }
    xtcp_debug_printf("des_ip %d,%d,%d,%d\n",div_tmp_p->div_info.ip[0],div_tmp_p->div_info.ip[1],div_tmp_p->div_info.ip[2],div_tmp_p->div_info.ip[3]);
    //---------------------------------------------------------------------------------------------------------------
    for(uint8_t i=0;i<MAX_RTTASK_CONTORL_NUM;i++){
        // �򲥷���Դ�豸���벥�ż�ʱ����
        if(rttask_build_state[i].des_conn_id==0){
            rttask_build_state[i].src_conn_id = conn.id;
            rttask_build_state[i].des_conn_id = new_conn.id;
            rttask_build_state[i].rttask_id = id;
            rttask_build_state[i].user_id = user_id;
            rttask_build_state[i].contorl = !xtcp_rx_buf[RTTASK_PLAY_CONTORL];
            rttask_build_state[i].over_time = 0;
            rttask_build_state[i].dura_time = g_tmp_union.rttask_dtinfo.dura_time;
            // ��־��¼
            log_rttask_runingstate(id);
            user_sending_len = rttask_connect_build(rttask_build_state[i].contorl,
                                                    rttask_build_state[i].user_id,
                                                    rttask_build_state[i].rttask_id,
                                                    RTTASK_BUILD_CMD,00);
            
            // ����������
            user_xtcp_send(new_conn,0);
            return;
        }
    }
    //--------------------------------------------------------------------------------------
    // ������ʧ��
    rttaask_creat_fail:
    xtcp_debug_printf("rt creat fail\n");
    user_sending_len = rttask_creat_build(0x00,0x03,id);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}
//====================================================================================================
// ��ʱ���� �豸����                    B407
//====================================================================================================
void rttask_build_recive(){
    xtcp_debug_printf("rttask build \n");
    uint8_t state;
    conn_list_t *conn_tmp_p; 
    for(uint8_t i=0;i<MAX_RTTASK_CONTORL_NUM;i++){
        if(conn.id == rttask_build_state[i].des_conn_id){
            rttask_build_state[i].des_conn_id = 0;
            //---------------------------------------------------------------------------------------------------------------------
            //��������������
            xtcp_debug_printf("contorl %d\n",rttask_build_state[i].contorl);
            if(rttask_build_state[i].contorl==0){
                xtcp_debug_printf("creat run rt %d\n",rttask_build_state[i].rttask_id);
                //---------------------------------------------------------------------------------------------------------
                // �ر������еļ�ʱ���� 
                close_running_rttask(&xtcp_rx_buf[POL_MAC_BASE],rttask_build_state[i].rttask_id);
                //---------------------------------------------------------------------------------------------------------
                // ����������
                if(!(rttask_run_chk(rttask_build_state[i].rttask_id))){
                    if(create_rttask_run_node(rttask_build_state[i].rttask_id)==0){
                        state = 0; 
                        goto rttask_contorl_end;
                    }
                    state = 1;
                    rttask_lsit.run_end_p->user_id = rttask_build_state[i].user_id;
                    rttask_lsit.run_end_p->dura_time = rttask_build_state[i].dura_time;
                    rttask_lsit.run_end_p->over_time = 0;
                    rttask_lsit.run_end_p->run_state = 01;
                }
                //---------------------------------------------------------------------------------------------------------
            }
            //---------------------------------------------------------------------------------------------------------------------
            //�ر�����������
            else if(rttask_build_state[i].contorl==1){   
                state = 1;
                delete_rttask_run_node(rttask_build_state[i].rttask_id);
                xtcp_debug_printf("close run rt %d\n",rttask_build_state[i].rttask_id);
            }
            //---------------------------------------------------------------------------------------------------------------------
            rttask_contorl_end:
            xtcp_debug_printf("rt build over %d\n",state);
            user_sending_len = rttask_creat_build(rttask_build_state[i].user_id,state,rttask_build_state[i].rttask_id);
            //--------------------------------------------------------------------------------------------------------------------
            // �ҿ���ԴCONN���� �ظ�Ӧ��ɹ���
            if(rttask_build_state[i].src_conn_id == g_sys_val.could_conn.id){
                user_xtcp_send(g_sys_val.could_conn,1);
                mes_send_rttaskinfo(rttask_build_state[i].rttask_id,02,0);
                return;
            }
            conn_tmp_p = get_conn_info_p(rttask_build_state[i].src_conn_id);
            if(conn_tmp_p!=null){
                user_xtcp_send(conn_tmp_p->conn,0);
            }
            //
            mes_send_rttaskinfo(rttask_build_state[i].rttask_id,02,0);
            break;
        }
    }
}
//==============================================================================
// ��ʱ���񿪹�״̬��λ
void rttask_build_overtime10hz(){
    for(uint8_t i=0;i<MAX_RTTASK_CONTORL_NUM;i++){
        if(rttask_build_state[i].des_conn_id!=0){
            rttask_build_state[i].over_time++;
            if((rttask_build_state[i].over_time%4)==0){
                conn_list_t *conn_tmp_p; 
                conn_tmp_p = get_conn_info_p(rttask_build_state[i].des_conn_id);
                if(conn_tmp_p!=null){
                    user_sending_len = rttask_connect_build(rttask_build_state[i].contorl,
                                                            rttask_build_state[i].user_id,
                                                            rttask_build_state[i].rttask_id,
                                                            RTTASK_BUILD_CMD,00);
                    user_xtcp_send(conn_tmp_p->conn,xtcp_rx_buf[POL_COULD_S_BASE]);
                }
            }
            if(rttask_build_state[i].over_time>15){
                rttask_build_state[i].des_conn_id=0;
                xtcp_debug_printf("rttask build timeout\n");
            }
        }
    }
}

//====================================================================================================
// ��ʱ���� �ն˵���� �����ط�����                    B40A
//====================================================================================================
void task_rttask_rebuild(){
    //��������������
    rttask_info_t *tmp_p = rttask_lsit.run_head_p;
    while(tmp_p!=null){
        //�Ƚ�MAC
        fl_rttask_read(&g_tmp_union.rttask_dtinfo,tmp_p->rttask_id);
        if(charncmp(g_tmp_union.rttask_dtinfo.src_mas,&xtcp_rx_buf[POL_DAT_BASE],6)){
            //������������
            user_sending_len = rttask_connect_build(00,
                                                    tmp_p->user_id,
                                                    tmp_p->rttask_id,RTTASK_REBUILD_CMD,00);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
            return;
        }
        tmp_p = tmp_p->run_next_p;
    }
    // û������������
    memset(&g_tmp_union.rttask_dtinfo,sizeof(rttask_dtinfo_t),0x00);
    user_sending_len = rttask_connect_build(01,
                                            00,
                                            00,RTTASK_REBUILD_CMD,01);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//====================================================================================================
// ��ʱ����  �����б����            BF0D
//====================================================================================================
void rttask_playlist_updata_init(uint8_t ip[],div_node_t *div_info_p){
    g_sys_val.rttask_updat_p = rttask_lsit.run_head_p;
    g_sys_val.rttask_updat_f = 1;
    g_sys_val.rttask_div_p = div_info_p;
    memcpy(g_sys_val.rttask_up_ip,ip,4);
    xtcp_debug_printf("div ip change %d %d %d %d\n",div_info_p->div_info.ip[0],div_info_p->div_info.ip[1],div_info_p->div_info.ip[2],div_info_p->div_info.ip[3]);
}

void rttask_playlist_updata(){
    uint8_t needsend;
    if(g_sys_val.rttask_updat_f==0)
        return;
    while(g_sys_val.rttask_updat_p!=null){
        user_sending_len = rttask_listupdat_build(&needsend,g_sys_val.rttask_updat_p->rttask_id,g_sys_val.rttask_div_p);
        if(needsend){
            //---------------------------------------------------------------------------------------------------------------
            // �Ҳ���Դ�豸���ӽڵ�
            div_node_t *div_tmp_p=null;
            conn_list_t *div_conn_p=null;
            div_tmp_p = get_div_info_p(g_tmp_union.rttask_dtinfo.src_mas);
            if(div_tmp_p!=null)
                div_conn_p = get_conn_for_ip(div_tmp_p->div_info.ip);
            //
            if(div_conn_p != null)
                user_xtcp_send(div_conn_p->conn,0);
            xtcp_debug_printf("send rttask list updat\n");
            g_sys_val.rttask_updat_p = g_sys_val.rttask_updat_p->run_next_p;
            return;
        }
        g_sys_val.rttask_updat_p = g_sys_val.rttask_updat_p->run_next_p;
    }
    g_sys_val.rttask_updat_f = 0;
    xtcp_debug_printf("rttask list updat end\n");
}


//====================================================================================================
void timer_rttask_run_process(){
    rttask_info_t *tmp_p = rttask_lsit.run_head_p;
    div_node_t *div_tmp_p;
    conn_list_t *div_conn_p;

    while(tmp_p!=null){
        //xtcp_debug_printf("tim %d\n",tmp_p->dura_time);
        if(tmp_p->dura_time!=0xFFFFFFFF){
            // �����쳣 ��ʱ��ͣ
            if(tmp_p->run_state!=2)
                tmp_p->over_time++;
            //xtcp_debug_printf("task time id%d t%d,\n",tmp_p->rttask_id,tmp_p->over_time);
            if(tmp_p->over_time>=tmp_p->dura_time){ 
                fl_rttask_read(&g_tmp_union.rttask_dtinfo,tmp_p->rttask_id);
                // ��־��¼
                log_rttask_timeover();
                //------------------------------------------------------------------------------------------------------------
                // �ҵ�Դ�豸
                div_tmp_p = get_div_info_p(g_tmp_union.rttask_dtinfo.src_mas);
                if(div_tmp_p == null){
                    goto close_rttask;
                }
                //xtcp_debug_printf("close rt mac %x,%x,%x,%x,%x,%x\n",g_tmp_union.rttask_dtinfo.src_mas[0],g_tmp_union.rttask_dtinfo.src_mas[1],g_tmp_union.rttask_dtinfo.src_mas[2],
                //                                                g_tmp_union.rttask_dtinfo.src_mas[3],g_tmp_union.rttask_dtinfo.src_mas[4],g_tmp_union.rttask_dtinfo.src_mas[5]);
                //------------------------------------------------------------------------------------------------------------
                // �ҵ�Դ����
                div_conn_p = get_conn_for_ip(div_tmp_p->div_info.ip);
                if(div_conn_p == null){
                    goto close_rttask;
                }
                user_sending_len = rttask_connect_build(1,
                                                        g_tmp_union.rttask_dtinfo.account_id,
                                                        g_tmp_union.rttask_dtinfo.rttask_id,
                                                        RTTASK_BUILD_CMD,00);
                user_xtcp_send(div_conn_p->conn,0);
                close_rttask:
                // ֹͣ���ֲ���
                close_rttask_musicplay(tmp_p->rttask_id);
                // ֹͣ��������
                delete_rttask_run_node(tmp_p->rttask_id);
                
                // ��Ϣ����
                mes_send_rttaskinfo(g_tmp_union.rttask_dtinfo.rttask_id,2,0);
            }
        }
        tmp_p = tmp_p->run_next_p;
    }//while
}

//====================================================================================================
// B312 �������������ʾ B312
//====================================================================================================
void task_pageshow_recive(){
    user_sending_len = taskview_page_build(TASK_PAGESHOW_B312_CMD);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//====================================================================================================
// B310 �����������ķ����б��ѯ B310
//====================================================================================================
void solulist_chk_forapp_recive(){
    user_sending_len = solution_list_ack_build(TASK_SOLUFORAPP_B310_CMD,1);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//====================================================================================================
// B311 �鿴ָ������ B311
//====================================================================================================
void tasklist_forsolu_chk_recive(){
    uint8_t list_num = list_sending_init(TASK_TASK_FORSOLU_B311_CMD,TASK_LIST_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num==LIST_SEND_INIT)
        return;
    //
    t_list_connsend[list_num].list_info.tasklist.solu_en=1;
    t_list_connsend[list_num].list_info.tasklist.solu_id=xtcp_rx_buf[POL_DAT_BASE];
    //
    uint16_t task_tol=0;
    timetask_t *task_p = timetask_list.all_timetask_head;
    //xtcp_debug_printf("solu id chl%d\n",xtcp_rx_buf[POL_DAT_BASE]);
    while(task_p!=null){
        if(task_p->solu_id==xtcp_rx_buf[POL_DAT_BASE]){
            task_tol++;
        }
        task_p = task_p->all_next_p;
    }
    //xtcp_debug_printf("task num %d\n",task_tol);
    t_list_connsend[list_num].list_info.tasklist.task_tol = task_tol/MAX_TASK_ONCESEND;
    if(task_tol%MAX_TASK_ONCESEND || t_list_connsend[list_num].list_info.tasklist.task_tol==0)
        t_list_connsend[list_num].list_info.tasklist.task_tol++;
    //
    user_sending_len = task_list_ack_build(t_list_connsend[list_num].list_info.tasklist.cmd,1,t_list_connsend[list_num].list_info.tasklist.solu_id,list_num);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//====================================================================================================
// B40E ��ѯ������Դ ��ʱ���������б�
//====================================================================================================
void rttask_musiclist_chk_recive(){
    uint8_t list_num = list_sending_init(RTTASK_MUSICLIST_CHKCMD,RTTASKMUSIC_LIST_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num == LIST_SEND_INIT)
        return;
    //
	if(g_sys_val.list_sending_f==0){
		g_sys_val.list_sending_f = 1;
	    //
        user_sending_len = rttask_muslist_chk_build(list_num);
	    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
	}
    xtcp_debug_printf("muc task\n");
}

//----------------------------------------------
// ��ϸ��Ϣ��������
void rttask_musiclist_chk_decode(uint8_t list_num){
    user_sending_len = rttask_muslist_chk_build(list_num);
    user_xtcp_send(t_list_connsend[list_num].conn,t_list_connsend[list_num].could_s);
    //xtcp_debug_printf("task dtinfo send\n");
}

//====================================================================================================
// B40F ����������Դ ��ʱ���������б�  BF04
//====================================================================================================
void rttask_musiclist_set_recive(){
    uint8_t ch,i,j;
    uint16_t task_id = xtcp_rx_buf[RTTASK_MUCLISTSET_ID]|(xtcp_rx_buf[RTTASK_MUCLISTSET_ID+1]<<8);
    fl_rttask_read(&g_tmp_union.rttask_dtinfo,task_id);
    // ��ʼ��
    if(xtcp_rx_buf[RTTASK_MUCLISTSET_PACKINC]==0){
        g_tmp_union.rttask_dtinfo.music_tol = 0;
    }
    // 
    xtcp_debug_printf("get music tol %d %d\n",xtcp_rx_buf[RTTASK_MUCLISTSET_MUSTOL],task_id);
    // �����Ŀ
    uint16_t dat_base=RTTASK_MUCLISTSET_DATBASE;
    for(i=0;i<xtcp_rx_buf[RTTASK_MUCLISTSET_MUSTOL];i++){
        // ���·����
        memcpy(g_tmp_union.rttask_dtinfo.music_info[g_tmp_union.rttask_dtinfo.music_tol+i].music_path,&xtcp_rx_buf[dat_base+RTTASK_MUCLISTSET_PATCH],PATCH_NAME_NUM);
        // ���������
        memcpy(g_tmp_union.rttask_dtinfo.music_info[g_tmp_union.rttask_dtinfo.music_tol+i].music_name,&xtcp_rx_buf[dat_base+RTTASK_MUCLISTSET_MUSNAME],MUSIC_NAME_NUM);

        dat_base+=RTTASK_MUCLISTSET_DATLEN;
    }
    g_tmp_union.rttask_dtinfo.music_tol += xtcp_rx_buf[RTTASK_MUCLISTSET_MUSTOL];
    // ��������
    fl_rttask_write(&g_tmp_union.rttask_dtinfo,task_id);
    //
    // ������ ������
    if((xtcp_rx_buf[RTTASK_MUCLISTSET_PACKINC]+1)==xtcp_rx_buf[RTTASK_MUCLISTSET_PACKTOL]){
        // �ж��Ƿ�������ִ��
        for(ch=0;ch<MAX_MUSIC_CH;ch++){
            if(timetask_now.ch_state[ch]!=0xFF && timetask_now.task_musicplay[ch].task_id==task_id && timetask_now.task_musicplay[ch].rttask_f){
                g_sys_val.rttask_musicset_f[ch]=1;
                for(uint8_t i=0;i<MAX_SEND_RTTASKINFO_NUM;i++){
                    if(rttask_info_list[i].conn.id!=0 && (rttask_info_list[i].task_id==task_id)){
                        rttask_info_list[i].need_send=1;
                    }
                }
                timetask_now.task_musicplay[ch].music_tol = g_tmp_union.rttask_dtinfo.music_tol;
                //---------------------------------------------------------------------------------------------------------------------------------
                // û�и���ֹͣ���ֲ���
                if(g_tmp_union.rttask_dtinfo.music_tol ==0){
                    timetask_now.task_musicplay[ch].play_state=0;            
                    timetask_now.task_musicplay[ch].music_inc=0;
                    task_music_stop(ch);
                    memset(&g_sys_val.rttask_musinfo,0xFF,sizeof(task_music_info_t));
                    user_rttask_musname_put(&g_sys_val.rttask_musinfo,ch);
                    break;
                }
                //---------------------------------------------------------------------------------------------------------------------------------
                // �ж��Ƿ��о�����
                uint8_t have_music=1;
                user_rttask_musname_get(&g_sys_val.rttask_musinfo,ch);
                for(j=0;j<g_tmp_union.rttask_dtinfo.music_tol;j++){
                    if(charncmp(g_tmp_union.rttask_dtinfo.music_info[j].music_path,g_sys_val.rttask_musinfo.music_path,PATCH_NAME_NUM)&&
                       charncmp(g_tmp_union.rttask_dtinfo.music_info[j].music_name,g_sys_val.rttask_musinfo.music_name,MUSIC_NAME_NUM))
                    {
                        timetask_now.task_musicplay[ch].music_inc=j;
                        have_music=0;
                        break;
                    }
                }
                //---------------------------------------------------------------------------------------------------------------------------------
                //�����ֱ�ɾ��     ����һ�� �������б����ֲ���
                if(have_music){
                    if(timetask_now.task_musicplay[ch].music_inc>=timetask_now.task_musicplay[ch].music_tol){
                        timetask_now.task_musicplay[ch].music_inc=0;
                    }                    
                    //��ʼû����Ŀʱ������
                    if(timetask_now.task_musicplay[ch].play_state==0){
                        task_music_config_play(ch,g_sys_val.music_task_id[ch],g_sys_val.play_rttask_f[ch],1,1);
                        timetask_now.task_musicplay[ch].play_state=0;
                        user_playstate_set(0,ch);
                    }
                    //�ı���Ŀʱ��������
                    else{
                        task_music_config_play(ch,g_sys_val.music_task_id[ch],g_sys_val.play_rttask_f[ch],1,1);
                    }
                }
                //rttask_music_play(task_id);
                break;
            }
        }

        //
        user_sending_len = threebyte_ack_build(xtcp_rx_buf[RTTASK_MUCLISTSET_ID],xtcp_rx_buf[RTTASK_MUCLISTSET_ID+1],0,RTTASK_MUSICLIST_SETCMD);
	    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    }    
}

//====================================================================================================
// B405 ������Դ���� 
//====================================================================================================
void rttask_host_contorl_recive(){
    uint16_t task_id = xtcp_rx_buf[RTTASK_CONTORL_TASKID]|(xtcp_rx_buf[RTTASK_CONTORL_TASKID+1]<<8);
    uint16_t user_id = xtcp_rx_buf[RTTASK_CONTORL_USERID]|(xtcp_rx_buf[RTTASK_CONTORL_USERID]<<8);
    uint16_t contorl_cmd = xtcp_rx_buf[RTTASK_CONTORL_VALUE]|(xtcp_rx_buf[RTTASK_CONTORL_VALUE+1]<<8);
    uint16_t ch=0xFF;
    // �Ҳ��ŵ�����ͨ��
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(timetask_now.ch_state[i]!=0xFF && timetask_now.task_musicplay[i].task_id==task_id && timetask_now.task_musicplay[i].rttask_f){
            ch=i;
        }
    }
    if(ch==0xFF){
        user_sending_len = onebyte_ack_build(0,RTTASK_HOST_CONTORL_CMD,&xtcp_rx_buf[POL_ID_BASE]);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        return;
    }
    switch(xtcp_rx_buf[RTTASK_CONTORL_COMAND]){
        case RTTASK_CMD_PLAY:{
            if(timetask_now.task_musicplay[ch].music_tol==0)
                break;
            if(timetask_now.task_musicplay[ch].music_tol){
                user_playstate_set(1,ch);
                timetask_now.task_musicplay[ch].play_state=1;
            }
            break;
        }
        case RTTASK_CMD_PAUSE:{
            user_playstate_set(0,ch);
            timetask_now.task_musicplay[ch].play_state=0;
            break;
        }
        case RTTASK_CMD_LASTMUS:{
            if(timetask_now.task_musicplay[ch].music_tol==0)
                break;
            // �ҵ���һ�׸��㷨  
            timetask_now.task_musicplay[ch].music_inc+=timetask_now.task_musicplay[ch].music_tol-1;
            if(timetask_now.task_musicplay[ch].music_inc>=timetask_now.task_musicplay[ch].music_tol){
                timetask_now.task_musicplay[ch].music_inc -=timetask_now.task_musicplay[ch].music_tol;
            }
            //
            xtcp_debug_printf("set last %d\n",timetask_now.task_musicplay[ch].music_inc);
            task_musicevent_change(ch,0,1,1);  
            if(timetask_now.task_musicplay[ch].music_tol){
                user_playstate_set(1,ch);
                timetask_now.task_musicplay[ch].play_state=1;
            }
            break;
        }
        case RTTASK_CMD_NEXTMUS:{
            if(timetask_now.task_musicplay[ch].music_tol==0)
                break;
            timetask_now.task_musicplay[ch].music_inc++;
            if(timetask_now.task_musicplay[ch].music_inc>=timetask_now.task_musicplay[ch].music_tol)
                timetask_now.task_musicplay[ch].music_inc=0;
            task_musicevent_change(ch,0,1,1);
            if(timetask_now.task_musicplay[ch].music_tol){
                user_playstate_set(1,ch);
                timetask_now.task_musicplay[ch].play_state=1;
            }
            //rttask_music_next(task_id);
            break;
        }
        case RTTASK_CMD_STOP:{
            break;
        }
        case RTTASK_CMD_SELECT_MUSID:{
            if(timetask_now.task_musicplay[ch].music_tol==0)
                break;
            // �ҵ�ָ�����㷨  
            timetask_now.task_musicplay[ch].music_inc = contorl_cmd-1;
            // ��ֹͣ����
            task_musicevent_change(ch,0,1,1);
            if(timetask_now.task_musicplay[ch].music_tol){
                user_playstate_set(1,ch);
                timetask_now.task_musicplay[ch].play_state=1;
            }
            //rttask_music_select(task_id,contorl_cmd);
            break;
        }
        case RTTASK_CMD_VOL:{
            //rttask_music_setvol(task_id,contorl_cmd);
            timetask_now.task_musicplay[ch].task_vol = contorl_cmd;
            set_audio_vol(ch,timetask_now.task_musicplay[ch].task_vol );
            break;
        }
        case RTTASK_CMD_INFORETURN:{
            // ������Ϣ���¶���
            uint8_t list_num;
            for(list_num=0;list_num<MAX_SEND_RTTASKINFO_NUM;list_num++){
                //���Ƿ�����ͬ����
                if(rttask_info_list[list_num].conn.id==conn.id){
                    goto find_rtinfo_connect;
                }
            }
            //û�ҵ������� ����������
            for(list_num=0;list_num<MAX_SEND_RTTASKINFO_NUM;list_num++){
                //�ҵ�������
                if(rttask_info_list[list_num].conn.id==0){
                    rttask_info_list[list_num].conn=conn;
                    goto   find_rtinfo_connect;
                }
            }
            break;
            // �ҵ�����
            find_rtinfo_connect:
            // ���Ϸ�����Ϣ
            rttask_info_list[list_num].task_id = task_id;
            rttask_info_list[list_num].user_id = user_id;
            rttask_info_list[list_num].could_f = xtcp_rx_buf[POL_COULD_S_BASE];
            memcpy(rttask_info_list[list_num].could_id,&xtcp_rx_buf[POL_ID_BASE],6);
            
            rttask_info_list[list_num].need_send = 1;
            break;
        }
        case RTTASK_CMD_SELECT_TIME:{
            timetask_now.task_musicplay[ch].music_sec=contorl_cmd;
            user_setmusic_sec(ch,contorl_cmd);
            break;
        }
        case RTTASK_CMD_STEPMODE:{
            timetask_now.task_musicplay[ch].play_mode = ORDER_PLAY_M;
            //rttask_music_setmode(task_id,ORDER_PLAY_M);
            break;
        }
        case RTTASK_CMD_LOOPMODE:{    
            timetask_now.task_musicplay[ch].play_mode = LOOP_PLAY_M;
            //rttask_music_setmode(task_id,LOOP_PLAY_M);
            break;
        }
        case RTTASK_CMD_RANDOMMODE:{
            timetask_now.task_musicplay[ch].play_mode = RANDOM_PLAY_M;
            //rttask_music_setmode(task_id,RANDOM_PLAY_M);
            break;
        }
        default:
            break;
    }
    for(uint8_t i=0;i<MAX_SEND_RTTASKINFO_NUM;i++){
        if(rttask_info_list[i].conn.id!=0 && (rttask_info_list[i].task_id==task_id)){
            rttask_info_list[i].need_send=1;
        }
    }

    user_sending_len = onebyte_ack_build(1,RTTASK_HOST_CONTORL_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//====================================================================================================
// B406 ������Դ��Ϣ�ر�
//====================================================================================================
void rttask_host_info_send(uint16_t id,uint8_t list_num){
    for(uint8_t ch=0;ch<MAX_MUSIC_CH;ch++){
        // �Ҷ�Ӧͨ��
        if(timetask_now.ch_state[ch]!=0xFF && timetask_now.task_musicplay[ch].task_id==id && timetask_now.task_musicplay[ch].rttask_f){
            user_sending_len = rttask_infosend_build(list_num,ch);
            user_xtcp_send(rttask_info_list[list_num].conn,rttask_info_list[list_num].could_f);
        }
    }
}

//ÿ3��ˢ��
void rttask_infosend_loop(){
    static uint8_t loop_tim=0;
    loop_tim++;
    if(loop_tim<30)
        return;
    loop_tim=0;
    for(uint8_t i=0;i<MAX_SEND_RTTASKINFO_NUM;i++){
        if(rttask_info_list[i].conn.id!=0){
            rttask_info_list[i].need_send=1;            
        }
    }
}

void rttask_infosend_process(){
    rttask_infosend_loop();
    static tim_10hz=0;
    tim_10hz++;
    if(tim_10hz<3)
        return;
    tim_10hz=0;
    for(uint8_t i=0;i<MAX_SEND_RTTASKINFO_NUM;i++){
        if(rttask_info_list[i].need_send){
            rttask_info_list[i].need_send=0;
            rttask_host_info_send(rttask_info_list[i].task_id,i);
        }
    }
    memset(g_sys_val.rttask_musicset_f,0x00,MAX_MUSIC_CH);
}

void task_secinc_process(){
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(timetask_now.task_musicplay[i].play_state)
            timetask_now.task_musicplay[i].music_sec++;
    }
}

