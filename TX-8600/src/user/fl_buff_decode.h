#ifndef  __FL_BUFF_DECODE_H_
#define  __FL_BUFF_DECODE_H_

#include <stdint.h>
#include "flash_adrbase.h"
#include "file_list.h"

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

//--------------------------------------------------
// timer ��д���������б�
void timer_flash_dat_process();

//--------------------------------------------------
//ϵͳ��Ϣ ��д
void sys_dat_read(uint8_t buff[],uint16_t num,uint16_t base_adr);

void sys_dat_write(uint8_t buff[],uint16_t num,uint16_t base_adr);

void hostinfo_fl_write();

uint8_t timer_fl_hostinfo_decode();

//--------------------------------------------------
// ������Ϣ��д
void area_fl_read();

void area_fl_write();

uint8_t timer_fl_arealist_decode();

//------------------------------------------------------
// �豸�б��д
void divlist_fl_read();

void divlist_fl_write();

uint8_t timer_fl_divlist_decode();

//------------------------------------------------------
// �˻��б���Ϣ��д
void account_fl_read(account_all_info_t *account_all_info,uint8_t id);

void account_fl_write(account_all_info_t *account_all_info,uint8_t id);
//-------------------------------------------------------
// ��ʱ�����д
void timer_task_read(task_allinfo_tmp_t     *task_allinfo_tmp,uint16_t id);

void timer_task_write(task_allinfo_tmp_t *task_allinfo_tmp, uint16_t id);
//-------------------------------------------------------
// ��ʱ�����д
void rt_task_read(rttask_dtinfo_t     *rttask_dtinfo,uint16_t id);

void rt_task_write(rttask_dtinfo_t *rttask_dtinfo,uint16_t id);

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__FL_BUFF_DECODE_H_

