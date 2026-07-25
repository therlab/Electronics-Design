/**
 * @file    delay.c
 * @brief   SysTick delay + grayscale serial read functions
 */

#include "delay.h"
#include "ti_msp_dl_config.h"
#include "gray_serial.h"

/* Reuse OLED module's SysTick counter (tick_ms) */
extern volatile unsigned long tick_ms;

/* Millisecond delay using SysTick counter */
void Tick_delay(uint32_t ms)
{
    uint32_t start = (uint32_t)tick_ms;
    while (((uint32_t)tick_ms - start) < ms);
}

/* Microsecond busy-wait delay (32MHz ~ 32 cycles/us) */
void delay_us(unsigned long us)
{
    volatile uint32_t count = us * 32;
    while (count--);
}

/* 8-channel grayscale serial read (CLK=PB7, DAT=PB8) */
uint8_t gray_serial_read(void)
{
    uint8_t i, ret = 0;
    DL_GPIO_clearPins(Serial_Gray_PORT, Serial_Gray_CLK_PIN);
    for (i = 0; i < 8; i++) {
        DL_GPIO_setPins(Serial_Gray_PORT, Serial_Gray_CLK_PIN);
        delay_us(5);
        DL_GPIO_clearPins(Serial_Gray_PORT, Serial_Gray_CLK_PIN);
        if (DL_GPIO_readPins(Serial_Gray_PORT, Serial_Gray_DAT_PIN)) {
            ret |= (uint8_t)(1 << i);
        }
    }
    return ret;
}
