#ifndef _TCP_IP_BaseAdr_H_
#define _TCP_IP_BaseAdr_H_

//=========================================================
//  TCP/IP Adr Def
//=========================================================
//--------------------MAC Header-----------------
#define UDP_DESMAC_ADR        0
//------------------- IP Header ---------------------
#define UDP_IPHEADLEN_ADR     16

#define UDP_IDENT_ADR         18

#define UDP_IPHEAD_SUM_ADR    24

#define UDP_SOURCE_IP_ADR     26
#define UDP_DES_IP_ADR        30  //4Byte-------34

#define IP_HEADER_LEN         20	  // IP Header Len Num

//------------------UDP Header -------------------
#define UDP_SOUPORT_ADR   	34
#define UDP_DESPORT_ADR   	36
#define UDP_HEADERLEN_ADR   38
#define UDP_HEADERSUM_ADR   40
//
//====================================================
// Header Base Address
#define IP_HEADBASE_ADR     14
#define UDP_HEADBASE_ADR    34
#define UDP_DATA_BASE_ADR   42
//====================================================
// UDP Data Base Adress
#define AUDIO_TYPE_ADR          (UDP_DATA_BASE_ADR)  //2Byte    //��Ƶͷ 0XAAAA
// ��Ƶʱ���
#define AUDIO_TIMESTAMP         (AUDIO_TYPE_ADR+2)  //4Byte
// ��Ƶ������־
#define AUDIO_AREA_F            (AUDIO_TIMESTAMP+4) //1Byte
// ������־
#define AUDIO_STATE_ADR         (AUDIO_AREA_F+1)    //2Byte
//
#define AUDIO_FORMAT_ADR        (AUDIO_STATE_ADR+2)  //1Byte     //��Ƶ����ʽ ������/λ��
//
#define AUDIO_CHTOTAL_ADR       (AUDIO_FORMAT_ADR+1)  //1Byte   //��Ƶͨ������
//
#define AUDIO_CHDATA_BASE_ADR   (AUDIO_CHTOTAL_ADR+1)  //1Byte  //��Ƶͨ��������ʼλ
//
#define AUDIO_AUXTYPE_ADR       0   //1Byte         //��Ƶ���� S,P,E
//
#define AUDIO_CHID_ADR          (AUDIO_AUXTYPE_ADR+1)   //1Byte    // ��Ƶͨ��id
//
#define AUDIO_CHPRIO_ADR        (AUDIO_CHID_ADR+1)   //1Byte       //��Ƶ���ȼ� ����
//
#define AUDIO_CHVOL_ADR         (AUDIO_CHPRIO_ADR+1)   //1Byte     //ͨ������
//
#define AUDIO_SILENT_ADR        (AUDIO_CHVOL_ADR+1)   //1Byte      //Ĭ���ȼ�
// 
#define AUDIO_DATALEN_ADR       (AUDIO_SILENT_ADR+1)   //2Byte     //��Ƶ�������ݳ���
//
#define AUDIO_DATABASE_ADR		(AUDIO_DATALEN_ADR+2) 	//Nbyte     //��������
//
//===========================================================================================
#endif
