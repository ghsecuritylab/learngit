#ifndef _MP3_TYPES_H_
#define _MP3_TYPES_H_

#if defined(__XC__)
extern "C" {
#endif

//MP3 Xing֡��Ϣ(û��ȫ���г���,���г����õĲ���)
typedef struct 
{
    unsigned char id[4];            //֡ID,ΪXing/Info
    unsigned char flags[4];     //��ű�־
    unsigned char frames[4];        //��֡��
    unsigned char fsize[4];     //�ļ��ܴ�С(������ID3)
}MP3_FrameXing;

//MP3 VBRI֡��Ϣ(û��ȫ���г���,���г����õĲ���)
typedef struct 
{
    unsigned char id[4];            //֡ID,ΪXing/Info
    unsigned char version[2];       //�汾��
    unsigned char delay[2];     //�ӳ�
    unsigned char quality[2];       //��Ƶ����,0~100,Խ������Խ��
    unsigned char fsize[4];     //�ļ��ܴ�С
    unsigned char frames[4];        //�ļ���֡�� 
}MP3_FrameVBRI;
//MP3_INFO
typedef struct 
{
    unsigned int totsec ;                   //���׸�ʱ��,��λ:��

    unsigned int bitrate;                   //������
    unsigned int samplerate;                //������
    unsigned short outsamples;              //PCM�����������С(��16λΪ��λ),������MP3,�����ʵ�����*2(����DAC���)

    unsigned char wav_bitwidth;
    unsigned char wav_chanal_num;
    unsigned char wav_format;
    unsigned wav_datlen;
    
    unsigned int datastart;                 //����֡��ʼ��λ��(���ļ������ƫ��)
}MP3_Info;

#if defined(__XC__)
}
#endif

#endif
