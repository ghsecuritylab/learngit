#ifndef __USER_UNTI_H_
#define __USER_UNTI_H_

#include <stdint.h>
#include <string.h>
#include "eth_audio.h"
#include "eth_audio_config.h"
#include "protocol_adrbase.h"
#include "list_instance.h"

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

uint8_t mac_cmp(uint8_t *a, uint8_t *b);

uint8_t ip_cmp(uint8_t *a, uint8_t *b);

uint8_t charncmp(uint8_t *c1,uint8_t *c2,unsigned len);

#define sn_cmp(a,b) (charncmp(a,b,SYS_PASSWORD_NUM))  

#define NEED_FL_HOSTINFO    0x01
#define NEED_FL_AREALIST    0x02
#define NEED_FL_DIVLIST     0x04
#define NEED_FL_SOLUTION    0x08
#define NEED_FL_RTTASK_LIST 0x10

typedef struct g_sys_val_t{
    //-------------------------------------------
    // flash ��д��־λ
    unsigned need_flash;
    // flash �豸�б���д����λ
    unsigned fl_divlist_inc;
    // ��flash ������������
    uint16_t fl_task_sn;
    //
    tmp_union_t tmp_union;
    // Э����������
    uint8_t task_busy;
    uint8_t task_pol_cmd;
    // ����·��
    uint16_t fsrc[(PATCH_NAME_NUM+MUSIC_NAME_NUM)/2];
    uint16_t fdes[(PATCH_NAME_NUM+MUSIC_NAME_NUM)/2];
    //-------------------------
    //�����б� ��ϸ��Ϣ����״̬
    uint16_t task_recid;
    uint16_t task_con_id;
    uint16_t task_music_inc;
    uint16_t task_packrec_inc;
    uint16_t task_rec_count;
    uint8_t  task_con_state;
    uint8_t  task_creat_s;
    uint8_t  task_config_s;
    uint8_t  task_delete_s;
    uint8_t  task_dtinfo_setmusic_f;
    uint8_t  task_dtinfo_setdiv_f;
    //------------------------
    //���������ļ����� ״̬
    xtcp_connection_t file_bat_conn;
    uint8_t file_bat_contorl_s;
    unsigned file_bat_tim;
    uint8_t file_batpack_inc;
    uint8_t file_bat_tolnum;
    uint8_t file_bat_musicinc;
    uint8_t file_bat_contorl;
    uint8_t file_bat_nametmp[MUSIC_NAME_NUM];
    uint8_t file_bat_srcpatch[PATCH_NAME_NUM];
    uint8_t file_bat_despatch[MUSIC_NAME_NUM];
    uint8_t file_bat_id[6];
    uint8_t file_bat_resendtim;
    uint8_t file_bat_resendf;
    uint8_t file_bat_resend_inc;
    uint8_t file_bat_overtime;

    uint8_t file_bat_resend_tmp[3];
    //---------------------------------------------------
    //���ȼ��б�
    enum AUDIO_TYPE_E audio_type[NUM_MEDIA_INPUTS];
    //-------------------------------------------
    // �û� flash
     uint8_t fl_account_id;
    //-------------------------------------------
    // ��̫������״̬
    uint8_t eth_link_state;
    //------------------------------------------
    // ʱ����Ϣ
    time_info_t time_info;
    date_info_t date_info;
    //
    //-------------------------------------------
    // ������������
    // 
    uint8_t task_wait_state;
    uint8_t play_ok;
    uint16_t music_task_id[MAX_MUSIC_CH];
    uint8_t play_error_inc[MAX_MUSIC_CH];
    //-------------------------------------
    date_info_t today_date;     //��������
    //----------------------------------------
    // ������ʾ
    uint8_t disp_furef[MAX_MUSIC_CH+2]; //�Ƿ񼴽���������
    uint8_t disp_ch[MAX_MUSIC_CH+2];
    uint8_t dispname_buff[MAX_MUSIC_CH+2][DIV_NAME_NUM*2];
    uint8_t disptime_buff[MAX_MUSIC_CH+2][DIV_NAME_NUM];
    uint8_t dispdura_buff[MAX_MUSIC_CH+2][DIV_NAME_NUM];
    uint8_t dispmusic_buff[MAX_MUSIC_CH+2][MUSIC_NAME_NUM];
    uint8_t disp_num;
    uint8_t disp_delay_inc;
    uint8_t disp_delay_f;
    // �ļ�����
    uint16_t file_ack_cmd;
    xtcp_connection_t file_conn_tmp;
    //-----------------------------------------------
    // ��Ϣ���²���
    uint8_t connect_ip[4];
    uint8_t connect_build_f;
    uint8_t connect_send_f;
    // 
    uint8_t messend_state;
    uint8_t messend_inc;
    uint16_t messend_len;
    uint8_t messend_over_time;
    uint8_t tx_buff[1472];
    //-------------------------------------------------
    uint16_t dtinfo_chk_task_id;
    // �ط���־
    uint8_t resend_inc;
    // ������־
    uint8_t key_state;
    uint8_t key_delay;
    uint8_t wifi_mode;
    uint8_t key_reselse;
    uint8_t key_wait_inc;
    // ������־
    uint8_t reboot_f;
    uint8_t reboot_inc;
    // ����Ԥ��ģʽ
    uint8_t gateway_standy;
    uint8_t gateway_time;
    // SD��״̬
    uint8_t sd_state;
    // sys timer
    unsigned sys_timinc;
    // ----------------------------------------------
    // ����ͨ��ʱ���
    unsigned tx_timestamp[MAX_MUSIC_CH];
    //-----------------------------------------------
    // ��Ͳռ��״̬
    uint8_t aux_ch_state[AUX_TYPE_NUM*AUX_RXCH_NUM];
    uint8_t aux_ch_tim[AUX_TYPE_NUM*AUX_RXCH_NUM];
    uint8_t aux_ch_ip[AUX_TYPE_NUM*AUX_RXCH_NUM][4];
    //-----------------------------------------------------
    // �㲥����
    xtcp_connection_t broadcast_conn;
    //
    uint8_t could_send_cnt;
    xtcp_connection_t could_conn; //������״̬
    xtcp_ipaddr_t could_ip;
    uint8_t colud_connect_f;
    int colud_port;
    //
    uint8_t tftp_busy_f;
    //�������ݻָ�״̬
    uint8_t backup_busy_f;
    uint8_t backup_bar;
    uint8_t backup_timechk;
    uint8_t backup_resend;
    uint8_t backup_resend_inc;
    xtcp_connection_t backup_conn;
    //
    //������
    uint8_t maschine_code[10];
    //
    uint8_t register_code[10];
    xtcp_connection_t regsiter_conn;
    // ϵͳ����
    // �����豸��
    unsigned search_div_tol;
    xtcp_connection_t divsearch_conn;
    uint8_t divsreach_f;
    uint8_t contorl_id[6];
    uint8_t divsreach_tim_inc;
    uint8_t divsreach_could_f;
    //
    uint8_t sn_key[DIV_NAME_NUM];
    //
    uint8_t con_id_tmp[6];
}g_sys_val_t;

extern g_sys_val_t g_sys_val;

extern tmp_union_t tmp_union;

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__USER_UNTI_H_

