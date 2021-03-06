#include <xs1.h>
#include <string.h>
#include <debug_print.h>
#include "ff.h"	

#include "file.h"
#include "music_decoder.h"

#include "kfifo.h"

#include "sdram_def.h"

#include "file_operation.h"

extern kfifo_t upload_fifo;

extern uint8_t file_scaning_flag;
extern uint8_t file_scaning_overf;
extern uint8_t g_wav_mode;

extern file_contorl_s file_contorl; 

void scan_musictosec_process(); //2MS


#define FILE_SERVER_TRAINING  (50000)//0.5ms

#define FILE_TRAINING_TICK  (100000)//1ms

uint8_t file_busy_decode(){
    //debug_printf("file control busy \n");
    return FOR_BUSY;
}

#define FILE_BUSY_DECODE  file_busy_decode()


[[combinable]]
void file_server(server file_server_if if_fs, chanend c_faction)
{
    timer tmr;
    unsigned int timeout;
    
    char sdcard_status = 0;
    
    char need2action = 0;
    
    f_opr_item_t &fopr = g_fopr_mgr.item[0];
    f_opr_upload_t &fupload = g_fopr_mgr.item[0].upload;
        
    music_decoder_status_t decoder_status[MUSIC_CHANNEL_NUM];
    memset(&decoder_status, 0, sizeof(decoder_status));

    tmr :> timeout;
    timeout += FILE_SERVER_TRAINING;

    while(1)
    {
        select{
            case if_fs.music_start(uint8_t ch, task_music_info_t music_info, unsigned f_offset) -> int res:
            {
                res = FOR_SUCCEED;
                // 修改为任何时候可播放音乐
                if(fopr.result != FOR_IDLE)
                {
                    res = FILE_BUSY_DECODE;
                    break;
                }
                
                fopr.data.music.ch = ch;
                fopr.data.music.foffset = f_offset;                
                //------------------------------------------------------------------------------------------------------------
                
                text_debug("ms state %d\n",fupload.state);
                uint8_t i,j;
                // 合成路径名
                for(i=0;i<(PATCH_NAME_NUM/2);i++){
                    if(((uint16_t *)music_info.music_path)[i]==0)
                        break;
                    ((uint16_t *)fopr.data.music.fname)[i] = ((uint16_t *)music_info.music_path)[i];
                }
                ((uint16_t *)fopr.data.music.fname)[i] = 0x002F;
                i++;
                // 文件名
                for(j=0; j<(MUSIC_NAME_NUM/2); i++,j++){
                    if(((uint16_t *)music_info.music_name)[j]==0)
                        break;
                    ((uint16_t *)fopr.data.music.fname)[i] = ((uint16_t *)music_info.music_name)[j];
                }
                ((uint16_t *)fopr.data.music.fname)[i] = 0x00;
                //------------------------------------------------------------------------------------------------------------
                
                //fopr.event = FOE_MUSIC_START;
                fopr.music_start = FOE_MUSIC_START;
                fopr.music_start_contorl = FOE_MUSIC_START;
                decoder_status[ch].status = MUSIC_DECODER_START;
                
                //need2action = 1;
                
                c_faction <: (char)1;
                text_debug("ms state %d\n",fupload.state);
                break;
            }      
            case if_fs.music_stop(uint8_t ch) -> int res:
            {     
                if(ch>=MUSIC_CHANNEL_NUM)
                    break;
                music_decode_stop(ch, 1);
                
                decoder_status[ch].status = MUSIC_DECODER_STOP;
                
                res = FOR_SUCCEED;

                break;
            }
            case if_fs.music_stop_all() -> int res:
            {
                for(int i=0; i<MUSIC_CHANNEL_NUM; i++)
                {
                    music_decode_stop(i, 1);
                    decoder_status[i].status = MUSIC_DECODER_STOP;
                }

                res = FOR_SUCCEED;
                break;
            }
            case if_fs.music_jumpsec(uint8_t ch,uint16_t second):
                set_music_secjump(ch,second);
                break;
            case if_fs.get_sdcard_size(unsigned &tol_mb,unsigned &free_mb)-> int res:
                unsafe{
                unsigned long tol_mb_tmp,free_mb_tmp;
                get_sdcard_size((unsigned long *)&tol_mb_tmp,(unsigned long *)&free_mb_tmp);
                tol_mb = tol_mb_tmp;
                free_mb = free_mb_tmp;
                }
                break;
            case if_fs.file_mkdir(uint8_t f_name[n], unsigned n) -> int res:
            {
                res = FOR_SUCCEED;
                debug_printf("mkdit event %d\n",fopr.event);
                if(fopr.event != FOE_IDLE)
                {
                    res = FILE_BUSY_DECODE;
                    break;
                }

                memcpy(fopr.data.file.fsrc, f_name, n);
                fopr.event = FOE_FMKDIR;

                need2action = 1;
                break;
            }
            case if_fs.file_delete(uint8_t f_name[n], unsigned n) -> int res:
            {
                res = FOR_SUCCEED;
                if(fopr.event != FOE_IDLE)
                {
                    res = FILE_BUSY_DECODE;
                    break;
                }

                memcpy(fopr.data.file.fsrc, f_name, n);
                fopr.event = FOE_FDELETE;

                need2action = 1;
                break;
            }
            case if_fs.file_rename(uint8_t fsrc[n1], unsigned n1, uint8_t fdes[n2], unsigned n2) -> int res:
            {
                res = FOR_SUCCEED;
                if(fopr.event != FOE_IDLE)
                {
                    res = FILE_BUSY_DECODE;
                    break;
                }
                text_debug("res %d\n",res);

                memcpy(fopr.data.file.fsrc, fsrc, n1);
                memcpy(fopr.data.file.fdes, fdes, n2);
                fopr.event = FOE_FRENAME;                

                need2action = 1;
                break;
            }
            case if_fs.file_copy(uint8_t fsrc[n1], unsigned n1, uint8_t fdes[n2], unsigned n2, uint8_t mode) -> int res:
            {
                res = FOR_SUCCEED;
                if(fopr.event != FOE_IDLE)
                {
                    res = FILE_BUSY_DECODE;
                    break;
                } 
                
                memcpy(fopr.data.file.fsrc, fsrc, n1);
                memcpy(fopr.data.file.fdes, fdes, n2);
                fopr.f_copy_mode = mode;
                
                fopr.event = FOE_FCOPY;    

                need2action = 1;
                break;
            }
            case if_fs.file_move(uint8_t fsrc[n1], unsigned n1, uint8_t fdes[n2], unsigned n2,uint8_t mode) -> int res:
            {
                res = FOR_SUCCEED;
                if(fopr.event != FOE_IDLE)
                {
                    res = FILE_BUSY_DECODE;
                    break;
                }

                memcpy(fopr.data.file.fsrc, fsrc, n1);
                memcpy(fopr.data.file.fdes, fdes, n2);
                fopr.f_copy_mode = mode;
                
                fopr.event = FOE_FMOVE;

                need2action = 1;
                break;
            }
            
            case if_fs.get_fcopy_progress(uint8_t & progress) -> int res:
            {
                //if(fopr.event==FOE_FCOPY || fopr.event==FOE_FMOVE)
                //{
                progress = get_fcopy_progress();
                res = FOR_SUCCEED;                    
                //}
                //else
                //{
                //    progress = 0;
                //    res = FOR_FAILED;
                // }
                break;
            }
            case if_fs.fcopy_forced_stop() -> int res:
            {
                if(file_contorl.bat_contorl_f)
                {
                    fcopy_forced_stop();
                    res = FOR_SUCCEED;                    
                }
                else
                {
                    res = FOR_FAILED;
                }
                break;
            }

            case if_fs.file_upload_start(uint8_t f_name[n], unsigned n) -> int res:
            {
                res = FOR_SUCCEED;
                if(fopr.event != FOE_IDLE)
                {
                    res = FILE_BUSY_DECODE;
                    break;
                }
                memcpy(fupload.fname, f_name, n);
                fupload.state = FOU_STATE_START;
                fupload.reply_type = FOU_REPLY_IDLE;
                fupload.reply_data = FOU_REPLY_SUCCEED;
                fupload.lsat_buff = 0;
                
                fopr.result = FOR_IDLE;
                fopr.error_code = 0;
                fopr.event = FOE_FUPLOAD;

                need2action = 1;
                break;
            }
            case if_fs.file_upload_put_data(uint8_t buff[], unsigned buff_size, uint8_t last_buff, int &fifo_size) -> int res:
            {
                res = FOR_SUCCEED;
                if(fopr.event != FOE_FUPLOAD)
                {
                    res = FOR_FAILED;
                    break;
                }
                
                if(fupload.state != FOU_STATE_PUT_DATA)
                {
                    res = FOR_FAILED;
                    break;
                }
                
                // 复制数据到fifo,fifo必须有足够空间
                KFIFO_PUT(upload_fifo, buff, buff_size);
                fupload.lsat_buff = last_buff;
                fifo_size = KFIFO_FREE_SIZE(upload_fifo);
                
                break;
            }
            case if_fs.file_upload_get_fifo_size() -> int res:
            {
                res = KFIFO_FREE_SIZE(upload_fifo);
                break;
            }
            case if_fs.file_upload_forced_stop()-> int res:
            {
                res = FOR_SUCCEED;
                if(fopr.event != FOE_FUPLOAD)
                {
                    res = FOR_FAILED;
                    break;
                }

                if(fupload.state != FOU_STATE_PUT_DATA)
                {
                    res = FOR_FAILED;
                    break;
                }
                // 强制终止操作，异步返回信息
                file_upload_forced_stop();
                
                break;
            }
            case if_fs.log_mklog(uint8_t new_fname[newlen],unsigned newlen,uint8_t old_fname[oldlen],unsigned oldlen)-> int res:
            {
                res = FOR_FAILED;
                memcpy(fopr.data.file.fsrc, new_fname, newlen);
                memcpy(fopr.data.file.fdes, old_fname, oldlen);
                fopr.log_event = FOR_LOGMK;
                debug_printf("scan flag %d\n",file_scaning_flag);
                if(file_scaning_flag==0 && sdcard_status==0){
                    c_faction <: (char)1;
                    res = FOR_SUCCEED;
                }
                break;
            }
            case if_fs.log_loginfo_add(uint8_t log_info[len],unsigned len)-> int res:
            {
                res = FOR_SUCCEED;
                memcpy(fopr.data.file.log_info, log_info, len);
                fopr.data.file.len =len;
                fopr.log_event = FOR_LOGADD;                  
                debug_printf("scan flag %d %d\n",file_scaning_flag,sdcard_status);
                if(file_scaning_flag==0 && sdcard_status==0){                    
                    c_faction <: (char)1;        
                }
                break;
            }            
            case if_fs.setwav_mode(uint8_t wav_mode):
                g_wav_mode = wav_mode;
                break;
            case if_fs.get_notify(file_server_notify_data_t &data):
            {
                //file_server_notify_data_t data;
                if(fopr.music_start == FOE_MUSIC_START){
                    data.event = FOE_MUSIC_START;
                }
                else{
                    data.event = fopr.event;
                }
                data.result = fopr.result;
                data.error_code = fopr.error_code;
                data.sdcard_status = get_sdcard_status();
                data.scan_file_over = file_scaning_overf;
                data.f_contorl_state = fopr.f_contorl_event;
                file_scaning_overf = 0;
                for(uint8_t i=0; i<MUSIC_CHANNEL_NUM; i++)
                {
                    data.music_status[i] = decoder_status[i];
                    decoder_status[i].is_new = 0;
                }

                // tftp传输文件, 返回文件下载的回复信息
                if((fopr.event == FOE_FUPLOAD))
                {
                    data.uplaod_reply_type = fupload.reply_type;
                    data.uplaod_reply_data = fupload.reply_data;
                    
                    //结束
                    if(fupload.state == FOU_STATE_END)
                    {
                        fopr.event = FOE_IDLE;
                    }

                    //清空回复标志
                    fupload.reply_type = FOU_REPLY_IDLE;
                    fopr.result = FOR_IDLE;
                }
                // 常规异步操作数据处理 
                // 注意: 调用接口后, 会清除事件标志位, 即停止事件
                else if(fopr.result != FOR_IDLE)
                {
                    fopr.event = FOE_IDLE;
                    fopr.result = FOR_IDLE;
                    fopr.f_contorl_event = F_CONTORL_IDLE;
                }
                
                text_debug("FOE_FUPLOAD %d\n",fupload.state);
                
                fopr.music_start = 0;
                fopr.error_code = 0;//ADD:20181023

                sdcard_status = get_sdcard_status();
                
                break;
            }
            case tmr when timerafter(timeout) :> void://0.5ms
            {

                // after music_file_handle()
                if(update_music_decoder_status(decoder_status))
                {
                    text_debug("music ev\n");
                    if_fs.notify2user();
                }

                // after fopr_handle()
                if(fopr.result != FOR_IDLE)
                {
                    text_debug("fil ev %d\n",fopr.result);
                    if_fs.notify2user();
                }

                // after upload
                if(fopr.event == FOE_FUPLOAD &&
                   fupload.reply_type != FOU_REPLY_IDLE)
                {
                    text_debug("up ev\n");
                    if_fs.notify2user();
                }

                if(sdcard_status != get_sdcard_status())
                {
                    sdcard_status = get_sdcard_status();
                    if_fs.notify2user();
                }
                if(file_scaning_overf)
                    if_fs.notify2user();
                if(need2action)
                {
                    //debug_printf("event in\n");
                    c_faction <: (char)1;
                    need2action = 0;
                }
                                
                tmr :> timeout;
                timeout += FILE_SERVER_TRAINING;                    
                break;
            }
        }
    }
}

extern void mem_init(void);
extern uint8_t mem_perused(void);

static streaming chanend * unsafe pc_sdram;
static s_sdram_state sdram_state;

void fl_read_ugbk_tbl(unsigned dword[], int offset)
{
    unsafe{
        sdram_read(*pc_sdram, sdram_state, SDRAM_GBK_UNICODE_TBL_START+offset/4, 1, dword);
        sdram_complete(*pc_sdram, sdram_state);
    }
}
void fl_erase_flielist(int secoter_index)
{
}
extern void debug_dir_list(char tag[] ,const uint8_t buff[]);
void fl_read_flielist(int secoter_index, uint8_t buff[], int br)
{
    unsafe {
        sdram_read(*pc_sdram, sdram_state, SDRAM_FILE_LIST_START+secoter_index*SDRAM_FILE_LIST_SECTOR_SIZE, (br/4), (unsigned*)buff);     
        sdram_complete(*pc_sdram, sdram_state);
    };
}

void fl_write_flielist(int secoter_index, const uint8_t buff[], int bw)
{
    unsafe {
        sdram_write(*pc_sdram, sdram_state, SDRAM_FILE_LIST_START+secoter_index*SDRAM_FILE_LIST_SECTOR_SIZE, (bw/4), (unsigned*)buff);
        sdram_complete(*pc_sdram, sdram_state);
    };    
}

extern client interface sd_host_if  * unsafe sd_hi;

void file_process(streaming chanend c_sdram, chanend c_faction,client sd_host_if sdif)
{

    file_contorl_dat_init();
    timer tmr;
    unsigned int timeout;
    
    unsafe{ pc_sdram = &c_sdram; sd_hi = &sdif;}
    sdram_init_state(c_sdram, sdram_state);
    
    mem_init();
    
    fpor_mgr_init();

    music_decoder_mgr_init();
    
    delay_milliseconds(1500);
    
    my_fatfs_init();
    
    tmr :> timeout;
    timeout += FILE_TRAINING_TICK;
    
    #if 0
    // TEXT  
    uint8_t tmpbuf[8*1024];
    unsigned t1,t2;
    FIL logfile;
    unsigned res,bw;
    memset(tmpbuf,0x31,8*1024);
    char text[12] = {0x31,0x00,0x31,0x00,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    res = f_open(&logfile,(const TCHAR *)text,FA_WRITE|FA_READ|FA_OPEN_ALWAYS);   //打开创建文件
    
    text_debug("\n\nsize %d\n\n",logfile.fsize);
    
    text_debug("open res %d\n",res);
    for(uint8_t i=0;i<10;i++){
        tmr :>t1;     
        res = f_write(&logfile,(const TCHAR*)tmpbuf, 8*1024, &bw);
        tmr :>t2; 
        text_debug("file write t2-t1 %d res %d fptr %x bw %d\n",t2-t1,res,logfile.fptr,bw);
    }
    text_debug("\n\nend\n\n");
    
    text_debug("\n\nsize %d\n\n",logfile.fsize);
    
    logfile.fptr =0;
    for(uint8_t i=0;i<10;i++){
        tmr :>t1;     
        res = f_read(&logfile,(const TCHAR*)tmpbuf,8*1024 , &bw);
        tmr :>t2; 
        text_debug("file read t2-t1 %d res %d fptr %x bw %d\n",t2-t1,res,logfile.fptr,bw);
    }
    text_debug("\n\nend\n\n");

    text_debug("\n\nsize %d\n\n",logfile.fsize);
    
    f_close(&logfile);
    while(1);
    #endif 
    //---------------------------------------------------------------------------------------------------
    while(1)
    {
        select{
            case tmr when timerafter(timeout) :> void://1ms
            {

                sdcard_hot_swap_check();
                
                music_file_handle(c_sdram, sdram_state);

                upload_handle(1);
                                
                scan_musictosec_process();

                file_copy_process();
                
                tmr :> timeout;
                timeout += FILE_TRAINING_TICK;
                break;
            }
            case c_faction :> char val: 
            {
                fopr_handle();
                break;
            }
        }
    }

}

unsigned get_timer(){
    static timer tmr;
    unsigned tmp;
    tmr :> tmp;
    return tmp;
}

