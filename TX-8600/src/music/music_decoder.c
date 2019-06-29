#include <string.h>
#include <debug_print.h>
#include <timer.h>
#include "mp3dec.h"
#include "mp3common.h"
#include "coder.h"

#include "mymalloc.h"
#include "ff.h"	

#include "file.h"
#include "music_decoder.h"
#include "mp3_types.h"
#include "swlock.h"
#include "xassert.h"
#include "sdram.h"
#include "sdram_def.h"
#include "adpcm.h"

static music_decoder_mgr_t gt_mmdm;

static swlock_t g_mp3_lock[MP3DEC_CHANNAL_NUM];

static uint8_t g_file_buff[MUSIC_FILE_BUFF_SZ];

extern struct adpcm_state s_adpcm_state[];

extern unsigned char mf_typetell(TCHAR *fname);

static void read_sdram_file_buff(unsigned c_sdram, s_sdram_state *sdram_state, uint8_t ch, uint8_t flag, uint8_t buff[], uint32_t size)
{
    sdram_read(c_sdram, sdram_state, SDRAM_MP3DECODER_START+(2*ch+flag)*(MUSIC_FILE_BUFF_SZ/4), /*size/4*/size>>2, (unsigned*)&buff[0]);
    sdram_complete(c_sdram, sdram_state);  

}
static void write_sdram_file_buff(unsigned c_sdram, s_sdram_state *sdram_state, uint8_t ch, uint8_t flag, uint8_t buff[], uint32_t size)
{
    sdram_write(c_sdram, sdram_state, SDRAM_MP3DECODER_START+(2*ch+flag)*(MUSIC_FILE_BUFF_SZ/4), /*size/4*/size>>2, (unsigned*)&buff[0]);
    sdram_complete(c_sdram, sdram_state);
}

static unsigned int get_mp3_datastart(unsigned char *buff, unsigned int buff_size)
{
    if(buff_size >= 10)
    {
        if (memcmp(buff, "ID3",3) == 0)
        {
            return ((buff[6] & 0x7F)<< 21)|((buff[7] & 0x7F) << 14) | ((buff[8] & 0x7F) << 7) | (buff[9] & 0x7F);
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

static unsigned int get_wav_datastart(unsigned char *buff, unsigned int buff_size)
{
    if(buff_size >= 10)
    {
        if (memcmp(buff, "ID3",3) == 0)
        {
            return ((buff[6] & 0x7F)<< 21)|((buff[7] & 0x7F) << 14) | ((buff[8] & 0x7F) << 7) | (buff[9] & 0x7F);
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

static unsigned char wav_get_info(TCHAR *pname, MP3_Info *p_info){
    FIL*fmp3;
    unsigned char *buf;
    unsigned int buf_size;
    unsigned int br;
    unsigned char res;
    //
    //����5K�ڴ�
    buf = g_file_buff;
    buf_size = sizeof(g_file_buff);
    fmp3=mymalloc(sizeof(FIL));
    res = f_open(fmp3,(const TCHAR*)pname,FA_READ);//���ļ�
    
    if(res==0){
        res = f_read(fmp3, (char*)buf, 36, &br);
        // �ж��Ƿ�WAV
        if(buf[0]!=0x52 || buf[1]!=0x49 || buf[2]!=0x46 || buf[3]!=0x46)
            br = 0;
        // ��ȡλ��
        p_info->wav_bitwidth = buf[WAV_BITWIDTH];
        // ��ȡ������
        p_info->samplerate = buf[WAV_SAMPLERATE_ADR] | (buf[WAV_SAMPLERATE_ADR+1]<<8) | (buf[WAV_SAMPLERATE_ADR+2]<<16) | (buf[WAV_SAMPLERATE_ADR+3]<<24);
        // ��ȡͨ����
        p_info->wav_chanal_num = buf[WAV_CHANAL_ADR];
        // ��ȡ��Ƶ��ʽ
        p_info->wav_format = buf[WAV_FROMAT_ADR];
        // ��ȡ������
        p_info->bitrate = buf[WAV_KBYES_ADR] | (buf[WAV_KBYES_ADR+1]<<8) | (buf[WAV_KBYES_ADR+2]<<16) | (buf[WAV_KBYES_ADR+3]<<24); 
        //-------------------------------------------------------------------------------------------
        // ��wav�ļ�����ͷ
        uint8_t offset;
        unsigned error=0;
        while(br){
            f_read(fmp3, (char*)buf, 4, &br); //��data���ݶ�
            // �ҵ�dataͷ
            if(buf[0]==0x64 && buf[1]==0x61 && buf[2]==0x74 && buf[3]==0x61)
                break;
            // �Ҳ���dataͷ
            for(offset=1;offset<4;offset++){
                if(buf[offset]==0x64){
                    break;
                }
            }
            res = f_lseek(fmp3, fmp3->fptr-(4-offset));
            // ǰ2K�����Ҳ���dataͷ�������ļ� ���ⳤʱ���ȡ�ļ�
            error++;
            if(error>2048){
                br=0;
                break;
            }
        }
        if(br==0 && p_info->wav_format!=1){ // û��dataͷ ��ʽ����PCM
            res=0XFF;
        }
        //----------------------------------------------------------------------------------------------------
        // ��ȡ���ݳ���
        else{
            f_read(fmp3, buf, 4, &br); //��data���ݶ�
            p_info->wav_datlen = (buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24));
            if(br==0 || p_info->wav_datlen==0){ 
                res=0XFF;
            }
        }
        //----------------------------------------------------------------------------------------------------
        // �����ļ�ʱ��
        unsigned tmp;
        // ���㵥ͨ���ֽڳ��� ���ֽ�/ͨ����
        tmp = p_info->wav_datlen/p_info->wav_chanal_num;
        // ������ٸ�������       �ֽ���/λ��
        tmp = tmp/(p_info->wav_bitwidth/8);
        // ����ʱ�� ����������/������
        p_info->totsec = tmp/p_info->samplerate;
    }
    else res=0XFF;
    //
    myfree(fmp3);
    return res;
}

//��ȡMP3������Ϣ
//used ram:28+2032+56 ~36+2048+64 = 2148
//pname:MP3�ļ�·��
//pctrl:MP3������Ϣ�ṹ��
//����ֵ:0,�ɹ�
//    ����,ʧ��
static unsigned char mp3_get_info(TCHAR *pname, MP3_Info *p_info)
{
    MP3FrameInfo frame_info;
    MP3_FrameXing* fxing;
    MP3_FrameVBRI* fvbri;
    FIL*fmp3;
    unsigned char *buf;
    unsigned int buf_size;
    unsigned int br;
    unsigned char res;
    int offset=0;
    unsigned int p;
    short samples_per_frame;    //һ֡�Ĳ�������
    unsigned int totframes;             //��֡��

    fmp3=mymalloc(sizeof(FIL));

    //����5K�ڴ�
    buf = g_file_buff;
    buf_size = sizeof(g_file_buff);

    if(fmp3&&buf)//�ڴ�����ɹ�
    {
        f_open(fmp3,(const TCHAR*)pname,FA_READ);//���ļ�
        res=f_read(fmp3,(char*)buf,buf_size,&br);
        if(res==0)//��ȡ�ļ��ɹ�,��ʼ����ID3V2/ID3V1�Լ���ȡMP3��Ϣ
        {
            p_info->datastart = get_mp3_datastart(buf, buf_size);

            /************************************************************************/
            /*MP3���������ڴ�*/
            HMP3Decoder decoder = mymalloc(sizeof(MP3DecInfo));//2032
            memset(decoder, 0, sizeof(MP3DecInfo));

            FrameHeader *fh = mymalloc(sizeof(FrameHeader));//56
            memset(fh, 0, sizeof(FrameHeader));
            ((MP3DecInfo*)decoder)->FrameHeaderPS =(void *)fh;

            /************************************************************************/

            f_lseek(fmp3, p_info->datastart);   //ƫ�Ƶ����ݿ�ʼ�ĵط�
            f_read(fmp3, (char*)buf,buf_size,&br);    //��ȡ5K�ֽ�mp3����
            while(br > MAINBUF_SIZE)
            {
                offset=MP3FindSyncWord(buf,br); //����֡ͬ����Ϣ
                
                if(offset>=0&&MP3GetNextFrameInfo(decoder,&frame_info,&buf[offset])==0)//�ҵ�֡ͬ����Ϣ��,����һ����Ϣ��ȡ����
                {
                    p=offset+4+32;
                    fvbri=(MP3_FrameVBRI*)(buf+p);
                    if(strncmp("VBRI",(char*)fvbri->id,4)==0)//����VBRI֡(VBR��ʽ)
                    {
                        if (frame_info.version==MPEG1)samples_per_frame=1152;//MPEG1,layer3ÿ֡����������1152
                        else samples_per_frame=576;//MPEG2/MPEG2.5,layer3ÿ֡����������576
                        totframes=((unsigned int)fvbri->frames[0]<<24)|((unsigned int)fvbri->frames[1]<<16)|((u16)fvbri->frames[2]<<8)|fvbri->frames[3];//�õ���֡��
                        p_info->totsec=totframes*samples_per_frame/frame_info.samprate;//�õ��ļ��ܳ���
                    }
                    else    //����VBRI֡,�����ǲ���Xing֡(VBR��ʽ)
                    {
                        if (frame_info.version==MPEG1)  //MPEG1
                        {
                            p=frame_info.nChans==2?32:17;
                            samples_per_frame = 1152;   //MPEG1,layer3ÿ֡����������1152
                        }
                        else
                        {
                            p=frame_info.nChans==2?17:9;
                            samples_per_frame=576;      //MPEG2/MPEG2.5,layer3ÿ֡����������576
                        }
                        p+=offset+4;
                        fxing=(MP3_FrameXing*)(buf+p);
                        if(strncmp("Xing",(char*)fxing->id,4)==0||strncmp("Info",(char*)fxing->id,4)==0)//��Xng֡
                        {
                            if(fxing->flags[3]&0X01)//������frame�ֶ�
                            {
                                totframes=((unsigned int)fxing->frames[0]<<24)|((unsigned int)fxing->frames[1]<<16)|((u16)fxing->frames[2]<<8)|fxing->frames[3];//�õ���֡��
                                p_info->totsec=totframes*samples_per_frame/frame_info.samprate;//�õ��ļ��ܳ���
                            }
                            else    //��������frames�ֶ�
                            {
                                p_info->totsec=fmp3->fsize/(frame_info.bitrate/8);
                            }
                        }
                        else        //CBR��ʽ,ֱ�Ӽ����ܲ���ʱ��
                        {
                            p_info->totsec=fmp3->fsize/(frame_info.bitrate/8);
                        }
                    }
                    p_info->bitrate=frame_info.bitrate;         //�õ���ǰ֡������
                    p_info->samplerate=frame_info.samprate;     //�õ�������.
                    if(frame_info.nChans==2)p_info->outsamples=frame_info.outputSamps; //���PCM��������С
                    else p_info->outsamples=frame_info.outputSamps*2; //���PCM��������С,���ڵ�����MP3,ֱ��*2,����Ϊ˫�������
                    res = 0;
                    break;
                }
                else 
                {
                    res=0XFE; //δ�ҵ�ͬ��֡
                    offset++;
                    br -= offset;
                    memmove(buf, buf+offset, br);
                }
                /************************************************************************/
            }                
            /*MP3�ڴ��ͷ�*/
            myfree(fh);
            myfree(decoder);
            /************************************************************************/

        }
        f_close(fmp3);
    }
    else res=0XFF;

    myfree(fmp3);
    return res;
}

//��ȡmp3�ļ��Ĳ���ʱ������ʱԼΪ6ms
//pname:�ļ���
//����ֵ:0 - �ļ���Ч��ʧ��
//       ����ֵΪmp3�ļ���ʱ��
unsigned int get_mp3_totsec(TCHAR *pname)
{
    MP3_Info mp3_info;
    int res = mp3_get_info(pname, &mp3_info);
    //debug_printf("mp3_get_info %d\n", res);
    if(res != 0)
    {
        return 0;
    }

    if(mp3_info.samplerate!=48000 && mp3_info.samplerate!=44100 && mp3_info.samplerate!=16000) //&& mp3_info.samplerate!=24000 && mp3_info.samplerate!=22050 && mp3_info.samplerate!=11025 && mp3_info.samplerate!=32000 && mp3_info.samplerate!=8000)
        return 0;
    else
        return mp3_info.totsec;
}

//pname:�ļ���
//����ֵ:0 - �ļ���Ч��ʧ��
//       ����ֵΪwav�ļ���ʱ��
unsigned int get_wav_totsec(TCHAR *pname)
{
    MP3_Info mp3_info;
    int res = wav_get_info(pname, &mp3_info);
    //debug_printf("mp3_get_info %d\n", res);
    if(res != 0)
    {
        return 0;
    }

    if(mp3_info.samplerate!=48000 && mp3_info.samplerate!=44100 && mp3_info.samplerate!=16000) //&& mp3_info.samplerate!=24000 && mp3_info.samplerate!=22050 && mp3_info.samplerate!=11025 && mp3_info.samplerate!=32000 && mp3_info.samplerate!=8000)
        return 0;
    else
        return mp3_info.totsec;
}


/*********************************************************************************************************************************/
#define _USE_IN_FILE_SYSTEM_TASK_

void music_decoder_mgr_init()
{
    memset(&gt_mmdm, 0, sizeof(gt_mmdm));   
    gt_mmdm.ch_num = MUSIC_CHANNEL_NUM;

#if 1
    for(uint8_t ch=0; ch<MP3DEC_CHANNAL_NUM; ch++)
        swlock_init(&g_mp3_lock[ch]);    
#endif
}
int music_decode_start(unsigned char ch, unsigned char f_name[], unsigned int f_offset)
{
    int res = 0;
    int mp3_tag_offset = 0;
    unsigned char tag[36];
    music_decoderdev_t * p_dev = NULL;

    UINT br;
    
    if(ch >= gt_mmdm.ch_num)
    {
        res = 1;
        return res;
    }
    
    p_dev = &gt_mmdm.ch_dev[ch];
    if(p_dev->file_close_flag) 
    {
        f_close(&p_dev->file);
        memset(&p_dev->file, 0, sizeof(FIL));
        p_dev->file_close_flag = 0;
    }
    
    //�������
    memset(p_dev, 0, sizeof(music_decoderdev_t));

    res = f_open(&p_dev->file, (const TCHAR*)f_name, FA_READ);
    if(res != FR_OK)
    {
        return res;
    }
    // MP3 �ļ�ͷ�ж� WAV�ļ�ͷ�ж�
    unsigned char music_type = mf_typetell((const TCHAR*)f_name);
    // MP3 �ļ�
    if(music_type==1){
        res = f_read(&p_dev->file, tag, 10, &br);
        mp3_tag_offset = get_mp3_datastart(tag, 10);
        if(mp3_tag_offset)
        {
            f_offset += (mp3_tag_offset+10);
        }

        res = f_lseek(&p_dev->file, f_offset);
        if(res != FR_OK)
        {
            return res;
        }
        debug_printf("MP3 music_decode_start [%d] succeed\n", ch);
        p_dev->decoder_status = MUSIC_DECODER_START;
        // MP3ģʽ
        p_dev->file_type = 0;
    }
    // wav�ļ�
    else if(music_type==2){
        // ��wav�ļ�ͷ ��ȡwavͷ�ļ���Ϣ
        res = f_read(&p_dev->file, tag, 36, &br);
        // �ж��Ƿ�WAV
        if(tag[0]!=0x52 || tag[1]!=0x49 || tag[2]!=0x46 || tag[3]!=0x46)
            return FR_NO_FILE;
        // ��ȡλ��
        p_dev->wav_bitwith = tag[WAV_BITWIDTH];
        // ��ȡ������
        p_dev->wav_samplerate = tag[WAV_SAMPLERATE_ADR] | (tag[WAV_SAMPLERATE_ADR+1]<<8) | (tag[WAV_SAMPLERATE_ADR+2]<<16) | (tag[WAV_SAMPLERATE_ADR+3]<<24);
        // ��ȡͨ����
        p_dev->wav_nchanal = tag[WAV_CHANAL_ADR];
        // ��ȡ��Ƶ��ʽ
        p_dev->wav_format = tag[WAV_FROMAT_ADR];
        //-------------------------------------------------------------------------------------------
        // ��wav�ļ�����ͷ
        uint8_t offset;
        unsigned error=0;
        while(br){
            f_read(&p_dev->file, tag, 4, &br); //��data���ݶ�
            // �ҵ�dataͷ
            if(tag[0]==0x64 && tag[1]==0x61 && tag[2]==0x74 && tag[3]==0x61)
                break;
            // �Ҳ���dataͷ
            for(offset=1;offset<4;offset++){
                if(tag[offset]==0x64){
                    break;
                }
            }
            res = f_lseek(&p_dev->file, p_dev->file.fptr-(4-offset));
            // ǰ2K�����Ҳ���dataͷ�������ļ� ���ⳤʱ���ȡ�ļ�
            error++;
            if(error>2048){
                br=0;
                p_dev->decoder_status = MUSIC_DECODER_ERROR1;
                return FR_NO_FILE;
            }
        }
        if(br==0){ // û��dataͷ
            p_dev->decoder_status = MUSIC_DECODER_ERROR1;
            return FR_NO_FILE;
        }
        //----------------------------------------------------------------------------------------------------
        // ��ȡ���ݳ���
        f_read(&p_dev->file, tag, 4, &br); //��data���ݶ�
        p_dev->wav_datlen = (tag[0]|(tag[1]<<8)|(tag[2]<<16)|(tag[3]<<24));
        if(br==0 || p_dev->wav_datlen==0){ 
            p_dev->decoder_status = MUSIC_DECODER_FILE_END;
            return FR_NO_FILE;
        }
        //----------------------------------------------------------------------------------------------------
        s_adpcm_state[ch].index=0;
        s_adpcm_state[ch].valprev=0;
        debug_printf("\n\nWAV music_decode_start [%d] succeed\n", ch);
        p_dev->decoder_status = MUSIC_DECODER_START;
        // WAVģʽ
        p_dev->file_type = 1;
    }
    else{;}
    return FR_OK;
}

//f_close ֻ���ڴ����ݲ�����û��Ӳ������
int music_decode_stop(unsigned char ch, unsigned char change_status)
{
    int res = 0;
    if(ch >= gt_mmdm.ch_num)
    {
        res = -1;
        return res;
    }
    if(gt_mmdm.ch_dev[ch].decoder_status != MUSIC_DECODER_STOP) 
    {
        gt_mmdm.ch_dev[ch].file_close_flag = 1;
        
        if(change_status)
            gt_mmdm.ch_dev[ch].decoder_status = MUSIC_DECODER_STOP;
    }
    return 0;
}

int music_decode_play(unsigned char ch)
{
    return 0;
}

int music_decode_pause(unsigned char ch)
{
    return 0;
}

int update_music_decoder_status(music_decoder_status_t s[])
{
    music_decoderdev_t * p_dev = NULL;
    uint8_t need2notify = 0;
    for(uint8_t ch=0; ch<gt_mmdm.ch_num; ch++)
    {
        p_dev = &gt_mmdm.ch_dev[ch];
        // MUSIC_DECODER_START/MUSIC_DECODER_STOP ����֪ͨ����
        if(p_dev->decoder_status != s[ch].status &&
           p_dev->decoder_status != MUSIC_DECODER_START &&
           p_dev->decoder_status != MUSIC_DECODER_STOP)
        {
            music_decode_stop(ch, 0);
            s[ch].is_new = 1;
            need2notify = 1;

            s[ch].status = p_dev->decoder_status;
            p_dev->decoder_status= MUSIC_DECODER_STOP;//ADD:20181023
        }
        else
        {
            s[ch].status = p_dev->decoder_status;
        }
    }
    
    return need2notify;
}

void music_file_handle(STREAMING_CHANEND(c_sdram), REFERENCE_PARAM(s_sdram_state, p_sdram_state))
{
    UINT br = 0;
    uint8_t ch = 0;
    uint8_t res = 0;
    
    music_decoderdev_t * p_dev = NULL;
    
    for(ch=0; ch<gt_mmdm.ch_num; ch++)
    {
        p_dev = &gt_mmdm.ch_dev[ch];
        
        if(p_dev->decoder_status == MUSIC_DECODER_START && 
           p_dev->file_over_flag==0 &&
           (p_dev->file_buff_size[0]==0||p_dev->file_buff_size[1]==0))
        {
            res = f_read(&p_dev->file, g_file_buff, MUSIC_FILE_BUFF_SZ, &br);
            if(br == 0)
            {
                p_dev->file_over_flag = 1;
                
                debug_printf("f_read over %d %d\n", ch, res);
                
                if(res == FR_NOT_READY) return;
            }
            else
            {
                //debug_printf("f_read[%d] %d\n", ch, br);
                // ��ͬ����
                swlock_acquire(&g_mp3_lock[ch]);
                if(p_dev->file_buff_size[0]==0)
                {
                    //дSDRAM
                    write_sdram_file_buff(c_sdram, p_sdram_state, ch, 0, g_file_buff, MUSIC_FILE_BUFF_SZ);
                    
                    p_dev->file_buff_size[0] = br;
                    p_dev->file_buff_offset[0] = 0;
                    
                    if(p_dev->file_buff_size[1]==0)
                        p_dev->file_buff_for_used = 0;
                    else
                        p_dev->file_buff_for_used = 1;
                }
                else if(p_dev->file_buff_size[1]==0)
                {
                    //дSDRAM
                    write_sdram_file_buff(c_sdram, p_sdram_state, ch, 1, g_file_buff, MUSIC_FILE_BUFF_SZ);
                    
                    p_dev->file_buff_size[1] = br;
                    p_dev->file_buff_offset[1] = 0;
                
                    if(p_dev->file_buff_size[0]==0)
                        p_dev->file_buff_for_used = 1;
                    else
                        p_dev->file_buff_for_used = 0;                
                }
                // ��ͬ����
                swlock_release(&g_mp3_lock[ch]);
            }
        }

        
        if(p_dev->file_over_flag == 1 &&
           p_dev->file_buff_size[0] == 0 &&
           p_dev->file_buff_size[1] == 0)
        {
            p_dev->file_over_flag = 0;
            p_dev->decoder_status = MUSIC_DECODER_FILE_END;
            debug_printf("music_file_handle MUSIC_DECODER_FILE_END [%d] %d\n", ch, p_dev->mp3_frame_num);
        }
            
        if(p_dev->file_close_flag) 
        {
            f_close(&p_dev->file);
            memset(&p_dev->file, 0, sizeof(FIL));
            p_dev->file_close_flag = 0;
        }        
        
    }
    
}

/*********************************************************************************************************************************/
#define _USE_IN_MUSIC_DECOEDR_SERVER_TASK_
uint32_t get_mp3_frame(unsigned char ch, uint32_t *length, uint32_t *frame_num, uint32_t *samplerate,uint32_t *music_type)
{
    if(ch >= gt_mmdm.ch_num || gt_mmdm.ch_dev[ch].mp3_frame_full==0) 
    {  
#if 1
        if(gt_mmdm.ch_dev[ch].decoder_status == MUSIC_DECODER_START)
            debug_printf("fun get_mp3_frame empty [%d]\n", ch);
#endif
        return 0;
    }
    //debug_printf("get_mp3_frame[%d] length:%d\n", ch, *length);

    *length     = gt_mmdm.ch_dev[ch].mp3_frame_size;
    *frame_num  = gt_mmdm.ch_dev[ch].mp3_frame_num;    
    *music_type = gt_mmdm.ch_dev[ch].file_type;
    if(gt_mmdm.ch_dev[ch].file_type)
        *samplerate = gt_mmdm.ch_dev[ch].wav_samplerate;
    else
        *samplerate = gt_mmdm.ch_dev[ch].samplerate;    
    
    return (uint32_t)gt_mmdm.ch_dev[ch].mp3_frame;
}

void clear_mp3_frame(unsigned char ch)
{
    if(ch >= gt_mmdm.ch_num || gt_mmdm.ch_dev[ch].mp3_frame_full==0) 
    {
        return;
    }
    gt_mmdm.ch_dev[ch].mp3_frame_full = 0;
}

/*********************************************************************************************************************************/
#define _USE_IN_MUSIC_DECODER_HANDLE_TASK_

// ����mp3_frame_size
static int get_mp3_frame_size(MP3DecInfo *mp3DecInfo)
{     
    /*
     * mpeg1.0 
     *   layer1      : ֡��= (48000*bitrate kbps)/sampling_freq + padding
     *   layer2&3    : ֡��= (144000*bitrate kbps)/sampling_freq + padding
     * mpeg2.0 
     *   layer1      : ֡��= (24000*bitrate kbps)/sampling_freq + padding
     *   layer2&3    : ֡��= (72000*bitrate kbps)/sampling_freq + padding
     */
    FrameHeader *fh = mp3DecInfo->FrameHeaderPS;
    if(fh->ver==MPEG1)
    {
        if(fh->layer == 1)
            return (48*mp3DecInfo->bitrate)/mp3DecInfo->samprate + fh->paddingBit;
        else    
            return (144*mp3DecInfo->bitrate)/mp3DecInfo->samprate + fh->paddingBit;
    }
    else
    {
        if(fh->layer == 1)
            return (24*mp3DecInfo->bitrate)/mp3DecInfo->samprate + fh->paddingBit;
        else
            return (72*mp3DecInfo->bitrate)/mp3DecInfo->samprate + fh->paddingBit;
    }
}

static void set_file_buff_offset(music_decoderdev_t * p_dev, int offset)
{
        
    if((p_dev->file_buff_offset[p_dev->file_buff_for_used]+offset) < MUSIC_FILE_BUFF_SZ)
    {
        p_dev->file_buff_offset[p_dev->file_buff_for_used] += offset;
    }
    else
    {        
        offset = offset - (p_dev->file_buff_size[p_dev->file_buff_for_used] - p_dev->file_buff_offset[p_dev->file_buff_for_used]);
        
        p_dev->file_buff_size[p_dev->file_buff_for_used] = 0;
        p_dev->file_buff_offset[p_dev->file_buff_for_used] = 0;
        
        if(p_dev->file_buff_size[!p_dev->file_buff_for_used])
        {
            p_dev->file_buff_offset[!p_dev->file_buff_for_used] = offset;
            p_dev->file_buff_for_used = !p_dev->file_buff_for_used;
        }
    }
}
void music_decoder(STREAMING_CHANEND(c_sdram))
{
    music_decoderdev_t * p_dev = NULL;
    unsigned i=0,j=0;
    uint8_t ch = 0;
    uint8_t file_buff[MUSIC_FILE_BUFF_SZ*2];
    uint32_t file_buff_left = 0;
    uint8_t  *readptr;
    int offset = 0;

    MP3DecInfo mp3decinfo;
    FrameHeader fh;
   
    memset(&mp3decinfo, 0, sizeof(MP3DecInfo));
    memset(&fh, 0, sizeof(FrameHeader));
    mp3decinfo.FrameHeaderPS =(void *)&fh;

    s_sdram_state sdram_state;
    sdram_init_state(c_sdram, &sdram_state);

    while(1)
    {
        //TUDO:Ӧ��ÿ��ͨ����Ҫ��ʱ���ĸ�ͨ���ļ�ʱ���٣����Ƚ�����ͨ��
        for(ch=0; ch<MUSIC_CHANNEL_NUM; ch++)
        {
            p_dev = &gt_mmdm.ch_dev[ch];
            //------------------------------------------------------------------------------------------------------------
            // WAV ����ģʽ
            if(p_dev->file_type){
                //-----------------------------------------------------------------------------------
                if(p_dev->decoder_status != MUSIC_DECODER_START) continue;
                
                //mp3 frame��Ҫ���ʱ�ŵ���
                if(p_dev->mp3_frame_full) continue;
                
                if(p_dev->file_buff_size[0]==0 && p_dev->file_buff_size[1]==0) continue;
                //-----------------------------------------------------------------------------------
                // ����ͬ���������, ��ʱ0.1ms
                // ��ͬ����
                swlock_acquire(&g_mp3_lock[ch]);
                //                
                //-----------------------------------------------------------------------------------
                //��SDRAN MUSIC_FILE_BUFF_SZ*2
                read_sdram_file_buff(c_sdram, &sdram_state, ch, p_dev->file_buff_for_used, file_buff, MUSIC_FILE_BUFF_SZ);
                file_buff_left = p_dev->file_buff_size[p_dev->file_buff_for_used]-p_dev->file_buff_offset[p_dev->file_buff_for_used];
                
                if(p_dev->file_buff_for_used==1 && p_dev->file_buff_size[0])
                {
                    read_sdram_file_buff(c_sdram, &sdram_state, ch, 0, file_buff+MUSIC_FILE_BUFF_SZ, MUSIC_FILE_BUFF_SZ);
                    file_buff_left += (p_dev->file_buff_size[0]-p_dev->file_buff_offset[0]);
                }
                else if(p_dev->file_buff_for_used==0 && p_dev->file_buff_size[1])
                {
                    read_sdram_file_buff(c_sdram, &sdram_state, ch, 1, file_buff+MUSIC_FILE_BUFF_SZ, MUSIC_FILE_BUFF_SZ);
                    file_buff_left += (p_dev->file_buff_size[1]-p_dev->file_buff_offset[1]);
                }
                readptr = file_buff + p_dev->file_buff_offset[p_dev->file_buff_for_used];
                //-----------------------------------------------------------------------------------
                // ����λ��
                uint8_t byetwidth = p_dev->wav_bitwith/8;
                // ����ÿ���ֽ���
                unsigned sample_offset = p_dev->wav_nchanal*byetwidth;
                // ����ÿ���ֽ���     
                unsigned pack_allsample_len = sample_offset*WAV_PACK_SAMPLENUM;
                // �������ȡ���ֽ���
                unsigned dat_len = (file_buff_left<pack_allsample_len)?file_buff_left:pack_allsample_len;      
                // ȡ����������
                i=0;
                j=0;
                while(i<dat_len){
                    //------------------------------------------------
                    // ͨ��  Ĭ��ȡ������ �̶�ֻȡ16bit����
                    if(byetwidth==1){
                        p_dev->mp3_frame[j] = 0;
                        p_dev->mp3_frame[j+1] = readptr[i];
                    }
                    else if(byetwidth==2){
                        p_dev->mp3_frame[j] = readptr[i];
                        p_dev->mp3_frame[j+1] = readptr[i+1];
                    }
                    else if(byetwidth==3){
                        p_dev->mp3_frame[j] = readptr[i+1];
                        p_dev->mp3_frame[j+1] = readptr[i+2];
                    }
                    else if(byetwidth==4){
                        p_dev->mp3_frame[j] = readptr[i+2];
                        p_dev->mp3_frame[j+1] = readptr[i+3];
                    }
                    else{;}
                    //------------------------------------------------
                    // �������1 ͨ����*λ��
                    i +=sample_offset;
                    j +=2;
                }
                // ��ȡʵ�ʷ��ͳ���
                p_dev->mp3_frame_size = j;
                //                
                offset = dat_len;
                set_file_buff_offset(p_dev, offset);
                //
                if(file_buff_left<pack_allsample_len && p_dev->file_over_flag)
                {
                    p_dev->file_over_flag = 0;
                    p_dev->decoder_status = MUSIC_DECODER_FILE_END;
                }
                // ȡ֡���
                p_dev->mp3_frame_num++;
                p_dev->mp3_frame_full = 1;
                // ��ͬ����
                swlock_release(&g_mp3_lock[ch]);
            }
            //------------------------------------------------------------------------------------------------------------
            // MP3 ����ģʽ
            else{
                if(p_dev->decoder_status != MUSIC_DECODER_START) continue;

                //mp3 frame��Ҫ���ʱ�ŵ���
                if(p_dev->mp3_frame_full) continue;

                if(p_dev->file_buff_size[0]==0 && p_dev->file_buff_size[1]==0) continue;

                // ����ͬ���������, ��ʱ0.1ms
                // ��ͬ����
                swlock_acquire(&g_mp3_lock[ch]);

                //��SDRAN MUSIC_FILE_BUFF_SZ*2
                read_sdram_file_buff(c_sdram, &sdram_state, ch, p_dev->file_buff_for_used, file_buff, MUSIC_FILE_BUFF_SZ);
                file_buff_left = p_dev->file_buff_size[p_dev->file_buff_for_used]-p_dev->file_buff_offset[p_dev->file_buff_for_used];
                
                if(p_dev->file_buff_for_used==1 && p_dev->file_buff_size[0])
                {
                    read_sdram_file_buff(c_sdram, &sdram_state, ch, 0, file_buff+MUSIC_FILE_BUFF_SZ, MUSIC_FILE_BUFF_SZ);
                    file_buff_left += (p_dev->file_buff_size[0]-p_dev->file_buff_offset[0]);
                }
                else if(p_dev->file_buff_for_used==0 && p_dev->file_buff_size[1])
                {
                    read_sdram_file_buff(c_sdram, &sdram_state, ch, 1, file_buff+MUSIC_FILE_BUFF_SZ, MUSIC_FILE_BUFF_SZ);
                    file_buff_left += (p_dev->file_buff_size[1]-p_dev->file_buff_offset[1]);
                }
#if 0
                if(p_dev->mp3_frame_num>=30 && p_dev->mp3_frame_num<33)
                {
                    debug_printf("[%d] used:%d [%d %d] [%d %d] left:%d\n", 
                                 p_dev->mp3_frame_num, p_dev->file_buff_for_used,
                                 p_dev->file_buff_size[0], p_dev->file_buff_offset[0],
                                 p_dev->file_buff_size[1], p_dev->file_buff_offset[1],
                                 file_buff_left);
                }
#endif
                readptr = file_buff + p_dev->file_buff_offset[p_dev->file_buff_for_used];

                offset = MP3FindSyncWord(readptr, file_buff_left);//��readptrλ��,��ʼ����ͬ���ַ�
                if(offset < 0)//û���ҵ�ͬ���ַ�,����֡����ѭ��
    			{
                    //û�ҵ�֡ͬ���ַ�
                    //TUDO:����״̬��־����������һ֡
                    debug_printf("MP3FindSyncWord failed [%d]\n", ch);
                    debug_printf("[%d] used:%d [%d %d] [%d %d] left:%d\n", 
                                 p_dev->mp3_frame_num, p_dev->file_buff_for_used,
                                 p_dev->file_buff_size[0], p_dev->file_buff_offset[0],
                                 p_dev->file_buff_size[1], p_dev->file_buff_offset[1],
                                 file_buff_left);

                    if(p_dev->file_over_flag)
                    {
                        p_dev->file_over_flag = 0;
                        p_dev->decoder_status = MUSIC_DECODER_FILE_END;
                    }
                    else if(p_dev->decoder_status != MUSIC_DECODER_STOP && p_dev->decoder_error_cnt++ > MP3_DECODER_ERROR_MAX_CNT)
                    {
                        p_dev->decoder_status = MUSIC_DECODER_ERROR1;  
                    }
                    else
                    {
                        // ����file_buff_offset����
                        set_file_buff_offset(p_dev, file_buff_left/2);
                    }
                    
    			}
                else//�ҵ�ͬ���ַ���
    			{
                    readptr += offset;//MP3��ָ��ƫ�Ƶ�ͬ���ַ���.
                    
                    if((UnpackFrameHeader(&mp3decinfo, readptr)==-1) || (mp3decinfo.layer!=3))
                    {
                        debug_printf("UnpackFrameHeader failed [%d] %d\n", ch, mp3decinfo.layer);
                        debug_printf("file_buff_offset:%d offset:%d frame_num:%d %d\n", p_dev->file_buff_offset[p_dev->file_buff_for_used], offset, p_dev->mp3_frame_num, p_dev->file_over_flag);
                                                
                        if(p_dev->file_over_flag)
                        {
                            p_dev->file_over_flag = 0;
                            p_dev->decoder_status = MUSIC_DECODER_FILE_END;
                        }
                        else if(p_dev->decoder_status != MUSIC_DECODER_STOP && p_dev->decoder_error_cnt++ > MP3_DECODER_ERROR_MAX_CNT)
                        {
                            p_dev->decoder_status = MUSIC_DECODER_ERROR2;  
                            debug_printf("MP3 MUSIC_DECODER_ERROR2\n");
                        }
                        else
                        {
                            // ����file_buff_offset����
                            set_file_buff_offset(p_dev, offset+2);
                        }
                    }
                    else
                    {
                        p_dev->mp3_frame_size = get_mp3_frame_size(&mp3decinfo);
                        if(file_buff_left >= p_dev->mp3_frame_size)
                        {
                            offset += p_dev->mp3_frame_size;
                            
                            // ����file_buff_offset����
                            set_file_buff_offset(p_dev, offset);
                            
                            if(p_dev->bitrate != mp3decinfo.bitrate)//��������
                            {
                                p_dev->bitrate = mp3decinfo.bitrate; 
                                p_dev->samplerate = mp3decinfo.samprate;
                            }
                            
                            // put mp3 frame buff
                            memcpy(p_dev->mp3_frame, readptr, p_dev->mp3_frame_size);
                            p_dev->mp3_frame_num++;
                            p_dev->mp3_frame_full = 1;
                        }
                        else if(p_dev->file_over_flag)
                        {
                            p_dev->file_over_flag = 0;
                            p_dev->decoder_status = MUSIC_DECODER_FILE_END;
                        }
                    }
                }
                // ��ͬ����
                swlock_release(&g_mp3_lock[ch]);
            }
        }
    }
}

/*********************************************************************************************************************************/


