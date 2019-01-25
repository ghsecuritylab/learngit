#ifndef __USER_LCD_H_
#define __USER_LCD_H_

#include "stdint.h"
#include "xtcp.h"

void user_disp_time();

void user_disp_data();

void user_disp_ip(xtcp_ipconfig_t ipconfig);

// ��ʾ����
void user_disp_task(uint8_t state);
// ˢ������
void user_disptask_refresh();
// ��ʱˢ����
void timer_task_disp();
// ��ʾ�汾
void user_disp_version();

void user_dispunti_init();

// ��ʱ��ʾ
void disp_task_delay();

#endif

