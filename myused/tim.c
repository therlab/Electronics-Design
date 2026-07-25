/**
 * @file    tim.c
 * @brief   100ms 定时器中断处理
 *
 * 中断中执行:
 *   1. encoder_update_speed() — 编码器速度计算
 *   2. (预留) 其他周期性任务
 */

#include "tim.h"
#include "../BSP/ENCODER/encoder.h"

/**
 * @brief  初始化 100ms 定时器
 */
void TIMER_0_init(void)
{
    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
}

/**
 * @brief  TIMA1 100ms 定时器中断服务
 *
 * 触发源: TIMA1 ZERO 事件 (period=3999, clock=40kHz → 100ms)
 */
void TIMER_0_INST_IRQHandler(void)
{
    switch (DL_TimerA_getPendingInterrupt(TIMER_0_INST)) {
    case DL_TIMER_IIDX_ZERO:
        /* 清除中断标志 */
        DL_TimerA_clearInterruptStatus(TIMER_0_INST, DL_TIMER_IIDX_ZERO);

        /* 编码器速度更新 (每100ms执行一次) */
        encoder_update_speed();

        break;

    default:
        break;
    }
}
