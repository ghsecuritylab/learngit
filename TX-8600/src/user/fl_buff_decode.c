#include "fl_buff_decode.h"
#include "user_xccode.h"
#include "protocol_adrbase.h"
#include "flash_adrbase.h"
#include "sys_config_dat.h"
#include "list_contorl.h"
#include "user_unti.h"
#include "sys_log.h"

#include "debug_print.h"
#include "string.h"

static uint8_t divinfo_len = sizeof(div_node_t);

//===================================================================
// �û����� �����д����
//===================================================================
void sys_dat_read(uint8_t buff[],uint16_t num,uint16_t base_adr){
    memcpy(buff,g_tmp_union.buff+base_adr,num);
}

void sys_dat_write(uint8_t buff[],uint16_t num,uint16_t base_adr){
    memcpy(g_tmp_union.buff+base_adr,buff,num);
}

//----------------------------------------------------------------------------
// ��д������Ϣflash
//---------------------------------------------------------------------------
void fl_hostinfo_write(){
    user_fl_sector_read(SYSTEM_0_DAT_SECTOR_BASE);
    sys_dat_write((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);
    user_fl_sector_write(SYSTEM_0_DAT_SECTOR_BASE);
    //
    user_fl_sector_read(SYSTEM_1_DAT_SECTOR_BASE);
    sys_dat_write((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);
    user_fl_sector_write(SYSTEM_1_DAT_SECTOR_BASE);
}
//----------------------------------------------------------------------------
// ��ȡ������Ϣflash
//---------------------------------------------------------------------------
void fl_hostinfo_read(){
    unsigned init_string;
    user_fl_sector_read(SYSTEM_0_DAT_SECTOR_BASE);
	sys_dat_read((char*)(&init_string),4,FLASH_ADR_INIT);  
    //��0ҳ���ݳ���
    if(init_string!=FLASH_INIT_F){
        // ��ȡҳ1���� ͬ��ҳ0����
        user_fl_sector_read(SYSTEM_1_DAT_SECTOR_BASE);
        sys_dat_read((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);//������Ϣ��ȡ
        user_fl_sector_write(SYSTEM_0_DAT_SECTOR_BASE);
    }
    else{
        // ��ȡ�û����� ͬ��ҳ1����
        sys_dat_read((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);//������Ϣ��ȡ
        user_fl_sector_write(SYSTEM_1_DAT_SECTOR_BASE);
    }
}

//----------------------------------------------------------------------------
// �ж�������Ϣ�Ƿ���Ҫ��λ
//---------------------------------------------------------------------------
uint8_t read_hostinfo_reset_state(){
    unsigned init_string;
    user_fl_sector_read(SYSTEM_0_DAT_SECTOR_BASE);
	sys_dat_read((char*)(&init_string),4,FLASH_ADR_INIT);   
    if(init_string!=FLASH_INIT_F){
        user_fl_sector_read(SYSTEM_1_DAT_SECTOR_BASE);
    	sys_dat_read((char*)(&init_string),4,FLASH_ADR_INIT);  
        if(init_string!=FLASH_INIT_F){
            return 0;
        }
    }
    return 1;
}

//----------------------------------------------------------------------------
// ��ʼ����ȡ������Ϣ
//---------------------------------------------------------------------------
void fl_hostinfo_init(){
    // ����sectorͬʱ�����û���Ϣ
    unsigned init_string;
    // ��ȡ��0ҳ��Ϣ
    user_fl_sector_read(SYSTEM_0_DAT_SECTOR_BASE);
	sys_dat_read((char*)(&init_string),4,FLASH_ADR_INIT);
    // ��0ҳ��Ϣ�Ƿ�������
    if(init_string!=FLASH_INIT_F){
        // ��ȡ��1ҳ��Ϣ
        user_fl_sector_read(SYSTEM_1_DAT_SECTOR_BASE);
        sys_dat_read((char*)(&init_string),4,FLASH_ADR_INIT);   
    }
    // �����ʼ����Ϣ
	init_string = 0x5AA57349;
	sys_dat_write((char*)(&init_string),4,FLASH_ADR_INIT);
   //
    sys_dat_read((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);//������Ϣ��ȡ
    // �ж�MAC��ַ�Ƿ��Ѿ���¼
    if((host_info.mac[0]==0x42)&&(host_info.mac[1]==0x4C)&&(host_info.mac[2]==0x45)){
        // �Ѿ���¼��¼MAC ʹ��FLASH����
        memcpy(&host_info_tmp,&host_info,sizeof(host_info_t));
    }    
    if(host_info.mac_write_f==0xAB){
        memcpy(&host_info_tmp,&host_info,sizeof(host_info_t));
    }
    // ����˺���Ϣ����
    memcpy(&host_info,&host_info_tmp,sizeof(host_info_t));
    // ��¼��ҳ����
	sys_dat_write((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);
    user_fl_sector_write(SYSTEM_0_DAT_SECTOR_BASE);
    user_fl_sector_write(SYSTEM_1_DAT_SECTOR_BASE);
}

//===================================================================
// �˻���Ϣ      �����д����
//===================================================================
// д�˻���Ϣ
// ���˻���ϸ��Ϣ
void fl_account_read(account_all_info_t *account_all_info,uint8_t id){
    user_fl_sector_read(ACCOUT_SECTOR+id);
    sys_dat_read(account_all_info,sizeof(account_all_info_t),FLB_ACCOUNT_DAT_BASE);
}

void fl_account_write(account_all_info_t *account_all_info,uint8_t id){
    unsigned len;
    len = sizeof(account_all_info_t);
    sys_dat_write(account_all_info,len,FLB_ACCOUNT_DAT_BASE);
    user_fl_sector_write(ACCOUT_SECTOR+id);
}

//===================================================================
// �������ݶ�д����
//===================================================================
// д���� 
void fl_area_write(){
    memcpy(g_tmp_union.buff,&area_info,sizeof(area_info_t)*MAX_AREA_NUM);
    user_fl_sector_write(AREA_INFOLIST_SECTOR);
}
// ��ʱ��
void fl_area_read(){
    user_fl_sector_read(AREA_INFOLIST_SECTOR);
    memcpy(&area_info,g_tmp_union.buff,sizeof(area_info_t)*MAX_AREA_NUM);    
}
//===================================================================
// �豸�б��д����
//===================================================================
// ��ʱ��
void fl_divlist_read(){
    uint16_t divlist_inc=0;
    uint16_t buf_adrbase=0;
    uint8_t sector_num=0;
    //
    // ��ʼ��ָ��
    div_list.div_head_p=null;
    div_list.div_end_p=null;
    while(divlist_inc<MAX_DIV_LIST){
        // ����sector
        user_fl_sector_read(DIV_INFOLIST_SECTOR+sector_num);
        buf_adrbase = 0;
        // ȡ���� 100��byteһ���豸 ��һ��sector��40���豸
        for(uint16_t i=0;(i<30)&&(divlist_inc<MAX_DIV_LIST);i++,divlist_inc++){
            memcpy(&div_list.div_node[divlist_inc],g_tmp_union.buff+buf_adrbase,divinfo_len);
            buf_adrbase+=130;   // 100��byteһ���豸 ��һ��sector��30���豸
        }
        sector_num++;
    }//while    
    for(uint8_t i=0;i<MAX_DIV_LIST;i++){

        // ��λ�豸���߱�־
        div_list.div_node[i].div_info.div_state = 0;
        div_list.div_node[i].div_info.div_onlineok = 0;

        div_list.div_node[i].next_p = null;
        if(div_list.div_node[i].div_info.id!=0xFF){
            // ��ֹFLASH ����
            if(div_list.div_node[i].div_info.id!=i){
                div_list.div_node[i].div_info.id = 0xFF;
                i++;
                continue;
            }
            div_node_creat_forid(i);
        }
    }
    //-----------------------------------------------------------------------
    #if 0
    div_node_t *tmp_p = div_list.div_head_p;
    while(tmp_p!=null){
       debug_printf("div:%x,%x,%x,%x,%x,%x\n",tmp_p->div_info.mac[0],tmp_p->div_info.mac[1],tmp_p->div_info.mac[2],
                                              tmp_p->div_info.mac[3],tmp_p->div_info.mac[4],tmp_p->div_info.mac[5]);
       tmp_p = tmp_p->next_p;
    }
    #endif
    //-----------------------------------------------------------------------
}

//=====================================================================
// �豸�б�棬дһ��setor
uint8_t fl_divlist_onesector(){
    static uint8_t sector_num=0;
    uint16_t buf_adrbase=0;
    uint8_t  tmp=0;
    if(g_sys_val.fl_divlist_inc==0)
        sector_num=0;
    //
    // д���ݻ��� 130��byteһ���豸 ��һ��sector��30���豸
    for(uint16_t i=0;(i<30)&&(g_sys_val.fl_divlist_inc<MAX_DIV_LIST);i++,g_sys_val.fl_divlist_inc++){
        memcpy(g_tmp_union.buff+buf_adrbase,&div_list.div_node[g_sys_val.fl_divlist_inc],divinfo_len);
        buf_adrbase+=130;   // 130��byteһ���豸 ��һ��sector��30���豸
    }
    if(g_sys_val.fl_divlist_inc==MAX_DIV_LIST){    //��д���
        tmp = 1;
    }
    user_fl_sector_write(DIV_INFOLIST_SECTOR+sector_num);
    sector_num++;
    return tmp;
}
//------------------------------------------------------------
// �����豸�б�
//------------------------------------------------------------
void fl_divlist_write(){
    g_sys_val.fl_divlist_inc=0;
    while(!fl_divlist_onesector()); //��д
}
//-----------------------------------------------------------------------------------
// ��д������Ϣ
//-----------------------------------------------------------------------------------
void fl_solution_write(){
    user_fl_sector_read(SOLUSION_DAT_SECTOR);
    sys_dat_write((char*)(&solution_list),sizeof(solution_list_t),FLASH_SOLUSION_LIST);
    user_fl_sector_write(SOLUSION_DAT_SECTOR);
    //
}

void fl_solution_read(){
	user_fl_sector_read(SOLUSION_DAT_SECTOR);
	sys_dat_read((char*)(&solution_list),sizeof(solution_list_t),FLASH_SOLUSION_LIST); //������Ϣ��ȡ
	//��Ч��������
	for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
        if(solution_list.solu_info[i].state!=0xFF && solution_list.solu_info[i].id!=i){
            solution_list.solu_info[i].state=0xFF;
        }
    }
}
//==========================================================================================
// ����
//--------------------------------------------------------------------
// ����ʱ����ָ��
void fl_timertask_read(task_allinfo_tmp_t     *task_allinfo_tmp,uint16_t id){
    user_fl_sector_read(TIMED_TASK_SECTOR+id);
    sys_dat_read(task_allinfo_tmp,sizeof(task_allinfo_tmp_t),FLB_TASK_DAT_BASE);
}

//--------------------------------------------------
// д��ʱ����ָ��
//--------------------------------------------------
void fl_timertask_write(task_allinfo_tmp_t *task_allinfo_tmp, uint16_t id){
    sys_dat_write(task_allinfo_tmp,sizeof(task_allinfo_tmp_t),FLB_TASK_DAT_BASE);
    user_fl_sector_write(TIMED_TASK_SECTOR+id);
}
//--------------------------------------------------
// ����ʱ����ָ��
//--------------------------------------------------
void fl_rttask_read(rttask_dtinfo_t     *rttask_dtinfo,uint16_t id){
    if(id>MAX_RT_TASK_NUM-1)
        return;
    user_fl_sector_read(RT_TASK_SECTOR+id*2);
    user_fl_sector_read2sector(RT_TASK_SECTOR+id*2+1);
    sys_dat_read(rttask_dtinfo,sizeof(rttask_dtinfo_t),FLB_RTTASK_DAT_BASE);
}
//--------------------------------------------------
// д��ʱ����ָ��
//--------------------------------------------------
void fl_rttask_write(rttask_dtinfo_t *rttask_dtinfo,uint16_t id){
    if(id>MAX_RT_TASK_NUM-1)
        return;
    sys_dat_write(rttask_dtinfo,sizeof(rttask_dtinfo_t),FLB_RTTASK_DAT_BASE);
    user_fl_sector_write(RT_TASK_SECTOR+id*2);
    user_fl_sector_write2sector(RT_TASK_SECTOR+id*2+1);
}

//----------------------------------------------------------

