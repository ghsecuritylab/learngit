#ifndef __TASK_DECODE_H_
#define __TASK_DECODE_H_

#include <stdint.h>
#include "list_instance.h"

// ��ʱ�����б��ʼ��
void task_fl_init();

// ��ȡ��ʱ�����б������Ϣ
void timer_tasklist_read();
// ���������б�����
void create_alltask_list();
// ���������б�����
void create_todaytask_list(time_info_t time_info);

// ������ѯ
void solution_check_recive();
// ���������ж�
void solution_data_chk(uint8_t id);

// �����б��ѯ
void task_check_recive();
// �����б�����
void tasklist_sending_decode(uint8_t list_num);

// ������ϸ��Ϣ��ѯ
void task_dtinfo_check_recive();

// ���񲥷�
void task_music_config_play(uint8_t ch,uint16_t id);

// ��������
void solution_config_recive();

// ��������
void task_config_recive();

// ������ϸ��Ϣ����
void task_dtinfo_config_recive();

// ���������༭
void task_bat_config_recive();

// �����༭�ն�����                     B30F
void bat_task_divset_recive();

// ����������ֹ�༭
void task_en_recive();

// ������Ϣ����
void task_dtinfo_decode(uint8_t list_num);
// ��ʱ����ʧ��
void task_dtinfo_overtime_recive_close();

// ��ʱ���񲥷Ų���
void task_playtext_recive();

// ���������ѯ
void today_week_check_recive();

// ������������
void today_week_config_recive();

// ��ʱ�����ѯ
void rttask_list_check_recive();

// ��ʱ������ϸ��Ϣ��ѯ
void rttask_dtinfo_check_recive();

// ��ʱ��������
void rttask_config_recive();

// ��ʱ����APP����
void rttask_contorl_recive();
//
void rttask_contorl_connect_decode(uint8_t con_num);
//-------------------------------------------
// ��ʱ���� �豸���� 
void rttask_build_recive();

// ��ʱ���� �ط�
void task_rttask_rebuild();

// ��ʱ�����б� flash��ʼ��
void fl_rttask_dat_init();

// ��ʱ�����б� ��ȡ
void rt_task_list_read();

// ��ʱ�����б���������
void rttask_list_sending_decode(uint8_t list_num);

// ��ʱ����  ip�б����            BF0C
void rttask_playlist_updata_init(uint8_t ip[],div_node_t *div_info_p);

void rttask_playlist_updata();


//��flash ����ʱ 10hz Э�鴦���߳�
void task_pol_decode_process();

// 1hz��ʱ���������߳�
void timer_rttask_run_process();

// �����¼�����
void task_musicevent_change(uint8_t ch,char event,char data);

// 1S ��ʱ�����ʱ
void timer_taskmusic_check();

// ������
void task_check_and_play();
// �������񲥷�
void task_10hz_mutich_play();

// ����ֹͣ
void task_music_config_stop(uint8_t ch);
// ֹͣ���Բ�������
void task_music_stop_all();

// ��ʱ���񿪹�״̬��λ
void rttask_build_overtime10hz();

// �ر������м�ʱ����
void close_running_rttask(uint8_t *mac);

void task_pageshow_recive();

void solulist_chk_forapp_recive();

void tasklist_forsolu_chk_recive();

#endif  //__TASK_DECODE_H_

