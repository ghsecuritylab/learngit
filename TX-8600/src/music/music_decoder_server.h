#ifndef _MISIC_DECODER_SERVER_H_
#define _MISIC_DECODER_SERVER_H_


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
    
    unsigned get_pcmbuff_active(unsigned char          ch, unsigned int &samplerate, unsigned char pcmbuff[n], unsigned n, unsigned &length);
    
}music_decoder_output_if;

[[combinable]]
void music_decoder_server(server interface music_decoder_output_if if_mdo);


#endif
