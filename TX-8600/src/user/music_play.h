#ifndef _MUSIC_PLAY_H_
#define _MUSIC_PLAY_H_

#include <stdint.h>
#include "sys_config_dat.h"

void task_music_send(uint8_t ch,taskmac_info_t *p_taskmac_info,uint8_t div_tol,uint8_t task_prio,uint8_t task_vol);

void task_music_stop(uint8_t ch);

void task_music_play(uint8_t ch,uint8_t num,task_music_info_t *p_music_info);

// ��ʱ�������

// ���ֲ���
void rttask_music_play(uint16_t id);
// ����ֹͣ
void rttask_music_stop(uint16_t id);
// ��һ��
void rttask_music_last(uint16_t id);
// ��һ��
void rttask_music_next(uint16_t id);
// ����ѡ��
void rttask_music_select(uint16_t id,uint8_t mus_inc);
// ѡ������ʱ��
void rttask_music_totimer(uint16_t id,uint16_t music_sec);
// ѡ������ģʽ
void rttask_music_setmode(uint16_t id,uint8_t mode);
// ������������
void rttask_music_setvol(uint16_t id,uint8_t vol);

uint16_t get_music_tolsec(task_music_info_t *p_music_info);


#endif

