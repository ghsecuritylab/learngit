/*
*  File Name:remotel_business_base.h
*  Created on: 2019��5��26��
*  Author: caiws
*  description :��̨��־��¼����
*  Modify date: 
* 	Modifier Author:
*  description :
*/
#ifndef __SYSLOG_H_
#define __SYSLOG_H_

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

/*---------------------------------------------------------------------------------------
*@description: debug��ӡ�������ӡ
*@Author: ljh
*@param[out]: ��
*@param[in]: �ı���Ϣ
*@return: ��
---------------------------------------------------------------------------------------*/
void xtcp_debug_printf(char fmt[], ...);

#define xtcp_debug_printf(...)  xtcp_debug_printf(__VA_ARGS__)
//=====================================================================
typedef struct log_info_t{
    char buff[512];
    unsigned len;
}log_info_t;

/*
*@description: ��־��Ϣת������
*@Author: ljh
*@param[out]: ��
*@param[in]: �ı���Ϣ
*@return: log_info_tָ��=��buff������Ϣ/len���ı�����
*/
#ifdef __XC__ 
log_info_t *unsafe log_info_chang(char * fmt, ...);
int itoa_forutf16(unsigned n, char *unsafe buf, unsigned base, int fill);
// �ַ�ת��
int log_itoa(unsigned n, char *unsafe buf, unsigned base, int fill);

void log_reverse_array(char buf[], unsigned size);
#else
log_info_t* log_info_chang(char * fmt, ...);
int itoa_forutf16(unsigned n, char *buf, unsigned base, int fill);
// �ַ�ת��
int log_itoa(unsigned n, char * buf, unsigned base, int fill);

void log_reverse_array(char buf[], unsigned size);
#endif

//=======================================================================

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__SYSLOG_H_

