/**
 * @file    w25q64.h
 * @brief   W25Q64 8MB SPI Flash 驱动
 *
 * 接线: CS=PA3 SCK=PA4 MOSI=PA6 MISO=PA7
 */

#ifndef W25Q64_H
#define W25Q64_H

#include "ti_msp_dl_config.h"
#include <stdint.h>

/* ══════ 硬件引脚 ══════ */
#define W25_CS_PORT   GPIOA
#define W25_CS_PIN    DL_GPIO_PIN_3
#define W25_SCK_PORT  GPIOA
#define W25_SCK_PIN   DL_GPIO_PIN_4
#define W25_MOSI_PORT GPIOA
#define W25_MOSI_PIN  DL_GPIO_PIN_6
#define W25_MISO_PORT GPIOB
#define W25_MISO_PIN  DL_GPIO_PIN_1   /* PB1 */

/* ══════ API ══════ */
void w25q64_init(void);
void w25q64_read(uint32_t addr, uint8_t *buf, uint32_t len);
void w25q64_write(uint32_t addr, const uint8_t *buf, uint32_t len);
void w25q64_erase_sector(uint32_t addr);

#endif
