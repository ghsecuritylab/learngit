#ifndef __ACCOUNT_H_
#define __ACCOUNT_H_

#include <stdint.h>

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

void filename_decoder(uint8_t *buff,uint8_t num);

//-----------------------------------------
// ��������
void user_host_search_recive();
//-----------------------------------------
// �˻���¼����
void account_login_recive();
//-----------------------------------------
// �˻��б��ѯ
void account_userlist_recive();

void ac_list_sending_decode();
//-----------------------------------------
// Ȩ���б���
void account_div_list_recive();
//-----------------------------------------
// �˻�����
void account_config_recive();
//-----------------------------------------
// ϵͳע��
void account_sys_register_recive();
//-----------------------------------------
// ϵͳ���߼��
void account_sysonline_recive();
//-----------------------------------------
// ʱ��ͬ��
void user_timer_sync_recive();

void cld_timer_sync_recive();
//-----------------------------------------
// ���ӳ�ʱ
void account_login_overtime();

// ��Ͳ��¼
void mic_userlist_chk_recive();

// ��ͲѰ������
void mic_aux_request_recive();

// ��Ͳͨ������
void mic_aux_heart_recive();

// ��ʱ�رջ�Ͳͨ��
void mic_time1hz_close();

//�ָ�������æ��ѯ
void backup_busy_chk();

// �ָ����ݿ�ʼ����
void backup_contorl_chk();

// �ָ�������Ϣ����
void backup_mes_send_recive();

// ��ʱIP����   BF09
void tmp_ipset_recive();

// ��������IP
void sysset_ipset_recive();

// ����������
void maschine_code_init();

// ϵͳע���ѯ
void register_could_chk();

// ע������   BE08
void cld_register_request();

// ��ע������ظ�
void cld_register_recive();

// �ֻ�ע������ B90D
void app_register_request();


// ʱ��ͬ������ BE03
void cld_timesysnc_request();

// �˺��Ƶ�¼
void cld_account_login_recive();

// �˺��б�����
void account_list_updat();

void backup_sendmes_10hz();

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__ACCOUNT_H_

