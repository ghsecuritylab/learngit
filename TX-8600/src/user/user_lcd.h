/*
*  File Name:remotel_business_base.h
*  Created on: 2019��5��26��
*  Author: caiws
*  description :lcd��ʾ����
*   Modify date: 
* 	Modifier Author:
*  description :
*/

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
//��ʾ��ʼ��
void user_dispunti_init();
// ��ʱ��ʾ
void disp_task_delay();
// ��ʾ������״̬
void disp_couldstate(uint8_t state);
// DHCPͼ�꿪����ʾ
void dhcp_disp_en();
// DHCPͼ��ر���ʾ
void dhcp_disp_dis();
void dhcp_disp_none();

void ip_disp_decode(uint8_t data,uint8_t *base_adr);

void ip_conflict_disp(uint8_t state);

void dhcp_getin_disp();

void dhcp_getin_over_disp(uint8_t state);

void dhcp_getin_clear();

void wifi_open_disp();

void reset_data_disp(uint8_t second);

#endif

