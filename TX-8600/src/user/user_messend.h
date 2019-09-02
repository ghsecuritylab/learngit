#ifndef __USER_MESSEND_H
#define __USER_MESSEND_H

#include <stdint.h>
#include "sys_config_dat.h"
#include "user_unti.h"
#include "ack_build.h"
#include "user_xccode.h"
#include "xtcp.h"
#include "debug_print.h"

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

// ���ӽ���
uint8_t mes_list_add(xtcp_connection_t conn,uint8_t could_f,uint8_t could_id[],uint8_t account_f);

// ���ӹر�
void mes_list_close(unsigned id);

// ֪ͨ����
void mes_send_decode();

// ���б���Ϣ����֪ͨ
void mes_send_listinfo(uint8_t type,uint8_t need_send);

// �˺Ÿ���֪ͨ
void mes_send_acinfo(uint16_t id);

// �������֪ͨ ����ҳ�����
void mes_send_taskinfo(task_allinfo_tmp_t* task_all_info);

// �������֪ͨ ����ҳ�����
void mes_send_taskinfo_nopage(task_allinfo_tmp_t* task_all_info);

// ��ʱ�������֪ͨ
void mes_send_rttaskinfo(uint16_t id,uint8_t contorl,uint8_t page_state);

// ��������֪ͨ
void mes_send_suloinfo(uint16_t id);

// ���ͳ�ʱ
void mes_send_overtime();

// ����ҳ�����֪ͨ
void taskview_page_messend();

#if defined(__cplusplus) || defined(__XC__)
}
#endif


#endif

