#ifndef __CONN_PROCESS_H
#define __CONN_PROCESS_H

#include "stdint.h"

//--------------------------------------
// ָ����մ���
void conn_decoder();
//--------------------------------------
// ������ɼ�������������
void xtcp_sending_decoder();
//--------------------------------------
// ���ӳ�ʱ����
void conn_overtime_close();
// �����ӽ���
uint8_t conn_long_decoder();

void connlong_list_init();

uint8_t user_longconnect_build(uint8_t *ipaddr);


#endif  //__CONN_PROCESS_H

