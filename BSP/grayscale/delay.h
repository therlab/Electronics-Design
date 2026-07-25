/**
 * @file    delay.h
 * @brief   ����/΢����ʱ��������
 *
 * ��ģ�鸴�� OLED ģ��� SysTick ������ (tick_ms),
 * ���������� SysTick_Handler, �������ӳ�ͻ
 */

#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ������ʱ (��æ�ȴ�, ���� SysTick) */
void Tick_delay(uint32_t ms);

/* ΢����ʱ (æ�ȴ�, CPU��ѭ��, 32MHz �� 32����/us) */
void delay_us(unsigned long us);

#ifdef __cplusplus
}
#endif

#endif /* DELAY_H */
