#ifndef  __FL_BUFF_DECODE_H_
#define  __FL_BUFF_DECODE_H_

#include <stdint.h>
#include "flash_adrbase.h"
#include "file_list.h"

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

//--------------------------------------------------
//ϵͳ��Ϣ ��д
void sys_dat_read(uint8_t buff[],uint16_t num,uint16_t base_adr);

void sys_dat_write(uint8_t buff[],uint16_t num,uint16_t base_adr);

//------------------------------------------------------------------------
// ��ʼ���û�����
void fl_hostinfo_init();

// ��ȡflash��ʼ��״̬���ж�FLASH�Ƿ���Ҫ��ʼ��
uint8_t read_hostinfo_reset_state();
//----------------------------------------------------------
// ��д��������
void fl_hostinfo_write();

void fl_hostinfo_read();
//------------------------------------------------------
// �˻��б���Ϣ��д
void fl_account_read(account_all_info_t *account_all_info,uint8_t id);

void fl_account_write(account_all_info_t *account_all_info,uint8_t id);
//------------------------------------------------------
// ������Ϣ��д
void fl_area_read();
//
void fl_area_write();
//------------------------------------------------------
// �豸�б��д
void fl_divlist_read();
//
void fl_divlist_write();
//-------------------------------------------------------
// �����б���д
void fl_solution_write();

void fl_solution_read();
//-------------------------------------------------------
// ��ʱ�����д
void fl_timertask_read(task_allinfo_tmp_t     *task_allinfo_tmp,uint16_t id);
//
void fl_timertask_write(task_allinfo_tmp_t *task_allinfo_tmp, uint16_t id);
//-------------------------------------------------------
// ��ʱ�����д
void fl_rttask_read(rttask_dtinfo_t     *rttask_dtinfo,uint16_t id);
//
void fl_rttask_write(rttask_dtinfo_t *rttask_dtinfo,uint16_t id);

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__FL_BUFF_DECODE_H_

