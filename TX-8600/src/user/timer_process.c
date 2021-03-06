#include "timer_process.h"
#include "sys_config_dat.h"
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
#include "sys_log.h"
#include "debug_print.h"

//------------------------------------------------------------------------------
// 日期计时线程
//------------------------------------------------------------------------------
void timer_process(){
    static uint8_t get_f=3;
    static uint8_t need_mklog=1;
    g_sys_val.time_info.second++;
    
    if(g_sys_val.time_info.second>(60-1)){
        g_sys_val.time_info.second=0;
        g_sys_val.time_info.minute++;	
        if(g_sys_val.time_info.minute>(60-1))
        {
            g_sys_val.time_info.minute=0;
            g_sys_val.time_info.hour++;
            cld_timesysnc_request();
            //每天23点同步时间 更新注册
            if(g_sys_val.time_info.hour==23){
                get_f = 0; 
            }
            if(g_sys_val.time_info.hour>(24-1)){
                get_f=0;
                need_mklog=0;
                g_sys_val.time_info.hour=0;
                delay_milliseconds(1);
                // 同步ds1302时间
                ds1302_get_date();
                g_sys_val.today_date = g_sys_val.date_info;
                // 更新每日任务                
                create_todaytask_list(g_sys_val.time_info);
                // 判断时间播放任务
                task_check_and_play();
                // 判断方案日期
                for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
                    if((solution_list.solu_info[i].en==1)&&(solution_list.solu_info[i].state!=0xFF))
                        solution_data_chk(i);
                }
                fl_solution_write();
                //
                host_info.online_date_info = g_sys_val.date_info; 
                // 离线日期判断
                if(host_info.offline_day!=0){
                    host_info.offline_day--;
                }
                // 注册日期判断
                if(host_info.regiser_days!=0){
                    host_info.regiser_days--;
                    // 机器到期
                    if((host_info.regiser_days==0)&&(host_info.regiser_state==1)){
                        host_info.regiser_state = 0;
                    }
                }                
                fl_hostinfo_write();
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
            // 更新注册信息
            register_could_chk();
            //
            if(need_mklog==0){
                need_mklog=1;
                g_sys_val.log_waitmk_f = user_file_mklog();
            }
        }
    }
}

//===========================================================================

//---------------------------------------------------------------------------
//10hz 检测线程
//---------------------------------------------------------------------------
void timee10hz_process(){
    mes_send_overtime();        // 主动上传到客户端消息同步
    task_10hz_mutich_play();    // 任务连续播放处理
    task_dtinfo_overtime_recive_close(); //关闭详细信息 连接
    disp_task_delay();          // 任务刷新显示
    bat_filecontorl_resend_tim(); //音乐回复重发
    rttask_build_overtime10hz(); //即时任务建立状态超时
    backup_sendmes_10hz();      //备份信息更新
    divfound_over_timeinc();    //设备搜索计时
    rttask_playlist_updata();   //计时任务播放列表更新
    xtcp_bufftimeout_check_10hz();//发送buff发送超时
    list_sending_overtime();    //列表发送超时    
    rttask_infosend_process();
}

//--------------------------------------------------------------------------
// 1hz 检测线程
//--------------------------------------------------------------------------
void second_process(){
    //uint8_t tmp; 
	//unsigned *a;
	//tmp = a;
    timer_process();            //系统时钟
    if(g_sys_val.eth_link_state){
        conn_overtime_close();  // conn连接超时
        div_heart_overtime_close(); //心跳离线超时
        account_login_overtime();   //账号登入超时
        timer_rttask_run_process(); //即时任务即时
        #if COULD_TCP_EN
        could_heart_send_timer();   //云服务主动连接与心跳计时
        dns_twominute_chk();
        #endif
    }
    time5s_send_udpconnect();   //定时向云发送寻呼链路udp心跳包
    //ipconflict_for_15s();       //IP冲突检测
    broadcast_for_minute();     //每分钟发送广播包计时
    timer_taskmusic_check();    //定时任务播放计时
    task_check_and_play();      //定时任务定时播放
    user_disp_time();           // lcd 时钟显示
    timer_task_disp();          // 显示切换
    task_secinc_process();
}

