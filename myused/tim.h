/**
 * @file    tim.h
 * @brief   定时器模块 — 100ms周期性定时器
 *
 * TIMA1: 100ms间隔, 用于:
 *   - 编码器速度更新周期
 *   - 其他周期性任务
 */

#ifndef _tim_h
#define _tim_h

#include "ti_msp_dl_config.h"
#include "stdio.h"

/**
 * @brief  初始化定时器中断
 * @note   使能 TIMA1 ZERO 事件中断, 周期=100ms
 */
void TIMER_0_init(void);

#endif /* _tim_h */
