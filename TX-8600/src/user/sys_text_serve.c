#include "sys_text_serve.h"
#include "user_xccode.h"
#include "ack_build.h"
#include "user_unti.h"
#include "debug_print.h"

// �鿴�շ������� C001
void text_get_txpage_recive(){
	user_sending_len = chk_txpage_cnt_build();
	user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

// �����ӡ�����ر�   C002
void eth_debug_contorl_recive(){
	if(xtcp_rx_buf[POL_DAT_BASE]==0){
        debug_printf("debug_on\n");
		g_sys_val.eth_debug_f=1;
		debug_conn_connect(&xtcp_rx_buf[POL_DAT_BASE+1]);
	}
	else{
		//�ر������ӡ
		debug_printf("debug_off\n");
		g_sys_val.eth_debug_f=0;
		debug_conn_colse();
	}
}

