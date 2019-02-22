#ifndef __ACK_BUILD_H_
#define __ACK_BUILD_H_

#include <stdint.h>
#include "list_instance.h"

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

void could_list_init();

uint16_t build_endpage_decode(uint16_t len,uint16_t cmd,uint8_t id[]);

// ����Ӧ����
uint16_t online_request_ack_build(uint8_t online_state,uint8_t mode);

// Ӧ����
uint16_t onebyte_ack_build(uint8_t mode,uint16_t cmd);

uint16_t twobyte_ack_build(uint8_t state1,uint8_t state2,uint16_t cmd);

uint16_t threebyte_ack_build(uint8_t state1,uint8_t state2,uint8_t state3,uint16_t cmd);

uint16_t id_ack_build(uint16_t id,uint8_t state,uint16_t cmd);

// ������Ӧ��
uint16_t heart_ack_build(uint8_t state);

// �豸��ϸ��Ϣ����
uint16_t extra_info_build_ack();

// �豸�б���
uint16_t div_list_resend_build(uint16_t cmd,div_node_t **div_list_p,uint8_t div_send_num);
// ip mac �б���
uint16_t div_ipmac_list_send();

// �����б����
uint16_t area_list_send_build(uint16_t cmd);
// �������ûظ�
uint16_t area_config_ack_build(uint16_t area_sn,uint8_t state,uint8_t contorl);
// �û���¼��Ϣ�ظ�
uint16_t account_login_ack_build(uint8_t log_state,uint8_t user_id,uint8_t *mac_buf,uint16_t cmd);

// �˻��б�鿴
uint16_t account_list_ack_build();

// �˻�Ȩ���б�鿴
uint16_t account_maclist_ack_build(account_all_info_t *account_all_info);

// �˻����ûظ�
uint16_t account_control_ack_build(uint8_t contorl,uint8_t id,uint8_t state);

// �����б�鿴
uint16_t solution_list_ack_build(uint16_t cmd);
// �����б����ûظ�
uint16_t solution_config_build(uint16_t id,uint8_t state,uint8_t config);

// �������ûظ�
uint16_t task_config_ack_build(uint16_t id,uint8_t state);

// �����б���
uint16_t task_list_ack_build();

// ������ϸ��Ϣ����
uint16_t task_dtinfo_chk_build();

// ���������ѯ
uint16_t todaytask_ack_build();

// ��ʱ�������ûظ�
uint16_t rttask_config_ack_build(uint16_t id,uint8_t state);

// ��ʱ�����б��ȡ
uint16_t rttask_list_chk_build();

// ��ʱ������ϸ��Ϣ��ѯ
uint16_t rttask_dtinfo_chk_build(uint16_t id);

// ��ʱ������Ϣ�������
uint16_t rttask_connect_build(uint8_t contorl,uint16_t ran_id,uint16_t id,uint16_t cmd,uint8_t no_task);

// ��ʱ������Ϣ������
uint16_t rttask_creat_build(uint16_t ran_id,uint8_t state,uint16_t task_id);

// ��Ͳ �û� ��ѯ�ظ�
uint16_t mic_userlist_ack_build(uint8_t state,account_all_info_t *account_all_info);

// �����ļ��б��ѯ
uint16_t music_patchlist_chk_build();

// �����б�����ѯ
uint16_t music_namelist_chk_build(uint8_t state);
// �ļ��������Ȳ�ѯ
uint16_t file_progress_build(uint8_t state,uint8_t progress,uint8_t id[],uint8_t *name,uint8_t *patch);

// ��������ظ�
uint16_t file_batinfo_build(uint8_t *patch,uint8_t *file,uint8_t contorl,uint8_t bat_state,uint8_t state);

//ע��ظ�
uint16_t host_resiged_ack_build(uint8_t state);

uint16_t sysonline_chk_build(uint8_t state);
// ��Ϣ֪ͨ
uint16_t listinfo_upgrade_build(uint8_t type);
// ����֪ͨ
uint16_t taskinfo_upgrade_build(task_allinfo_tmp_t *task_allinfo_tmp,uint8_t contorl,uint16_t task_id);
// ��ʱ����֪ͨ
uint16_t rttaskinfo_upgrade_build(uint16_t id);
// �˻�֪ͨ
uint16_t acinfo_upgrade_build(uint16_t id);
// ����֪ͨ
uint16_t sulo_upgrade_build(uint8_t id);

// ���ݿ��� Э��  B90A
uint16_t backup_contorl_build(uint8_t state,uint8_t *data);

// ������Ϣ���� Э��  B90B
uint16_t backup_updata_build(uint8_t state,uint8_t bar);
// ͬ������IP Э��  BF07
uint16_t sync_hostip_build(uint8_t mac[],uint8_t *ipaddr);

// �����豸�б� Э��  BF09
uint16_t divsrc_list_build();

// �Ʒ��������� Э��  BE00
uint16_t cld_heart_build();

// �Ʒ�����ע���ѯ Э��  BE01
uint16_t cld_resigerchk_build();

// ��ע������ Э��  BE08
uint16_t cld_resiger_request_build();

// ʱ��ͬ������ BE03
uint16_t cld_timesysnc_request_build();


#if defined(__cplusplus) || defined(__XC__)
}
#endif


#endif //__ACK_BUILD_H_

