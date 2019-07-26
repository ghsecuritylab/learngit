#include <string.h>
#include <debug_print.h>
#include "mymalloc.h"
#include "ff.h"
#include "mp3dec.h"
#include "file_list.h"
#include "mystring.h"

#define FILE_LIST_DEBUG_ENBLE 1

#if FILE_LIST_DEBUG_ENBLE
#define DBG_PRINTF(...) debug_printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif

#define F_START_MUSIC_SECTOR    1


extern unsigned int get_mp3_totsec(TCHAR *pname);
extern unsigned int get_wav_totsec(TCHAR *pname);

extern void fl_erase_flielist(int secoter_index);
extern void fl_read_flielist(int secoter_index, unsigned char buff[], int br);
extern void fl_write_flielist(int secoter_index, const unsigned char buff[], int bw);


uint8_t file_scaning_flag=1;
uint8_t file_scaning_overf=0;
uint8_t g_wav_mode=0;


TCHAR ROOT_PATH[] = {0, 0};
//�����ļ�������
//fname:�ļ���
//����ֵ:0XFF,��ʾ�޷�ʶ����ļ����ͱ��.
//       1, mp3�ļ�
//       2, wav�ļ�
unsigned char mf_typetell(TCHAR *fname)
{
    const TCHAR mp31[] = {'m','p','3', 0};
    const TCHAR mp32[] = {'M','P','3', 0};
    const TCHAR wav31[]={'w','a','v',0};
    const TCHAR wav32[]={'W','A','V',0};
    TCHAR *attr = 0;//��׺��
    unsigned char i=0;
    while(i<250)
    {
        i++;
        if(*fname=='\0')break;//ƫ�Ƶ��������.
        fname++;
    }
    if(i==250)return 0XFF;//������ַ���.
    for(i=0; i<5; i++) //�õ���׺��
    {
        fname--;
        if(*fname=='.')
        {
            fname++;
            attr=fname;
            break;
        }
    }

    if(attr == 0) return 0xFF;

    if(wstrcmp(mp31, attr)==0 ||
       wstrcmp(mp32, attr)==0)
        return 1;

    if(wstrcmp(wav31, attr)==0 ||
       wstrcmp(wav32, attr)==0)
       return 2;

    return 0XFF;//û�ҵ�
}
void printfstr(TCHAR *str)
{

    if(sizeof(TCHAR)==1)
    {
        DBG_PRINTF("%s", str);
        return;
    }
    while(*str)
    {
        if(*str>0xff)
        {
            DBG_PRINTF("%c%c", (*str)>>8, *str);
        }
        else
        {
            DBG_PRINTF("%c", *str);
        }
        str++;
    }
}

//�����ļ�
//used ram:1536 + 2148(mp3_get_info) = 3684
//path:·��
//mark:0,�����ļ���;1,����Ŀ���ļ�;2,����Ŀ���ļ�,�ھɵĻ���������µ�Ŀ���ļ�
//buff:�洢�б��buff
//buff_size:buff�Ĵ�С
//num:�����ļ���Ŀ
//����ֵ:ִ�н��
char mf_scan_files(TCHAR *path, char mark, unsigned char *buff, int buff_size, int *num, uint8_t *buff_full,uint16_t *dir_index)
{
    //debug_printf("\n\n sacn in \n\n");   
    int i = 0;
    FRESULT res;
    unsigned char type;
    unsigned int offset = 0;
    dir_info_t *pt_di = NULL;
    music_info_t *pt_mi = NULL;

    TCHAR *fn;   /* This function is assuming non-Unicode cfg. */

    DIR *dir = mymalloc(sizeof(DIR));
    FILINFO *fno = mymalloc(sizeof(FILINFO));

    TCHAR *music_path = mymalloc(160);

#if _USE_LFN
    TCHAR *lfname = mymalloc(_MAX_LFN * 2 + 1);
    fno->lfsize = _MAX_LFN * 2 + 1;
    fno->lfname = &lfname[0];
#endif
    if(dir==NULL || fno==NULL || music_path==NULL || lfname==NULL)
    {
        debug_printf("mf_scan_files malloc failed\n");
#if _USE_LFN
        myfree(lfname);
#endif
        myfree(music_path);
        myfree(dir);
        myfree(fno);
        return 0xff;        
    }
    if(mark == 0 || mark == 1)
    {
        *num = 0;
        memset(buff, 0, buff_size);
    }
    else if(mark == 2)
    {
        offset = sizeof(music_info_t)*(*num);
        memset(buff+offset, 0, buff_size-offset);
    }
    
    res = f_opendir(dir,(const TCHAR*)path); //��һ��Ŀ¼
    if (res == FR_OK)
    {
        if(buff_full) *buff_full = 0;

        while(1)
        {
            res = f_readdir(dir, fno);                   //��ȡĿ¼�µ�һ���ļ�
            if (res != FR_OK || fno->fname[0] == 0) break;  //������/��ĩβ��,�˳�

#if _USE_LFN
            fn = *fno->lfname ? fno->lfname : fno->fname;
#else
            fn = fno->fname;
#endif                                                /* It is a file. */
            //debug_printf("  + ");
            //printfstr(fn);
            //debug_printf("  :%d %d\n", (fno->fattrib&(AM_DIR)), (fno->fattrib&(AM_HID)));
            if(fno->fattrib&(AM_HID)) continue;//���������ļ�
            
            //debug_printf("fno %d\n",dir->index);
            if((fno->fattrib&(AM_DIR))&&mark==0)//��һ���ļ�����mark=0
            {

                if(wstrlen(fn) > (DIR_NAME_SIZE/2)) continue;//�����ļ�������

                if(offset+sizeof(dir_info_t) < buff_size)
                {
                    pt_di = (dir_info_t*)&buff[offset];
                    
                    memset(pt_di->name, 0, sizeof(pt_di->name));//APP����Ҫ�ڽ�ֹ�������
                    wstrcpy(pt_di->name, fn);
                    
                    offset += sizeof(dir_info_t);
                    (*num)++;
                }
                else
                {
                    debug_printf(" buff_size over\n");
                    break;
                }

            }
            else if((fno->fattrib&(AM_ARC))&&mark==1) //��һ���鵵�ļ���mark=1
            {
                if(wstrlen(fn) > ((MUSIC_NAME_SIZE-2)/2)) continue;//�����ļ�������

                type=mf_typetell(fn);    //�������
                
                if(type == 1)//mp3�ļ�
                {
                    if(offset+sizeof(music_info_t) < buff_size)
                    {
                        TCHAR g[] = {'/',0};
                        pt_mi = (music_info_t*)&buff[offset];
                        //pt_mi->type = type;

                        wstrcpy(music_path, (const TCHAR*)path);
                        wstrcat(music_path, g);
                        wstrcat(music_path, (const TCHAR*)fn);
                        pt_mi->totsec = 0;//get_mp3_totsec((TCHAR*)music_path);
                        
                        //if(pt_mi->totsec == 0) continue;

                        memset(pt_mi->name, 0, sizeof(pt_mi->name));//APP����Ҫ�ڽ�ֹ�������
                        wstrcpy(pt_mi->name, fn);
                        offset += sizeof(music_info_t);
                        (*num)++;                        
                        // ����ļ�indexֵ
                        *dir_index = dir->index;
                    }
                    else
                    {
                        if(buff_full) *buff_full = 1;
                        debug_printf(" buff_size over\n");
                        break;
                    }
                }
                //#if WAV_FILE_ENABLE
                else if(type == 2)//wav�ļ�
                {
                    if(offset+sizeof(music_info_t) < buff_size)
                    {
                        TCHAR g[] = {'/',0};
                        pt_mi = (music_info_t*)&buff[offset];
                        //pt_mi->type = type;

                        wstrcpy(music_path, (const TCHAR*)path);
                        wstrcat(music_path, g);
                        wstrcat(music_path, (const TCHAR*)fn);

                        pt_mi->totsec = 0;//get_wav_totsec((TCHAR*)music_path);

                        //if(pt_mi->totsec == 0) continue;

                        memset(pt_mi->name, 0, sizeof(pt_mi->name));//APP����Ҫ�ڽ�ֹ�������
                        wstrcpy(pt_mi->name, fn);
                        offset += sizeof(music_info_t);
                        (*num)++;
                        // ����ļ�indexֵ
                        *dir_index = dir->index;
                    }
                    else
                    {
                        if(buff_full) *buff_full = 1;
                        debug_printf(" buff_size over\n");
                        break;
                    }
                }
                //#endif
                else 
                {
                    continue;  //����Ҫ������
                }
            }
            else if((fno->fattrib&(AM_ARC))&&mark==2) //��һ���鵵�ļ���mark=2
            {
                if(wstrlen(fn) > (MUSIC_NAME_SIZE/2)) continue;//�����ļ�������

                type=mf_typetell(fn);    //�������

                if(type == 1)//mp3�ļ�
                {
                    if(offset+sizeof(music_info_t) < buff_size)
                    {
                        //�ų���ͬ�ļ�����
                        for(i=0; i<(*num); i++)
                        {
                            if(wstrcmp(fn, ((music_info_t*)&buff[0])[i].name) == 0) break;
                        }
                        if(i != (*num)) continue;
                        
                        TCHAR g[] = {'/',0};
                        pt_mi = (music_info_t*)&buff[offset];
                        //pt_mi->type = type;
                        
                        wstrcpy(music_path, (const TCHAR*)path);
                        wstrcat(music_path, g);
                        wstrcat(music_path, (const TCHAR*)fn);
                        pt_mi->totsec = get_mp3_totsec((TCHAR*)music_path);

                        if(pt_mi->totsec == 0) continue;
                        
                        memset(pt_mi->name, 0, sizeof(pt_mi->name));//APP����Ҫ�ڽ�ֹ�������
                        wstrcpy(pt_mi->name, fn);
                        offset += sizeof(music_info_t);
                        (*num)++;
                        // ����ļ�indexֵ
                        if(dir->index>*dir_index){
                            *dir_index = dir->index;
                        }
                    }
                    else
                    {
                        if(buff_full) *buff_full = 1;
                        debug_printf(" buff_size over\n");
                        break;
                    }
                }
                #if WAV_FILE_ENABLE
                else if(type == 2)//wav�ļ�
                {
                    if(offset+sizeof(music_info_t) < buff_size)
                    {                      
                        //�ų���ͬ�ļ�����
                        for(i=0; i<(*num); i++)
                        {
                            if(wstrcmp(fn, ((music_info_t*)&buff[0])[i].name) == 0) break;
                        }
                        if(i != (*num)) continue;
                        //
                        TCHAR g[] = {'/',0};
                        pt_mi = (music_info_t*)&buff[offset];
                        //pt_mi->type = type;

                        wstrcpy(music_path, (const TCHAR*)path);
                        wstrcat(music_path, g);
                        wstrcat(music_path, (const TCHAR*)fn);
                        
                        pt_mi->totsec = get_wav_totsec((TCHAR*)music_path);

                        if(pt_mi->totsec == 0) continue;

                        memset(pt_mi->name, 0, sizeof(pt_mi->name));//APP����Ҫ�ڽ�ֹ�������
                        wstrcpy(pt_mi->name, fn);
                        offset += sizeof(music_info_t);
                        (*num)++;
                        // ����ļ�indexֵ
                        if(dir->index>pt_di->*dir_index ){
                            *dir_index = dir->index;
                        }
                    }
                    else
                    {
                        if(buff_full) *buff_full = 1;
                        debug_printf(" buff_size over\n");
                        break;
                    }
                }
                #endif
                else 
                {
                    continue;  //����Ҫ������
                }
            }            
            else 
            {
                continue;      //��������һ��
            }

        }
    }
#if _USE_LFN
    myfree(lfname);
#endif
    myfree(music_path);
    myfree(dir);
    myfree(fno);
    
    //debug_printf("\n\n sacn out \n\n");   
    return res;
}


void printf_dir_info(dir_info_t tbl[], int num)
{
    for(int i=0; i<num; i++)
    {
        DBG_PRINTF("-");
        printfstr(tbl[i].name);
        DBG_PRINTF("-%d-%d\n", tbl[i].sector, tbl[i].music_num);
    }
}


void printf_music_info(music_info_t tbl[], int num)
{
    for(int i=0; i<num; i++)
    {
        DBG_PRINTF("  ");
        printfstr(tbl[i].name);
        DBG_PRINTF(":%d\n", tbl[i].totsec);
    }
    DBG_PRINTF(" \n");
}

void debug_clean_flash_filelist()
{
    fl_erase_flielist(0);
}

void debug_display_sdram_filelist(uint8_t *dir_tbl, uint8_t *music_tbl)
{
    unsigned int dir_num = 0;
    unsigned int music_num = 0;

    dir_info_t *pt_fldi = (dir_info_t *)(dir_tbl+sizeof(int));

    fl_read_flielist(0, dir_tbl, F_DIR_TBL_BYTE_SIZE);
    dir_num = *(unsigned int*)dir_tbl;
    if(dir_num==0 || dir_num>F_DIR_MAX_NUM)
    {
        DBG_PRINTF("fl_filelist don`t init\n");
        return;
    }
    DBG_PRINTF("fl_filelist :%d %d \n", dir_num, mem_perused());
    for(int i=0; i<dir_num; i++)
    {
        DBG_PRINTF("[%d] dir:", pt_fldi[i].sector);
        printfstr(pt_fldi[i].name);
        DBG_PRINTF(" \n");
        fl_read_flielist(pt_fldi[i].sector, music_tbl, F_MUSIC_TBL_BYTE_SIZE);

        music_num = *(unsigned int*)music_tbl;
        if(music_num>F_MUSIC_MAX_NUM || music_num==0)
        {
            DBG_PRINTF("this file is empty\n\n");
            continue;
        }
        printf_music_info((music_info_t*)(music_tbl+sizeof(int)), music_num);
    }
}

void sd_scan_test()
{
    int num = 0;

    uint8_t *tbl = mymalloc(F_DIR_TBL_BYTE_SIZE);

    mf_scan_files(ROOT_PATH, 0, tbl, F_DIR_TBL_BYTE_SIZE, &num, NULL,NULL);

    printf_dir_info((dir_info_t*)tbl, num);

    memset(tbl, 0, F_DIR_TBL_BYTE_SIZE);

    mf_scan_files(ROOT_PATH, 1, tbl, F_DIR_TBL_BYTE_SIZE, &num, NULL,NULL);

    printf_music_info((music_info_t*)tbl, num);

    myfree(tbl);
}


//index 0 ������
//TUDO:���׳���bug�����������
#define SECTOR_RECORD_INIT(rceord)   memset(rceord, 0, F_DIR_MAX_NUM+1);rceord[0] = 1;
#define SET_SECTOR_RECORD(record, i)  record[i] = 1;

uint8_t find_idle_sector(uint8_t info[], int max_num)
{

    for(int i=0; i<max_num+1; i++)
    {
        if(info[i] == 0) return i;
    }

    return 0xFF;
}

static void debug_dir_list(char tag[] ,dir_tbl_t *dir_tbl)
{
    int i, j, stop;
    for(i=0; i<dir_tbl->num; i++)
    {
        stop = 0;
        for(j=0; j<(sizeof(dir_tbl->m[0].name)/sizeof(dir_tbl->m[0].name[0])); j++)
        {
            if(dir_tbl->m[i].name[j] == 0) stop = 1;
            if(stop && dir_tbl->m[i].name[j]) debug_printf("%s dir_info [%d] error\n", tag, i);
        }
    } 
}
//���������ַ�����ֹ������ַ���
static void clean_dir_info_name(dir_tbl_t *dir_tbl)
{
    int i, j, stop;
    for(i=0; i<dir_tbl->num; i++)
    {
        stop = 0;
        for(j=0; j<(sizeof(dir_tbl->m[0].name)/sizeof(dir_tbl->m[0].name[0])); j++)
        {
            if(dir_tbl->m[i].name[j] == 0) stop = 1;
            if(stop && dir_tbl->m[i].name[j]) dir_tbl->m[i].name[j]=0;
        }
    }
}
// 0-succeed 1-failed
static int scan_music_filelist(TCHAR *path, dir_info_t *dir_info, music_tbl_t *music_tbl)
{
    int music_num = 0;
    int res = 0;
    //������Ӧ�ļ��е������б�
    res = mf_scan_files(path, 1, (uint8_t*)&music_tbl->m[0], sizeof(music_tbl->m), &music_num, &dir_info->music_num_full,&dir_info->music_index);
    if(res != 0)
    {
        return 1;
    }
    if(music_num > F_MUSIC_MAX_NUM)
    {
        music_num = F_MUSIC_MAX_NUM;
        dir_info->music_num_full = 1;
    }
    dir_info->music_num = music_num;
    music_tbl->num = music_num;

    return 0;
}
/*
flash
sector[0]:�ļ�����(int)+�ļ����б�(dir_info_t�б�)
sector[1]:������(int)+�����б�(music_info_�б�)
sector[2]:������(int)+�����б�(music_info_�б�)
    .
    .
    .
sector[30]

1.����ļ����������ļ���Ϊ0��Ҳ��д��flash��¼

*/
//used ram:8*1024+2*2*1024+34 + 3684(mf_scan_files) < 16*1024 
void sd_scan_music_file(uint8_t *specify_path)
{

    file_scaning_flag = 1;

    int i, j, res;
    int dir_num = 0;
    
    uint8_t del_dir_num = 0;
    uint8_t add_dir_num = 0;

    dir_tbl_t *dir_tbl_sdram = mymalloc(F_DIR_TBL_BYTE_SIZE);
    dir_tbl_t *dir_tbl       = mymalloc(F_DIR_TBL_BYTE_SIZE);
    music_tbl_t *music_tbl   = mymalloc(F_MUSIC_TBL_BYTE_SIZE);
    uint8_t *sector_record   = mymalloc(F_DIR_MAX_NUM+1);
    
    if(dir_tbl_sdram==NULL || dir_tbl==NULL || music_tbl==NULL || sector_record==NULL)
    {
        debug_printf("sd_scan_music_file malloc failed\n");
        goto FUN_END;
    }

    SECTOR_RECORD_INIT(sector_record);

    //��sdram�����ļ����б�sector0
    fl_read_flielist(0, (uint8_t*)dir_tbl_sdram, F_DIR_TBL_BYTE_SIZE);
    
    //��SD�������ļ����б�
    res = mf_scan_files(ROOT_PATH, 0, (uint8_t*)dir_tbl->m, sizeof(dir_tbl->m), &dir_num, NULL,NULL);
    if(res != 0)
    {
        goto FUN_END;
    }

    //sd��Ϊ��
    if(dir_num == 0)
    {
        //ɾ��flash���ļ����б�, ����sector
        DBG_PRINTF("sd card is empty\n");
        memset(dir_tbl, 0, F_DIR_TBL_BYTE_SIZE);
        fl_write_flielist(0, (uint8_t*)dir_tbl, F_DIR_TBL_BYTE_SIZE);
        goto FUN_END;
    }
    
    debug_printf("p_fld_num 0x%x 0x%x\n\n", dir_tbl_sdram->num, dir_num);

    dir_num = (F_DIR_MAX_NUM>dir_num) ? dir_num : F_DIR_MAX_NUM;
    dir_tbl->num = dir_num;
    dir_tbl_sdram->num = (F_DIR_MAX_NUM>dir_tbl_sdram->num) ? dir_tbl_sdram->num : 0;
    
    clean_dir_info_name(dir_tbl_sdram);
    clean_dir_info_name(dir_tbl);
    //debug_dir_list("sdram", dir_tbl_sdram);
    //debug_dir_list("sdcard", dir_tbl);
    
    //sdram�б�Ϊ�գ���һ�γ�ʼ��
    if(dir_tbl_sdram->num==0 || dir_tbl_sdram->num>F_DIR_MAX_NUM)
    {
        dir_tbl->num = dir_num;
        for(i=0; i<dir_num; i++)
        {
            dir_tbl->m[i].sector = i+F_START_MUSIC_SECTOR;//sector[0]�洢�ļ����б�
            dir_tbl->m[i].music_index = 0; 
            //dir_tbl->m[i].user_index = 0;
            //������Ӧ�ļ��е������б�
            if(scan_music_filelist(dir_tbl->m[i].name, &dir_tbl->m[i], music_tbl)!=0) goto FUN_END;
            
            //music_tblд��sdram sector[dir_tbl->m[i].sector]
            fl_write_flielist(dir_tbl->m[i].sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);
        }

        //dir_tblд��sdram sector[0]
        fl_write_flielist(0, (uint8_t*)dir_tbl, F_DIR_TBL_BYTE_SIZE);

#if FILE_LIST_DEBUG_ENBLE
        //debug_display_sdram_filelist((uint8_t*)dir_tbl, (uint8_t*)music_tbl);
#endif

        goto FUN_END;
    }

    
    DBG_PRINTF("start cmp dir list\n\n");
    
    //�Ƚ������ļ����б�����ɾ����ɾ����Ӧsector�б���������ӵ��յ�sector�б�
    add_dir_num = dir_num;
    for(i=0; i<dir_tbl_sdram->num; i++)
    {
        char is_found_dir = 0;
        //dir_tbl�б� ȥ���Ѵ��ڵģ�ʣ����������
        for(j=0; j<dir_num; j++)
        {
            if(wstrcmp(dir_tbl_sdram->m[i].name, dir_tbl->m[j].name) == 0)
            {
                is_found_dir = 1;
                memset(&dir_tbl->m[j], 0, sizeof(dir_info_t));
                SET_SECTOR_RECORD(sector_record, dir_tbl_sdram->m[i].sector);//�����ʹ�õ�sector
                add_dir_num--;//���ʣ���������ļ�����
                break;
            }
        }

        //ɾ�������ڵ�,�����sector���Բ�����
        if(is_found_dir == 0)
        {
            DBG_PRINTF("del dir:%s\n", dir_tbl_sdram->m[i].name);
            memset(&dir_tbl_sdram->m[i], 0, sizeof(dir_info_t));
            del_dir_num++;
        }
    }
    
    DBG_PRINTF("del_dir_num:%d add_dir_num:%d\n\n", del_dir_num, add_dir_num);

    //�����dir_tbl_sdramɾ���������ļ����б�
    if(del_dir_num)
    {
        int right = 0;
        for(int left=0; left<dir_tbl_sdram->num; left++)
        {
            if(dir_tbl_sdram->m[left].name[0]==0)//�ҵ��յ���Ϣ��
            {
                if(right==0) right = left+1;

                while(right < dir_tbl_sdram->num)
                {
                    if(dir_tbl_sdram->m[right].name[0] != 0)//�ҵ���Ϣ�鲢��ǰ���
                    {
                        dir_tbl_sdram->m[left] = dir_tbl_sdram->m[right];
                        memset(&dir_tbl_sdram->m[right], 0, sizeof(dir_info_t));
                        break;
                    }
                    right++;
                }
            }
            //�����ʹ�õ�sector,���Ϊ�գ������sector0
            SET_SECTOR_RECORD(sector_record, dir_tbl_sdram->m[left].sector);
        }
        dir_tbl_sdram->num -= del_dir_num;
    }
    
    //�ҵ������������ļ��У��ҵ������sector
    if(add_dir_num)
    {
        int add_num = add_dir_num;
        int index = 0;

        for(i=dir_tbl_sdram->num; i<F_DIR_MAX_NUM && add_num; i++)
        {
            //��ȡ��������Ϣ��
            while(index < dir_num)
            {
                if(dir_tbl->m[index].name[0] != 0)
                {
                    dir_tbl->m[index].music_index=0;
                    dir_tbl_sdram->m[i] = dir_tbl->m[index];

                    //�ҵ����е�index
                    dir_tbl_sdram->m[i].sector = find_idle_sector(sector_record, F_DIR_MAX_NUM);
                    SET_SECTOR_RECORD(sector_record, dir_tbl_sdram->m[i].sector);
                    DBG_PRINTF("scan_music_filelist add in %d %d %d %d : ", mem_perused(), (uint32_t)music_tbl, (uint32_t)music_tbl->m, sizeof(music_tbl->m));
                    printfstr(dir_tbl_sdram->m[i].name);
                    DBG_PRINTF("\n");
                    //��SD����Ӧ�����ļ��е������ļ�
                    if(scan_music_filelist(dir_tbl_sdram->m[i].name, &dir_tbl_sdram->m[i], music_tbl)!=0) goto FUN_END;
                    DBG_PRINTF("scan_music_filelist add out\n");
                    //���µ�sdram, дmusic_tbl��sector[dir_tbl_sdram->m[i].sector]
                    fl_write_flielist(dir_tbl_sdram->m[i].sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);

                    add_num--;
                    index++;
                    break;

                }
                index++;
            }
        }

        dir_tbl_sdram->num += add_dir_num;

    }

    if(specify_path)
    {
        // �����������ļ���, ����ʣ�������ļ���
        for(i=0; i<(dir_tbl_sdram->num-add_dir_num); i++)
        {
            if(wstrcmp(dir_tbl_sdram->m[i].name, (wchar*)specify_path) != 0) continue;
            
            //��SD����Ӧ�ļ��������б�
            if(scan_music_filelist(dir_tbl_sdram->m[i].name, &dir_tbl_sdram->m[i], music_tbl)!=0) goto FUN_END;            
            
            //���µ�sdram��дmusic_tbl��sector[dir_tbl_sdram->m[i].sector]
            fl_write_flielist(dir_tbl_sdram->m[i].sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);
            DBG_PRINTF("update specify dir:%s sector:%d\n\n", dir_tbl_sdram->m[i].name, dir_tbl_sdram->m[i].sector);
            break;
        }
    }
    else
    {
        // ��������ɾ�������ļ���, ����ʣ�������ļ���
        for(i=0; i<(dir_tbl_sdram->num-add_dir_num); i++)
        {
            //��SD����Ӧ�ļ��������б�
            if(scan_music_filelist(dir_tbl_sdram->m[i].name, &dir_tbl_sdram->m[i], music_tbl)!=0) goto FUN_END;            
            
            //���µ�sdram��дmusic_tbl��sector[dir_tbl_sdram->m[i].sector]
            fl_write_flielist(dir_tbl_sdram->m[i].sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);
            DBG_PRINTF("update dir:%s sector:%d\n\n", dir_tbl_sdram->m[i].name, dir_tbl_sdram->m[i].sector);
        }
    }


    //����:dir_tbl_sdramд��sdram
    DBG_PRINTF("update dir_tbl_sdram\n");
    fl_write_flielist(0, (uint8_t*)dir_tbl_sdram, F_DIR_TBL_BYTE_SIZE);
    
#if FILE_LIST_DEBUG_ENBLE
    //debug_display_sdram_filelist((uint8_t*)dir_tbl, (uint8_t*)music_tbl);
#endif

FUN_END:
    myfree(dir_tbl_sdram);
    myfree(dir_tbl);
    myfree(music_tbl);
    myfree(sector_record);
    
    file_scaning_flag = 0;
}



static void find_path(TCHAR *dst, const TCHAR *src, int level)
{
	TCHAR *d;
    if (*src == '/' || *src == '\\')  /* Strip heading separator if exist */
      src++;
    do
    {
		d = dst;
        while(*src!=0 && *src!='/' && *src!='\\') *d++ = *src++;
        *d = 0;
        level--;
		if(*src!=0) src++;
    }while(level>0);
}
/*
* ����·��: ���ڸ���/�ƶ�1-2/ɾ���ļ�/�ϴ��ļ�
* һ��·��: �����½������ļ���(���ļ���)��ɾ�������ļ��С������������ļ���
*               ��Ҫ���ԭ�������ļ��б�, ����Ҫ����SD�������ļ��б�
*/
void update_music_filelist(uint8_t path[], uint8_t is_del)
{
    int i = 0;
    
    int dir_sector = -1;
    int dir_index = -1;
    int music_index = -1;
    int music_totsec = 0;
    
    uint8_t *dir_name = mymalloc(255*2);
    uint8_t *music_name = mymalloc(255*2);
    
    dir_tbl_t *dir_tbl_sdram = mymalloc(F_DIR_TBL_BYTE_SIZE);
    music_tbl_t *music_tbl = mymalloc(F_MUSIC_TBL_BYTE_SIZE);

    debug_printf("updat in\n");

    if(dir_tbl_sdram==NULL || music_tbl==NULL || dir_name==NULL || music_name==NULL)
    {
        debug_printf("update_music_filelist malloc failed\n");
        goto FUN_END;
    }
        
    find_path((TCHAR*)dir_name, (TCHAR*)path, 1);
    find_path((TCHAR*)music_name, (TCHAR*)path, 2);

    if(wstrlen((TCHAR*)dir_name) != 0) i++;
    if(wstrlen((TCHAR*)music_name) != 0) i++;
    debug_printf("update_music_filelist %d level\n", i);
    if(i == 0)          // ����·��
    {
        goto FUN_END;
    }
    else if(i == 1)     // һ��·��
    {
        goto filelist_layer_1;
    }
    else                // ����·��
    {
        goto filelist_layer_2;
    }
    
filelist_layer_1:   //һ��·�������߼�        
    myfree(dir_tbl_sdram);
    myfree(music_tbl);
    dir_tbl_sdram = NULL;
    music_tbl = NULL;
    sd_scan_music_file(dir_name);
    goto FUN_END;    
    
filelist_layer_2:   //����·�������߼�  
    
    //��ȡsdram�����ļ����б�
    fl_read_flielist(0, (uint8_t*)dir_tbl_sdram, F_DIR_TBL_BYTE_SIZE);
    
    //���Ҷ�Ӧ�����ļ��������ļ���sector
    for(i=0; i<dir_tbl_sdram->num; i++)
    {
        if(wstrcmp((TCHAR*)dir_tbl_sdram->m[i].name, (TCHAR*)dir_name) == 0)
        {
            dir_sector = dir_tbl_sdram->m[i].sector;
            dir_index = i;
            break;
        }
    }
    if(dir_sector == -1) goto FUN_END;
    
    //��ȡ��Ӧ�����ļ��е������ļ����б�
    fl_read_flielist(dir_sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);
    
    DBG_PRINTF("update_music_filelist sector[%d] music_num[%d]\n", dir_sector, *(unsigned int*)music_tbl);
    
    //���Ҷ�Ӧ�ļ��������ļ�index
    for(i=0; i<music_tbl->num; i++)
    {
        if(wstrcmp((TCHAR*)music_tbl->m[i].name, (TCHAR*)music_name) == 0)
        {
            music_index = i;
            break;
        }
    }
    
    unsigned char type=mf_typetell(path);    //�������
    if(type==1){
        music_totsec = get_mp3_totsec((TCHAR*)path);
    }else if(type==2){
        music_totsec = get_wav_totsec((TCHAR*)path);
    }else{
        music_totsec=0;
    }
    DBG_PRINTF("music_totsec[%d] music_index[%d]\n", music_totsec, music_index);
    //�����б����Ҳ�����������Ϣ
    if(music_index == -1)
    {
        //����һ��������Ϣ
        if(music_totsec) 
        {
            i = music_tbl->num;
            if(music_tbl->num < F_MUSIC_MAX_NUM)
            {
                music_tbl->m[i].totsec = music_totsec;
                memset(music_tbl->m[i].name, 0, sizeof(music_tbl->m[i].name));
                wstrcpy((TCHAR*)music_tbl->m[i].name, (TCHAR*)music_name);
                if(++music_tbl->num)
                    dir_tbl_sdram->m[dir_index].music_num_full = 0;
                dir_tbl_sdram->m[dir_index].music_num = music_tbl->num;
                DBG_PRINTF("  add new music_totsec dir_music_num:%d\n", dir_tbl_sdram->m[dir_index].music_num);
            }
            else
            {
                dir_tbl_sdram->m[dir_index].music_num_full = 1;
            }
        }
    }
    else
    {
        //�޸�����������Ϣ
        if(music_totsec) 
        {
            music_tbl->m[music_index].totsec = music_totsec;
            DBG_PRINTF("  update music_info_t music_totsec:%d\n", music_totsec);
        }
        //ɾ������������Ϣ
        else
        {
            i = music_tbl->num-music_index-1;
            if(i > 0)
                memmove(&music_tbl->m[music_index], &music_tbl->m[music_index+1], sizeof(music_info_t)*i);
            music_tbl->num--;
            dir_tbl_sdram->m[dir_index].music_num = music_tbl->num;
            
            //�������������ļ��������ļ���ʱ, ɾ�������ļ���Ҫ������ѵ�б�
            if(dir_tbl_sdram->m[dir_index].music_num_full && is_del)
            {
                mf_scan_files((TCHAR *)dir_name, 2, (uint8_t*)&music_tbl->m[0], sizeof(music_tbl->m), (int*)&music_tbl->num, &dir_tbl_sdram->m[dir_index].music_num_full,&dir_tbl_sdram->m[dir_index].music_index);
                if(music_tbl->num > F_MUSIC_MAX_NUM)
                {
                    music_tbl->num = F_MUSIC_MAX_NUM;
                    dir_tbl_sdram->m[dir_index].music_num_full = 1;
                }
                dir_tbl_sdram->m[dir_index].music_num = music_tbl->num;          
            }
            DBG_PRINTF("  del old music_totsec dir_music_num:%d\n", dir_tbl_sdram->m[dir_index].music_num);
        }
    }
    //����sdram�����б�
    fl_write_flielist(0, (uint8_t*)dir_tbl_sdram, F_DIR_TBL_BYTE_SIZE);
    fl_write_flielist(dir_sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);

FUN_END:
    myfree(dir_tbl_sdram);
    myfree(music_tbl);
    myfree(dir_name);
    myfree(music_name);
}

typedef struct{
    uint8_t scan_file_f;    // ��ʼɨ���־
    uint8_t scan_dir_cnt;   // �ļ��м���
    uint8_t scan_file_cnt;  // �����ļ�����
    uint8_t need_open_dir;
    FILINFO fno;    
    DIR dir;
    TCHAR  lfname[_MAX_LFN * 2 + 1];
    unsigned tim_cnt;
}scan_musicsec_s;

static scan_musicsec_s s_scan_musicsec={0};

void scan_musictosec_init(){
    s_scan_musicsec.scan_file_f = 1;
    s_scan_musicsec.scan_file_cnt = 0;
    s_scan_musicsec.scan_dir_cnt = 0;  
    s_scan_musicsec.tim_cnt = 0;
    #if _USE_LFN
    s_scan_musicsec.fno.lfsize = _MAX_LFN * 2 + 1;
    s_scan_musicsec.fno.lfname = &s_scan_musicsec.lfname[0];
    #endif
    
    s_scan_musicsec.need_open_dir=1;
}

void scan_musictosec_clear(){
    s_scan_musicsec.scan_file_f = 0;
}

void scan_musictosec_process(){ //2MS
    uint8_t res;
    // �Ƿ����ɨ��ʱ��ģʽ
    if(s_scan_musicsec.scan_file_f==0)
        return;
    s_scan_musicsec.tim_cnt++;
    if(s_scan_musicsec.tim_cnt<35){
        return;
    }
    s_scan_musicsec.tim_cnt=0;
    //debug_printf("scan music tosec in\n");
    //-------------------------------------------------------------------------------
    // ��ʼ��
    dir_tbl_t *dir_tbl = mymalloc(F_DIR_TBL_BYTE_SIZE);    
    music_tbl_t *music_tbl = mymalloc(F_MUSIC_TBL_BYTE_SIZE);    
    TCHAR *music_path = mymalloc(160);    
    //
    if(dir_tbl==NULL || music_tbl==NULL)
    {
        debug_printf("mf_scan_files malloc failed\n");
        myfree(music_tbl);
        myfree(dir_tbl);
        myfree(music_path);
        return;        
    }
    //---------------------------------------
    // ȡsdram������
    fl_read_flielist(0, (uint8_t*)dir_tbl, F_DIR_TBL_BYTE_SIZE);
    fl_read_flielist(dir_tbl->m[s_scan_musicsec.scan_dir_cnt].sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);
    //-------------------------------------------------------------------------------    
    if(music_tbl->num==0)
        goto scan_next_music;
    //
    uint8_t type=mf_typetell(music_tbl->m[s_scan_musicsec.scan_file_cnt].name);    //�������
    //
    if(type == 1)//mp3�ļ�
    {
        TCHAR g[] = {'/',0};
        wstrcpy(music_path, (const TCHAR*)dir_tbl->m[s_scan_musicsec.scan_dir_cnt].name);
        wstrcat(music_path, g);
        wstrcat(music_path, (const TCHAR*)(music_tbl->m[s_scan_musicsec.scan_file_cnt].name));
        // �������ʱ��
        music_tbl->m[s_scan_musicsec.scan_file_cnt].totsec = get_mp3_totsec((TCHAR*)music_path);       

    }
    else if(type == 2)//wav�ļ�
    {
        if(g_wav_mode){
            TCHAR g[] = {'/',0};
            wstrcpy(music_path, (const TCHAR*)dir_tbl->m[s_scan_musicsec.scan_dir_cnt].name);
            wstrcat(music_path, g);
            wstrcat(music_path, (const TCHAR*)music_tbl->m[s_scan_musicsec.scan_file_cnt].name);
            // �������ʱ��
            music_tbl->m[s_scan_musicsec.scan_file_cnt].totsec = get_wav_totsec((TCHAR*)music_path);
        }
    }
    else 
    {
        ;  //����Ҫ������
    }
    //------------------------------------------------------------------------------------------------------------
    // �Ƿ��������
    if(music_tbl->m[s_scan_musicsec.scan_file_cnt].totsec == 0){   
        //----------------------------------------------------------
        //ɾ����ǰ����������Ϣ        
        int music_index = s_scan_musicsec.scan_file_cnt;
        int move_size = music_tbl->num-music_index-1;
        if(move_size > 0)
            memmove(&music_tbl->m[music_index], &music_tbl->m[music_index+1], sizeof(music_info_t)*move_size);        
        //----------------------------------------------------------
        // �Ƿ񳬹�100�����֣�ȡ������������ֵ����λ��
        if(dir_tbl->m[s_scan_musicsec.scan_dir_cnt].music_num_full){
            if(s_scan_musicsec.need_open_dir){
                s_scan_musicsec.need_open_dir=0;
                res = f_opendir(&s_scan_musicsec.dir,(const TCHAR*)dir_tbl->m[s_scan_musicsec.scan_dir_cnt].name); //��һ��Ŀ¼
                //ָ���ļ�λ�� 
                //s_scan_musicsec.dir.index = dir_tbl->m[s_scan_musicsec.scan_dir_cnt].music_index;
                // ��ʧ��
                if(res!=FR_OK){
                    // ��������һ ������һ��ɨ��
                    music_tbl->num--;
                    s_scan_musicsec.scan_file_cnt--;        
                    dir_tbl->m[s_scan_musicsec.scan_dir_cnt].music_num = music_tbl->num;                    
                    goto scan_next_music;
                }
            }
            // Ŀ¼�򿪳ɹ�
            TCHAR *fn;   /* This function is assuming non-Unicode cfg. */
            music_info_t *pt_mi = NULL;
            while(1) //��ȡĿ¼�µ�һ���ļ�
            {
                //debug_printf("read next %d\n",s_scan_musicsec.dir.index);
                res = f_readdir(&s_scan_musicsec.dir, &s_scan_musicsec.fno);     
                //������/��ĩβ��,�˳� 
                if (res != FR_OK || s_scan_musicsec.fno.fname[0] == 0){
                    music_tbl->num--;
                    s_scan_musicsec.scan_file_cnt--;        
                    dir_tbl->m[s_scan_musicsec.scan_dir_cnt].music_num = music_tbl->num;
                    dir_tbl->m[s_scan_musicsec.scan_dir_cnt].music_num_full=0;
                    debug_printf("end file \n");
                    break; // ��������һ ������һ��ɨ��
                }
                // �����ļ�indexֵ
                //dir_tbl->m[s_scan_musicsec.scan_dir_cnt].music_index =s_scan_musicsec.dir.index;
                // ȡ���ļ���
                #if _USE_LFN
                fn = *s_scan_musicsec.fno.lfname ? s_scan_musicsec.fno.lfname : s_scan_musicsec.fno.fname;
                #else
                fn = s_scan_musicsec.fno.fname;
                #endif                                
                //
                if(s_scan_musicsec.fno.fattrib& AM_HID) continue;//���������ļ�

                //debug_printf("fno %d\n",dir->index);
                if(s_scan_musicsec.fno.fattrib & AM_ARC) //��һ���鵵�ļ�
                {
                    if(wstrlen(fn) > ((MUSIC_NAME_SIZE-2)/2)) continue;//�����ļ�������
                    //---------------------------------------------------------------------------------------
                    //�����ж�
                    uint8_t same_cnt=0;
                    for(same_cnt=0;same_cnt<F_MUSIC_MAX_NUM;same_cnt++){
                        if(wstrcmp((TCHAR*)music_tbl->m[same_cnt].name, (TCHAR*)fn) == 0)
                        {
                            break;
                        }

                    }
                    if(same_cnt!=F_MUSIC_MAX_NUM){
                        //debug_printf("cnt %d\n",same_cnt);
                        continue;
                    }
                    //----------------------------------------------------------------------------------------
                    type=mf_typetell(fn);    //�������
                    
                    if(type == 1)//mp3�ļ�
                    {
                        pt_mi = (music_info_t*)&music_tbl->m[music_tbl->num-1];
                        pt_mi->totsec = 0;
                        memset(pt_mi->name, 0, sizeof(pt_mi->name));//APP����Ҫ�ڽ�ֹ�������
                        wstrcpy(pt_mi->name, fn);
                        s_scan_musicsec.scan_file_cnt--;
                        break; //��һ�����ڴ���
                    }
                    //#if WAV_FILE_ENABLE
                    else if(type == 2)//wav�ļ�
                    {
                        pt_mi = (music_info_t*)&music_tbl->m[music_tbl->num-1];
                        pt_mi->totsec = 0;
                        memset(pt_mi->name, 0, sizeof(pt_mi->name));//APP����Ҫ�ڽ�ֹ�������
                        wstrcpy(pt_mi->name, fn);
                        
                        s_scan_musicsec.scan_file_cnt--;
                        break; //��һ�����ڴ���
                    }
                    //#endif
                    else 
                    {
                        continue;  //����Ҫ������
                    }
                }
                else 
                {
                    continue;      //��������һ��
                }
            }
        }
        else{
            music_tbl->num--;
            s_scan_musicsec.scan_file_cnt--;        
            dir_tbl->m[s_scan_musicsec.scan_dir_cnt].music_num = music_tbl->num;
        }
        //dir_tbl->m[s_scan_musicsec.scan_dir_cnt].music_num_full=0;
        // �����ļ�����Ϣ
        fl_write_flielist(0, (uint8_t*)dir_tbl, F_DIR_TBL_BYTE_SIZE);
    }
    //------------------------------------------------------------------------------------------------------------
    // ��������ʱ����Ϣ
    fl_write_flielist(dir_tbl->m[s_scan_musicsec.scan_dir_cnt].sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);
    scan_next_music:
    // ��һ�������ļ�
    s_scan_musicsec.scan_file_cnt++;
    // �Ƿ�ɨ�����������ļ� 
    if(s_scan_musicsec.scan_file_cnt>=music_tbl->num){
        // ��һ���ļ���        
        s_scan_musicsec.scan_file_cnt=0;
        s_scan_musicsec.scan_dir_cnt++;
        s_scan_musicsec.need_open_dir=1;
        // �Ƿ�ɨ�������ļ���
        if(s_scan_musicsec.scan_dir_cnt>=dir_tbl->num){
            s_scan_musicsec.scan_file_f = 0;
            s_scan_musicsec.scan_dir_cnt=0;
            file_scaning_overf = 1;
            debug_printf("\n\n\n scan music end\n\n\n");
        }
    }
    myfree(dir_tbl);
    myfree(music_tbl);    
    myfree(music_path);
    
    //debug_printf("scan music tosec out\n");
}

