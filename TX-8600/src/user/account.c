#include "account.h"
#include "ack_build.h"
#include "protocol_adrbase.h"
#include "user_xccode.h"
#include "list_instance.h"
#include "user_unti.h"
#include "list_contorl.h"
#include "bsp_ds1302.h"
#include "fl_buff_decode.h"
#include "debug_print.h"
#include "task_decode.h"
#include "user_lcd.h"
#include "conn_process.h"
#include "string.h"
#include "user_messend.h"
#include "eth_audio_config.h"


void filename_decoder(uint8_t *buff,uint8_t num){
    for(uint8_t i=0;i<num/2;i++){
        if(buff[i*2]==0 && buff[i*2+1]==0){
            break;
        }
        buff[i*2] = buff[i*2]^g_sys_val.sn_key[i*2];
        buff[i*2+1] = buff[i*2+1]^g_sys_val.sn_key[i*2+1];
    }
}

//===============================================================================
// �˻���¼����
//================================================================================
void account_login_recive(){
    //if(!(sn_cmp(host_info.sn,&xtcp_rx_buf[A_LOGIN_SN_B]))){ //�������
    //    user_sending_len = account_login_ack_build(02,0,null);
    //    user_xtcp_send(conn);
    //    return; //fail
    //}
    // ���ܵ�¼��
    filename_decoder(&xtcp_rx_buf[A_LOGIN_NAME_B],DIV_NAME_NUM);
    // ��������
    filename_decoder(&xtcp_rx_buf[A_LOGIN_SN_B],SYS_PASSWORD_NUM);
    //------------------------------------------------------------------------------------------------------------------------
    // ����
    for(uint8_t i=0;i<MAX_ACCOUNT_NUM;i++){
        //����û���ϸ��Ϣ
        account_fl_read(&tmp_union.account_all_info,i);
        // �ж��˻��Ƿ���Ч
        if(account_info[i].id==0xFF)
            continue;
        // �ж��û���
        if(!charncmp(&xtcp_rx_buf[A_LOGIN_NAME_B],tmp_union.account_all_info.account_info.name,DIV_NAME_NUM)){
            continue;
        }
        // �ж�����
        debug_printf("sn:");
        for(uint8_t i=0;i<SYS_PASSWORD_NUM;i++){
            debug_printf("%x,",tmp_union.account_all_info.account_info.sn[i]);
        }
        debug_printf("\n");
        for(uint8_t i=0;i<SYS_PASSWORD_NUM;i++){
            debug_printf("%x,",xtcp_rx_buf[A_LOGIN_SN_B+i]);
        }
        debug_printf("\n");
        if(!sn_cmp(&xtcp_rx_buf[A_LOGIN_SN_B],tmp_union.account_all_info.account_info.sn)){
            filename_decoder(&xtcp_rx_buf[A_LOGIN_SN_B],SYS_PASSWORD_NUM);
            user_sending_len = account_login_ack_build(02,0,null,ACCOUNT_LOGIN_CMD);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
            debug_printf("sn error\n");
            return; //fail
        }
        //��ȡ�˻���ϸ��Ϣ
        account_fl_read(&tmp_union.account_all_info,account_info[i].id);
        account_info[i].login_state = 1; // account login
        account_info[i].over_time =0;   
        //-----------------------------------------
        //��õ�¼ʱ��
        account_info[i].time_info = g_sys_val.time_info;
        account_info[i].date_info = g_sys_val.date_info;
        //
        user_sending_len = account_login_ack_build(0,i,&tmp_union.account_all_info.mac_list,ACCOUNT_LOGIN_CMD);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        // �����¼��Ϣ
        tmp_union.account_all_info.account_info = account_info[i];
        account_fl_write(&tmp_union.account_all_info,i);
        // ��ӽ���Ϣ����
        mes_list_add(conn,xtcp_rx_buf[POL_COULD_S_BASE],&xtcp_rx_buf[POL_ID_BASE]);
        //
        //debug_printf("login\n");
        return; //success
    }
    user_sending_len = account_login_ack_build(1,0,null,ACCOUNT_LOGIN_CMD);   //�˻�������
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    //debug_printf("login no\n");
    return; //fail
}

//===============================================================================
// �˻��б��ѯ
//================================================================================
void account_list_send(uint8_t id[],uint8_t could_f,uint8_t could_cmd){
    //�Ƿ����б����ڷ���
    if(conn_sending_s.id!=null)
        return;
    //-----------------------------------------------------------------
    conn_sending_s.conn_sending_tim = 0;
    // Send area list contorl
    conn_sending_s.ac_list.pack_tol=0;
    conn_sending_s.ac_list.pack_inc=0;
    conn_sending_s.ac_list.account_inc=0;
    conn_sending_s.ac_list.could_cmd = could_cmd;
    memcpy(conn_sending_s.ac_list.id,id,6);
    //
    conn_sending_s.conn_state |= AC_LIST_SENDING;
    //
    conn_sending_s.could_s = could_f;
    if(could_f){
        conn_sending_s.id = g_sys_val.could_conn.id;
    }
    else{
        conn_sending_s.id = conn.id;
    }
    //
    //------------------------------------------------
    // ��ȡ�ܰ���
    uint8_t total_user=0;
    //    
    for(uint8_t i=0;i<MAX_ACCOUNT_NUM;i++){
        if(account_info[i].id!=0xFF){
            total_user++;
        }
    }
    conn_sending_s.ac_list.pack_tol = total_user/10;
    if(total_user%10)
        conn_sending_s.ac_list.pack_tol++;
    //----------------------------------------------------------------------------------
    user_sending_len = account_list_ack_build();
}


void account_userlist_recive(){
    account_list_send(&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE],0);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//---------------------------------------------
// �б���������
//---------------------------------------------
void ac_list_sending_decode(){
    user_sending_len = account_list_ack_build();
    //��������
    if(conn_sending_s.ac_list.could_cmd){
        user_could_send(1);        
        debug_printf("send list\n");
    }
    //����ת��
    else{
        user_xtcp_send(conn,conn_sending_s.could_s);
    }
}


//===============================================================================
// �û�Ȩ���豸�б��ѯ
//================================================================================
void account_div_list_recive(){
    account_fl_read(&tmp_union.account_all_info,xtcp_rx_buf[A_MACLIST_ID_B]);
    user_sending_len = account_maclist_ack_build(&tmp_union.account_all_info);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    //debug_printf("div_list\n");
}

//===============================================================================
// �˻����ù���
//================================================================================
void account_config_recive(){
    uint8_t id=0xFF;
    uint8_t state = 0;
    uint8_t i;
    //---------------------------------------------------------------------------------
    uint16_t dat_base;
    dat_base = A_CONFIG_AC_DIV_MAC_B;
    //---------------------------------------------------------------------------------
    // ���ܵ�¼��
    filename_decoder(&xtcp_rx_buf[A_CONFIG_NAME_B],DIV_NAME_NUM);
    // ��������
    filename_decoder(&xtcp_rx_buf[A_CONFIG_AC_SN_B],SYS_PASSWORD_NUM);
    //-----------------------------------------------------
    // ��ȡ�༭ID
    if((xtcp_rx_buf[A_CONFIG_ACNUM_B] < MAX_ACCOUNT_NUM)&&(xtcp_rx_buf[A_CONFIG_CONTORL_B]!=0)){
        id = xtcp_rx_buf[A_CONFIG_ACNUM_B];
    }
    // �˻������ж� �ֻ��������ж�
    if(xtcp_rx_buf[A_CONFIG_CONTORL_B]!=2){
        for(uint8_t i=0;i<MAX_ACCOUNT_NUM;i++){
            debug_printf("id %d %d\n",account_info[i].id,id);
            if((account_info[i].id!=0xFF)&&((account_info[i].id!=id)||(xtcp_rx_buf[A_CONFIG_CONTORL_B]==0))){
                if(charncmp(account_info[i].name,&xtcp_rx_buf[A_CONFIG_NAME_B],DIV_NAME_NUM))
                    goto fail_account_config;
                for(uint8_t c=0;c<DIV_NAME_NUM;c++){
                    debug_printf("%x ,",account_info[i].phone_num[c]);
                }
                debug_printf("\n");
                for(uint8_t c=0;c<DIV_NAME_NUM;c++){
                    debug_printf("%x ,",xtcp_rx_buf[A_CONFIG_PHONE_NUM_B+c]);
                }
                debug_printf("\n");
                
                if((xtcp_rx_buf[A_CONFIG_PHONE_NUM_B]!=0 || xtcp_rx_buf[A_CONFIG_PHONE_NUM_B+1]!=0)&&
                    charncmp(account_info[i].phone_num,&xtcp_rx_buf[A_CONFIG_PHONE_NUM_B],DIV_NAME_NUM)
                  ){
                    debug_printf("ac phone same\n");
                    goto fail_account_config;
                }
            }
        }
    }
    //------------------------------------------------------------------
    // �½��˻�ID
    if(xtcp_rx_buf[A_CONFIG_CONTORL_B]==0){
        //�ҿ��˻�
        debug_printf("build ac \n");
        for(i=0; i<MAX_ACCOUNT_NUM; i++){
            if(account_info[i].id==0xFF){
                account_info[i].id=i;
                id=i;
                //���ô���ʱ�� ��¼ʱ��
                account_info[i].time_info = g_sys_val.time_info;
                account_info[i].date_info = g_sys_val.date_info;
                account_info[i].build_time_info = g_sys_val.time_info;
                account_info[i].build_date_info = g_sys_val.date_info;
                account_info[i].build_date_info = g_sys_val.date_info;
                break;
            }
        }
    } 
    debug_printf("config ac id %d\n",id);
    if(id==0){
        memcpy(account_info[id].sn,&xtcp_rx_buf[A_CONFIG_AC_SN_B],SYS_PASSWORD_NUM);
        state = 1;
        goto  ac_config_succes;
    }
    if(id!=0xFF){
        //--------------------------------------------------------------------------------------
        if(xtcp_rx_buf[A_CONFIG_CONTORL_B]==2){
            account_info[id].id=0xFF; //ɾ���豸����id 0xFF 
            tmp_union.account_all_info.account_info.id=0xFF;
            state = 1;
            goto  ac_config_succes;
        }    
        // ���豸��Ϣ
        memcpy(account_info[id].name,&xtcp_rx_buf[A_CONFIG_NAME_B],DIV_NAME_NUM);
        memcpy(account_info[id].phone_num,&xtcp_rx_buf[A_CONFIG_PHONE_NUM_B],DIV_NAME_NUM);
        account_info[id].type = xtcp_rx_buf[A_CONFIG_ACTYPE_B];
        memcpy(account_info[id].sn,&xtcp_rx_buf[A_CONFIG_AC_SN_B],SYS_PASSWORD_NUM);
        account_info[id].div_tol = xtcp_rx_buf[A_CONFIG_AC_DIVTOL_B];
        tmp_union.account_all_info.account_info = account_info[id];
        // ��mac�б���Ϣ
        uint16_t data_base=0;
        for(i=0; i<xtcp_rx_buf[A_CONFIG_AC_DIVTOL_B]; i++){
            memcpy(tmp_union.account_all_info.mac_list+data_base,&xtcp_rx_buf[A_CONFIG_AC_DIV_MAC_B+data_base],6);
            data_base+=6;    
        }
        state=1;
    }
    ac_config_succes:
    if(state){
        mes_send_acinfo(id);
        account_fl_write(&tmp_union.account_all_info,id);
    }
    fail_account_config:
    user_sending_len = account_control_ack_build(xtcp_rx_buf[A_CONFIG_CONTORL_B],id,state);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    //���������б�
    account_list_updat();
    debug_printf("account_config\n");
}

//===============================================================================
// ����������
//===============================================================================
void maschine_code_init(){
    uint16_t sum_tmp=0;
    uint8_t key_tmp;
    g_sys_val.maschine_code[0] = 0x69;
    g_sys_val.maschine_code[1] = 0x74;
    g_sys_val.maschine_code[2] = 0x63;
    memcpy(&g_sys_val.maschine_code[3],host_info.mac,6);
    //
    for(uint8_t i=0;i<9;i++){
        sum_tmp += g_sys_val.maschine_code[i];
    }
    key_tmp = (sum_tmp&0xff)+(sum_tmp>>8);
    //ȡ��
    for(uint8_t i=0;i<9;i++){
        g_sys_val.maschine_code[i] = g_sys_val.maschine_code[i]^key_tmp;
    }
    key_tmp ^=0xFF;
    //
    g_sys_val.maschine_code[9] = key_tmp;
}

//===============================================================================
// ϵͳע���ѯ
//================================================================================
void register_could_chk(){
    user_sending_len = cld_resigerchk_build();
    user_could_send(1);
    debug_printf("chk regsier\n");
}


//===============================================================================
// ϵͳע����Ϣ���� BE02
//================================================================================
void account_sys_register_recive(){
    uint8_t key_tmp;
    uint8_t maschien_code[10];
    key_tmp = (xtcp_rx_buf[POL_DAT_BASE+13]^0xFF);
    debug_printf("dat:");
    for(uint8_t i=0;i<13;i++){
        debug_printf("%x,",xtcp_rx_buf[POL_DAT_BASE+i]^key_tmp);
    }
    debug_printf("\n");
    for(uint8_t i=0;i<10;i++){
        maschien_code[i] = (xtcp_rx_buf[POL_DAT_BASE+i]^key_tmp);
    }
    debug_printf("\n");
    //�Ƚϻ�����
    if(charncmp(maschien_code,g_sys_val.maschine_code,10)){
        host_info.regiser_state = xtcp_rx_buf[POL_DAT_BASE+10]^key_tmp;
        debug_printf("BE02 rec %d \n",host_info.regiser_state );
        // δע��
        if((host_info.regiser_state==0)&&(host_info.regiser_days == 0)){
            host_info.regiser_state = 0;
        }
        // ������ע��
        else if(host_info.regiser_state==1){
            host_info.regiser_state = 2;
        }
        // ������ע��
        else if((host_info.regiser_state==2)&&(host_info.regiser_days==0)){
            host_info.regiser_state = 0;
        }
        else{
            host_info.regiser_state = 1;
        }
        host_info.regiser_days = (xtcp_rx_buf[POL_DAT_BASE+11]^key_tmp)|((xtcp_rx_buf[POL_DAT_BASE+12]^key_tmp)<<8);
        // �����ע����Ϣ
        if(g_sys_val.register_need_send){
            user_sending_len = cld_appregsied_request_build();   
            if(g_sys_val.regsiter_conn.id!=0)
                user_xtcp_send(g_sys_val.regsiter_conn,g_sys_val.register_could_f);
            g_sys_val.regsiter_conn.id = 0;
            g_sys_val.register_need_send = 0;
            debug_printf("B90D resend %d %d\n",g_sys_val.register_rec_s_tmp,host_info.regiser_state);
        }
        debug_printf("regsied updat state %d, day %d\n\n",host_info.regiser_state,host_info.regiser_days);
        // flash info 
        hostinfo_fl_write();    //��д������Ϣ
    }
}

//===============================================================================
// ��ע������    BE08
//================================================================================
void cld_register_request(){
    user_sending_len = cld_resiger_request_build();
    user_could_send(1);    
    debug_printf("register request\n");
}

//===============================================================================
// ��ע������ظ� BE08
//===============================================================================
void cld_register_recive(){
    //����ע��״̬
    g_sys_val.register_rec_s_tmp = xtcp_rx_buf[POL_DAT_BASE];
    g_sys_val.register_need_send = 1;
    debug_printf("reg BE08 rec %d\n",g_sys_val.register_rec_s_tmp);
    //����ע��״̬
    register_could_chk();
    
    /*
    user_sending_len = onebyte_ack_build(xtcp_rx_buf[POL_DAT_BASE],APP_REGISTER_CONTORL);    
    if(g_sys_val.regsiter_conn.id!=0)
        user_xtcp_send(g_sys_val.regsiter_conn,0);
    g_sys_val.regsiter_conn.id = 0;
    debug_printf("register request %d\n",xtcp_rx_buf[POL_DAT_BASE]);
    */
}

//================================================================================
// �ֻ�ע������ B90D
//================================================================================
void app_register_request(){
    g_sys_val.regsiter_conn = conn;
    g_sys_val.register_could_f = xtcp_rx_buf[POL_COULD_S_BASE];
    memcpy(g_sys_val.register_code,&xtcp_rx_buf[POL_DAT_BASE],10);
    memcpy(g_sys_val.register_could_id,&xtcp_rx_buf[POL_ID_BASE],6);
    debug_printf("mach code ");
    for(uint8_t i=0;i<10;i++){
        debug_printf("%x ",g_sys_val.maschine_code[i]);
    }
    debug_printf("\n");
    for(uint8_t i=0;i<10;i++){
        debug_printf("%x ",g_sys_val.register_code[i]);
    }
    debug_printf("\n");
    cld_register_request();
}

//===============================================================================
// �˺��б�����
//===============================================================================
void account_list_updat(){
    if(g_sys_val.could_conn.id==0)
        return;
    account_list_send(&xtcp_rx_buf[POL_ID_BASE],1,1);
    user_could_send(1);  
}

//===============================================================================
// ʱ��ͬ������ BE03
//================================================================================
void cld_timesysnc_request(){
    user_sending_len = cld_timesysnc_request_build();
    user_could_send(1);    
    debug_printf("time request\n");
}

//===============================================================================
// �˺��Ƶ�¼ BE06
//================================================================================
void cld_account_login_recive(){
    if(xtcp_rx_buf[POL_DAT_BASE])
        return;
    //�жϵ�¼
    for(uint8_t i=0;i<MAX_ACCOUNT_NUM;i++){
        if(charncmp(&xtcp_rx_buf[POL_DAT_BASE+1],account_info[i].phone_num,DIV_NAME_NUM)){
            //��ȡ�˻���ϸ��Ϣ
            account_fl_read(&tmp_union.account_all_info,account_info[i].id);
            account_info[i].login_state = 1; // account login
            account_info[i].over_time =0;   
            //-----------------------------------------
            //��õ�¼ʱ��
            account_info[i].time_info = g_sys_val.time_info;
            account_info[i].date_info = g_sys_val.date_info;
            //�������⴦��            
            memcpy(&xtcp_rx_buf[AC_LOGIN_NAME_B],account_info[i].name,DIV_NAME_NUM);
            //--------------------------------------------------------------------
            user_sending_len = account_login_ack_build(0,i,&tmp_union.account_all_info.mac_list,CLD_CLOULDLOGIN_CMD);
            debug_printf("could len %d\n",user_sending_len);
            user_could_send(0);
            return;
        }
    }   
    // �Ƶ�¼��Ϣ����
    
    user_sending_len = account_login_ack_build(1,0,null,CLD_REGISTER_INFO_CMD); 
    debug_printf("could len %d\n",user_sending_len);
    user_could_send(0);    
}


//================================================================================
// ϵͳ/�˻����߲�ѯ
//================================================================================
uint8_t sysonline_recive(){
    uint8_t state=0;
    state = mes_list_add(conn,xtcp_rx_buf[POL_COULD_S_BASE],&xtcp_rx_buf[POL_ID_BASE]);
    return state;
}

// ϵͳ/�˻����߲�ѯ
void account_sysonline_recive(){
    user_sending_len = sysonline_chk_build(sysonline_recive());
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

// �ֻ���������
void app_sysonline_recive(){
    sysonline_recive();
}


//================================================================================
// ��˷��ѯ�˻��б� ���½
//================================================================================
void mic_userlist_chk_recive(){
    
    debug_printf("user list chk \n");

    for(uint8_t i=0;i<MAX_ACCOUNT_NUM;i++){
        //����û���ϸ��Ϣ
        account_fl_read(&tmp_union.account_all_info,i);
        // �ж��û�
        if(!charncmp(&xtcp_rx_buf[A_LOGIN_NAME_B],tmp_union.account_all_info.account_info.name,DIV_NAME_NUM)){
            continue;
        }
        
        // �ж�����
        if(!sn_cmp(&xtcp_rx_buf[A_LOGIN_SN_B],tmp_union.account_all_info.account_info.sn)){
            user_sending_len = mic_userlist_ack_build(0,&tmp_union.account_all_info);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
            return; //fail
        }    
        uint8_t state=1;
        if(tmp_union.account_all_info.account_info.type == ADMIN_USER_TYPE)
            state=2; 
        //
        user_sending_len = mic_userlist_ack_build(state,&tmp_union.account_all_info);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        // ������Ϣ����
        mes_list_add(conn,xtcp_rx_buf[POL_COULD_S_BASE],&xtcp_rx_buf[POL_ID_BASE]);
    }
}


//===============================================================================
// ʱ��ͬ��
//===============================================================================
void time_sync_deocde(uint8_t could_s){
    if((xtcp_rx_buf[USER_TIMSYNC_DAY_B]==0) ||(xtcp_rx_buf[USER_TIMSYNC_MONTH_B]==0)||(xtcp_rx_buf[USER_TIMSYNC_WEEK_B]==0)){
        if(could_s==0){
            user_sending_len = onebyte_ack_build(0,USER_TIMER_SYNC_CMD,&xtcp_rx_buf[POL_ID_BASE]);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        }    
        return;
    }
    g_sys_val.date_info.year = xtcp_rx_buf[USER_TIMSYNC_YEAR_B];
    g_sys_val.date_info.month = xtcp_rx_buf[USER_TIMSYNC_MONTH_B];
    g_sys_val.date_info.date = xtcp_rx_buf[USER_TIMSYNC_DAY_B];
    g_sys_val.time_info.hour = xtcp_rx_buf[USER_TIMSYNC_HOUR_B];
    g_sys_val.time_info.minute = xtcp_rx_buf[USER_TIMSYNC_MINUTE_B];
    g_sys_val.time_info.second = xtcp_rx_buf[USER_TIMSYNC_SECOND_B];
    g_sys_val.date_info.week = xtcp_rx_buf[USER_TIMSYNC_WEEK_B];
    g_sys_val.today_date = g_sys_val.date_info;
    //
    ds1302_date_set();
    ds1302_time_set();
    //
    if(could_s==0){
        user_sending_len = onebyte_ack_build(1,USER_TIMER_SYNC_CMD,&xtcp_rx_buf[POL_ID_BASE]);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    }
    //�жϷ����Ƿ���Ч������
    for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
        if(solution_list.solu_info[i].en==1)
            solution_data_chk(i);
            g_sys_val.need_flash |= NEED_FL_SOLUTION;
    }
    //
    create_todaytask_list(g_sys_val.time_info);
    //
    user_disp_time();

    user_disp_data();
}

void user_timer_sync_recive(){
    time_sync_deocde(0);
}

void cld_timer_sync_recive(){
    time_sync_deocde(1);
}


//===============================================================================
// ��Ͳ������� B503
//===============================================================================
#if 0
void mic_aux_request_recive(){
    uint8_t aux_type,state,ch_tmp,tol_type;
    //
    aux_type = xtcp_rx_buf[POL_DAT_BASE+1];
    //debug_printf("aux req %x\n",xtcp_rx_buf[POL_DAT_BASE]);
    //debug_printf("aux type %x\n" ,xtcp_rx_buf[POL_DAT_BASE+1]);
    // �ر�ͨ��
    if(xtcp_rx_buf[POL_DAT_BASE]){
        tol_type = aux_type>>4;
        g_sys_val.aux_ch_state[(tol_type*AUX_RXCH_NUM) + aux_type&0x0F] = 0;
        ch_tmp = aux_type;
        state = 0;
    }
    // ����ͨ��
    else{
        state = 1;
        for(uint8_t i=1;i<AUX_RXCH_NUM;i++){
            if(g_sys_val.aux_ch_state[aux_type*AUX_RXCH_NUM+i]==0){
                g_sys_val.aux_ch_state[aux_type*AUX_RXCH_NUM+i] = 1;
                g_sys_val.aux_ch_tim[aux_type*AUX_RXCH_NUM+i] = 0; 
                memcpy(g_sys_val.aux_ch_ip[aux_type*AUX_RXCH_NUM+i],conn.remote_addr,4);
                state = 0;
                ch_tmp = (aux_type<<4)|i;
                break;
            }
            else if(charncmp(conn.remote_addr,g_sys_val.aux_ch_ip[aux_type*AUX_RXCH_NUM+i],4)){
                state = 0;
                ch_tmp = (aux_type<<4)|i;
                break;
            }
        }
    }
    debug_printf("MIC request busy %d ch:%x\n",state,ch_tmp);
    user_sending_len = threebyte_ack_build(xtcp_rx_buf[POL_DAT_BASE],state,ch_tmp,MIC_AUX_REQUEST_CMD);    
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}   
#endif
//===============================================================================
// ��Ͳͨ�������� B504
//===============================================================================
#if 0
void mic_aux_heart_recive(){
    uint8_t aux_type;
    aux_type = xtcp_rx_buf[POL_DAT_BASE];
    g_sys_val.aux_ch_tim[((aux_type>>4)*AUX_RXCH_NUM) + aux_type&0x0F] = 0;
}
#endif
//===============================================================================
// ��Ͳ��ʱ�رմ���
//===============================================================================
#if 0
void mic_time1hz_close(){
    for(uint8_t i=0;i<AUX_TYPE_NUM*AUX_RXCH_NUM;i++){
        if(g_sys_val.aux_ch_state[i]){
            g_sys_val.aux_ch_tim[i]++;
            if(g_sys_val.aux_ch_tim[i]>3){
                g_sys_val.aux_ch_state[i]=0;
            }
        }
    }
}
#endif
//===============================================================================
// ������������   B906
//===============================================================================
void user_host_search_recive(){
    user_sending_len = onebyte_ack_build(1,HOST_SEARCH_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//===============================================================================
// ��ʱIP����   BF09
//===============================================================================
void tmp_ipset_recive(){
    if(xtcp_rx_buf[TMP_IPSET_STATE])
        return;
    if(mac_cmp(&xtcp_rx_buf[TMP_IPSET_MAC],host_info.mac)){
        memcpy(tmp_union.ipconfig.ipaddr,&xtcp_rx_buf[TMP_IPSET_IP],4);
        memcpy(tmp_union.ipconfig.netmask,&xtcp_rx_buf[TMP_IPSET_MASK],4);
        memcpy(tmp_union.ipconfig.gateway,&xtcp_rx_buf[TMP_IPSET_GATE],4);
        //
        user_sending_len = onebyte_ack_build(1,TMP_IPSET_CMD,&xtcp_rx_buf[POL_ID_BASE]);
        user_xtcp_send(g_sys_val.broadcast_conn,0);
        //
        user_xtcp_ipconfig(tmp_union.ipconfig);
    }
}

//===============================================================================
// ����IP����   BF0B
//===============================================================================
#if 0
void sysset_ipset_recive(){
    if(xtcp_rx_buf[SYSSET_IPSET_SENDSTATE]==1){
        return;
    }
    if(charncmp(&xtcp_rx_buf[SYSSET_IPSET_DESMAC],host_info.mac,6)==0){
        return;
    }
    if(xtcp_rx_buf[SYSSET_IPSET_CONFIG_F]==0){
        return;
    }
    if(xtcp_rx_buf[SYSSET_IPSET_IP_MODE]){
        return;
    }
    memcpy(host_info.ipconfig.ipaddr,&xtcp_rx_buf[SYSSET_IPSET_IP],4);    
    memcpy(host_info.ipconfig.gateway,&xtcp_rx_buf[SYSSET_IPSET_GATE],4);    
    memcpy(host_info.ipconfig.netmask,&xtcp_rx_buf[SYSSET_IPSET_MASK],4);    
    user_xtcp_ipconfig(host_info.ipconfig);    
}
#endif

//================================================================================
// �˻����ӳ�ʱ����
//================================================================================
#define account_overtime 30 //30��

void account_login_overtime(){
    for(uint8_t i=0;i<MAX_ACCOUNT_NUM; i++){
        account_info[i].over_time++;
        if((account_info[i].login_state)&&(account_info[i].over_time > account_overtime)){
            account_info[i].login_state=0;
            account_fl_read(&tmp_union.account_all_info,i);
            tmp_union.account_all_info.account_info = account_info[i];
            account_fl_write(&tmp_union.account_all_info,i);
            debug_printf("account logout\n");
        }
    }
}

//===============================================================================
// �ָ�������æ��ѯ
//===============================================================================
void backup_busy_chk(){
    // ��æ
    if(g_sys_val.tftp_busy_f){
        user_sending_len = onebyte_ack_build(2,BACKUP_BUSY_CHK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    }
    else if(g_sys_val.backup_busy_f){
        user_sending_len = onebyte_ack_build(1,BACKUP_BUSY_CHK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    }
    // ����
    else{
        user_sending_len = onebyte_ack_build(0,BACKUP_BUSY_CHK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);    
    }
}

//===============================================================================
// �ָ����ݿ�ʼ����
//===============================================================================
void backup_contorl_chk(){
    uint8_t data[64];
    if(g_sys_val.backup_busy_f){
        user_sending_len = backup_contorl_build(2,data);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    }
    // ��ʼ�ָ� ����
    else{
        if(start_sysdat_backup()){
            g_sys_val.backup_bar=0;
            g_sys_val.backup_resend = 0;
            g_sys_val.backup_resend_inc = 0;
            g_sys_val.backup_busy_f = 1;
            g_sys_val.backup_bar = 0;
            g_sys_val.backup_conn = conn;
            user_sending_len = backup_contorl_build(1,data);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        }
        else{//�ļ�����
            user_sending_len = backup_contorl_build(0,data);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        }

    }
}

//===============================================================================
// �ָ�������Ϣ����
//===============================================================================
void backup_mes_send_recive(){
    if(g_sys_val.backup_resend){
        g_sys_val.backup_busy_f = 0;
        g_sys_val.backup_resend = 0;
        user_sending_len = backup_updata_build(2,g_sys_val.backup_bar);
        user_xtcp_send(g_sys_val.backup_conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        g_sys_val.reboot_f = 1;
    }
}

void backup_sendmes_10hz(){
    uint8_t state;
    if(g_sys_val.backup_busy_f==0)
        return;
    g_sys_val.backup_timechk++;
    if(g_sys_val.backup_timechk>5){
        //
        backup_system_chk(&state,&g_sys_val.backup_bar);
        //
        g_sys_val.backup_timechk = 0;
        if(state){
            g_sys_val.backup_resend = 1;
            g_sys_val.backup_resend_inc++;
            if(g_sys_val.backup_resend_inc>3){
                g_sys_val.backup_busy_f=0;
            }    
            user_sending_len = backup_updata_build(2,g_sys_val.backup_bar);
            user_xtcp_send(g_sys_val.backup_conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        }
        else{
            user_sending_len = backup_updata_build(1,g_sys_val.backup_bar);
            user_xtcp_send(g_sys_val.backup_conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        }
    }
}



