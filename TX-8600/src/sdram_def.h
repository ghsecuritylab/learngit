#ifndef _SDRAM_DEF_H_
#define _SDRAM_DEF_H_

/**
 * SDRAM ʹ�����:
 * 
 *       0      - 43*4KB    �洢GBKת���
 *       44*4KB - 200*4KB    ����
 *       200*4KB - 3MB       FLASH��д����
 *       �û����ݴ洢�Ǵ�FLASH_DAT_SECTOR_BASE*4KB��ʼ
 *                       END_FL_SECTOR*4KB����, С��3MB
 *       
 *       3MB   - 6MB      ���ݻ���
 *       6MB   - 7MB      �����ļ��б�
 *       7MB   - 7.5MB    MP3���뻺��
 *               
 *       7.5MB   -           �û�ʹ����ʱ����
 *
 * ע��:SDRAM��0MB-3MB����flash
 */


#define SDRAM_FLASH_SECTOR_MAX_NUM  ((3*1024)/4)     //3MB

#define SDRAM_GBK_UNICODE_TBL_START (0)              //0MB ��172KB

#define SDRAM_DATA_BACKUP_START     (3*1024*1024/4)  //3MB, ��3MB

#define SDRAM_FILE_LIST_START       (6*1024*1024/4)  //6MB, ��512KB

#define SDRAM_FILE_LIST_SECTOR_SIZE (10*1024/4)     //10Kһ��sector

#define SDRAM_MP3DECODER_START      (((6*1024+512)*1024)/4) //6.5MB, ��1MB


// �������ļ���ʱbuff
#define USER_FILE_BAT_TMPBUF_BASE   (((7*1024+512)*1024)/4) //7.5MB, ��512KB

// 20K ����������������ļ�
// 10K ����豸������Ϣ
// 60K ���xtcp tx rx fifo

// ������������buff
#define USER_MUSICNAME_TMP_BASE     (USER_FILE_BAT_TMPBUF_BASE) //20K
// �豸������ʱbuff
#define USER_DIV_SEARCH_BASE        (USER_MUSICNAME_TMP_BASE+(20*1024/4)) //10K
// TX����FIFO 
#define USER_XTCP_TXFIFO_BASE       (USER_DIV_SEARCH_BASE+(30*1024/4))  //30K
// RX����FIFO
#define USER_XTCP_RXFIFO_BASE       (USER_XTCP_TXFIFO_BASE+(30*1024/4)) //30K

#endif
