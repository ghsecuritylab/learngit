#ifndef _MISIC_DECODER_SERVER_H_
#define _MISIC_DECODER_SERVER_H_

#include <stdint.h>
/*
��Ƶ�����
pcm����ӿ�
1.һ��ȫ��ͨ������
2.�ִ�ͨ������
3.������
��һ����־λ
*/
typedef interface music_decoder_output_if
{
    void get_mp3_frame(uint8_t ch, uint8_t mp3_frame[], uint32_t &length, uint32_t &frame_num, uint32_t &samplerate);   
}music_decoder_output_if;

[[combinable]]
void music_decoder_server(server interface music_decoder_output_if if_mdo);


#endif
