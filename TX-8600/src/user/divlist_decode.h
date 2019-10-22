#ifndef  __DECODE_FUNLIST_H
#define  __DECODE_FUNLIST_H

#include <stdint.h>

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

// ������������
void div_heart_recive(); //0xD000

// �������߰�����
void div_online_recive();   //0xB000

//-------------------------------------------------
// �豸�б��������
void divlist_request_recive(); //0xB100

void div_sending_decode(uint8_t list_num); //�б�ְ����ʹ���
//--------------------------------------------------

// ��ϸ��Ϣ��ȡ����
void div_extra_info_recive(); //0xB102

// �豸��Ϣ���ù���
void div_info_set_recive(); //B104
//----------------------------------------------------
//������Ϣ����
void arealist_request_rec();    // B200

void arealist_sending_decode(uint8_t list_num);
//
//��������
void area_config_recive();  //B202

//-----------------------------------------------------
// �豸���ӳ�ʱ���� 1HZ process
void div_heart_overtime_close();
//-----------------------------------------------------
// �豸IP MAC��ȡ
void div_ip_mac_check_recive();

//-----------------------------------------------------
// �����豸��Ϣ��ȡ BF08
void research_lan_revice();
//-----------------------------------------------------
// ��ȡ�����豸�б�
void sysset_divfound_recive();
//-----------------------------------------------------
// ������ѡ�����豸 Ŀ������IP
void divresearch_hostset_recive();
//-----------------------------------------------------
// ����ָ��mac��IP
void divlist_ipchk_recive();

void divfound_over_timeinc();
//
void divsrc_sending_decode(uint8_t list_num);

// �Ƶ�¼����ģʽ C004
void offlinediv_mode_recive();

// �ն˲���ָ�� BE0F
void div_textsend_recive();


#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif  //__DECODE_FUNLIST_H

