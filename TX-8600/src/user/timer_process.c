#include "timer_process.h"
#include "list_instance.h"
#include "list_contorl.h"
#include "user_xccode.h"
#include "user_unti.h"
#include "divlist_decode.h"
#include "bsp_ds1302.h"
#include "conn_process.h"
#include "account.h"
#include "flash_adrbase.h"
#include "fl_buff_decode.h"
#include "task_decode.h"
#include "user_lcd.h"
#include "user_messend.h"
#include "user_file_contorl.h"
#include "could_serve.h"

#include "debug_print.h"

//------------------------------------------------------------------------------
// ���ڼ�ʱ�߳�
//------------------------------------------------------------------------------
void timer_process(){
	uint8_t tmp;
    static uint8_t get_f=3;
    g_sys_val.time_info.second++;
    if(g_sys_val.time_info.second>(60-1)){
        g_sys_val.time_info.second=0;
        g_sys_val.time_info.minute++;	
        if(g_sys_val.time_info.minute>(60-1))
        {
            g_sys_val.time_info.minute=0;
            g_sys_val.time_info.hour++;
            cld_timesysnc_request();
            //ÿ��23��ͬ��ʱ�� ����ע��
            if(g_sys_val.time_info.hour==23){
                get_f = 0; 
            }
            if(g_sys_val.time_info.hour>(24-1)){
                get_f=0;
                ds1302_get_date();
                g_sys_val.time_info.hour=0;
                //
                // �жϷ�������
                for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
                    if((solution_list.solu_info[i].en==1)&&(solution_list.solu_info[i].state!=0xFF))
                        solution_data_chk(i);
                        g_sys_val.need_flash |= NEED_FL_SOLUTION;
                }
                //
                g_sys_val.today_date = g_sys_val.date_info;
                //
                create_todaytask_list(g_sys_val.time_info);
                // ע�������ж�
                if(host_info.regiser_days!=0){
                    host_info.regiser_days--;
                    // ��������
                    if((host_info.regiser_days==0)&&(host_info.regiser_state==1)){
                        host_info.regiser_state = 0;
                    }
                    hostinfo_fl_write();
                }
                g_sys_val.today_date = g_sys_val.date_info;
            }
        }    
    }
    if(get_f<3){
        get_f++;
        ds1302_get_date();
        if(get_f==3){
            mes_send_listinfo(TODAYTASK_INFO_REFRESH,1);
            user_disp_data();
            // ����ע����Ϣ
            register_could_chk();
        }
    }
}

//===========================================================================

//---------------------------------------------------------------------------
//10hz ����߳�
//---------------------------------------------------------------------------
void timee10hz_process(){
    mes_send_overtime();        // �����ϴ����ͻ�����Ϣͬ��
    task_10hz_mutich_play();    // �����������Ŵ���
    timer_flash_dat_process();  // flash ��ʱ��¼�����߳�
    task_dtinfo_overtime_recive_close(); //�ر���ϸ��Ϣ ����
    disp_task_delay();          // ����ˢ����ʾ
    bat_filecontorl_resend_tim(); //���ֻظ��ط�
    rttask_build_overtime10hz(); //��ʱ������״̬��ʱ
    backup_sendmes_10hz();      //������Ϣ����
    divfound_over_timeinc();    //�豸������ʱ
    rttask_playlist_updata();   //��ʱ���񲥷��б����
    xtcp_bufftimeout_check_10hz();//����buff���ͳ�ʱ
    list_sending_overtime();    //�б��ͳ�ʱ
}

//--------------------------------------------------------------------------
// 1hz ����߳�
//--------------------------------------------------------------------------
void second_process(){
    timer_process();            //ϵͳʱ��
    if(g_sys_val.eth_link_state){
        conn_overtime_close();  // conn���ӳ�ʱ
        div_heart_overtime_close(); //�������߳�ʱ
        account_login_overtime();   //�˺ŵ��볬ʱ
        timer_rttask_run_process(); //��ʱ����ʱ
        #if COULD_TCP_EN
        could_heart_send_timer();   //�Ʒ�������������������ʱ
        dns_twominute_chk();
        #endif
    }
    broadcast_for_minute();     //ÿ���ӷ��͹㲥����ʱ
    timer_taskmusic_check();    //��ʱ���񲥷ż�ʱ
    task_check_and_play();      //��ʱ����ʱ����
    user_disp_time();           // lcd ʱ����ʾ
    timer_task_disp();          // ��ʾ�л�
}

