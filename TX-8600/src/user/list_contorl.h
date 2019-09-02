#ifndef __LIST_CONTORL_H
#define __LIST_CONTORL_H

#include "xtcp.h"
#include "sys_config_dat.h"

#ifndef null
#define null -1
#endif

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

//===================================================================
// ��������
//===================================================================
extern conn_list_t *conn_list_head;
extern conn_list_t *conn_list_end;
extern conn_list_t *conn_list_tmp;

void conn_list_init();  //CONN�����ʼ��

uint8_t create_conn_node(xtcp_connection_t *conn);  //����һ��CONN�ڵ� return 0 ʧ�� return 1�ɹ�

uint8_t delete_conn_node(int id);   //ɾ��һ��CONN�ڵ� return 0 ʧ�� return 1�ɹ�

conn_list_t *get_conn_info_p(int id);

conn_list_t *get_conn_for_ip(xtcp_ipaddr_t ip);

//===================================================================
// �豸����
//===================================================================
div_node_t *get_div_info_p(uint8_t mac[]);

uint8_t create_div_node();

uint8_t delete_div_node(uint8_t mac[]);

void div_list_init();

// ��ʼ��ʱ ��id��������
uint8_t div_node_creat_forid(uint8_t id);

//===================================================================
// ��������
//��������ڵ�
uint8_t create_task_node();
// ��ʼ����ID���б�
uint8_t create_task_node_forid(uint16_t id);
// ɾ������ڵ�
uint8_t delete_task_node(uint16_t id);
// ��ȡĿ������ڵ�
timetask_t *get_task_info_p(uint16_t id);
//==================================================================
// ��ʱ����
//����һ����ʱ����ڵ� return 0 ʧ�� return 1�ɹ�
uint8_t create_rttask_node();
// ��id��������
uint8_t create_rttask_node_forid(uint16_t id);
//ɾ��һ������ڵ� 
uint8_t delete_rttask_node(uint16_t id);
//�����������еļ�ʱ����
uint8_t create_rttask_run_node(uint16_t id);
// ɾ���������еļ�ʱ����
uint8_t delete_rttask_run_node(uint16_t id);
// ������������������
uint8_t rttask_run_chk(uint16_t id);

//==================================================================
// �����б�
//==================================================================
void area_list_init();

//==================================================================
// �˻��б�
//==================================================================
void account_list_init();

void account_list_read();


#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__LIST_CONTORL_H

