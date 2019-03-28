#ifndef __CONN_PROCESS_H
#define __CONN_PROCESS_H

#include "stdint.h"
#include "user_unti.h"

//--------------------------------------
// ָ����մ���
void conn_decoder();
//--------------------------------------
// ������ɼ�������������
void xtcp_sending_decoder();
//--------------------------------------
// ���ӳ�ʱ����
void conn_overtime_close();

void xtcp_buff_fifo_put(uint8_t tx_rx_f,uint8_t *buff,xtcp_fifo_t *kf);

void xtcp_buff_fifo_get(uint8_t tx_rx_f,uint8_t *buff,xtcp_fifo_t *kf,uint8_t clear_f);

uint8_t xtcp_check_fifobuff(xtcp_fifo_t *kf);

void xtcp_fifobuff_throw(xtcp_fifo_t *kf);

void xtcp_bufftimeout_check_10hz();

void xtcp_sendend_decode();

void user_xtcp_fifo_send();

void xtcp_tx_fifo_put();

void xtcp_rx_fifo_put();

void xtcp_tx_fifo_get();

void xtcp_rx_fifo_get();

#endif  //__CONN_PROCESS_H

