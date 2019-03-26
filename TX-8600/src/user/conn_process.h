#ifndef __CONN_PROCESS_H
#define __CONN_PROCESS_H

#include "stdint.h"
#include "kfifo.h"

//--------------------------------------
// 指令接收处理
void conn_decoder();
//--------------------------------------
// 发送完成及数据连发处理
void xtcp_sending_decoder();
//--------------------------------------
// 连接超时处理
void conn_overtime_close();

void xtcp_buff_fifo_put(uint8_t tx_rx_f,uint8_t *buff,kfifo_t *kf);

void xtcp_buff_fifo_get(uint8_t tx_rx_f,uint8_t *buff,kfifo_t *kf,uint8_t clear_f);

uint8_t xtcp_check_fifobuff(kfifo_t *kf);

void xtcp_fifobuff_throw(kfifo_t *kf);

void xtcp_bufftimeout_check_10hz();


#endif  //__CONN_PROCESS_H

