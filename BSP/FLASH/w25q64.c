/**
 * @file    w25q64.c
 * @brief   W25Q64 SPI Flash 软件模拟驱动
 */

#include "w25q64.h"
#include "uart.h"
#include "delay.h"

/* ══════ 引脚控制宏 ══════ */
#define CS_LOW()   DL_GPIO_clearPins(W25_CS_PORT, W25_CS_PIN)
#define CS_HIGH()  DL_GPIO_setPins(W25_CS_PORT, W25_CS_PIN)
#define SCK_LOW()  DL_GPIO_clearPins(W25_SCK_PORT, W25_SCK_PIN)
#define SCK_HIGH() DL_GPIO_setPins(W25_SCK_PORT, W25_SCK_PIN)
#define MOSI_LOW() DL_GPIO_clearPins(W25_MOSI_PORT, W25_MOSI_PIN)
#define MOSI_HIGH()DL_GPIO_setPins(W25_MOSI_PORT, W25_MOSI_PIN)
#define MISO_READ()(DL_GPIO_readPins(W25_MISO_PORT, W25_MISO_PIN) ? 1 : 0)

/* ══════ W25Q64 指令 ══════ */
#define CMD_WREN  0x06
#define CMD_WRDI  0x04
#define CMD_RDSR  0x05
#define CMD_READ  0x03
#define CMD_PP    0x02
#define CMD_SE    0x20

void w25q64_init(void)
{
    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    delay_ms(1);

    DL_GPIO_initDigitalOutput(IOMUX_PINCM4);   /* PA3 = CS */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM5);   /* PA4 = SCK */
    DL_GPIO_initDigitalOutput(IOMUX_PINCM7);   /* PA6 = MOSI */
    DL_GPIO_initDigitalInput(IOMUX_PINCM42);   /* PB1 = MISO */

    CS_HIGH();
    SCK_LOW();
}

static void spi_write_byte(uint8_t b)
{
    for (int i = 7; i >= 0; i--) {
        if (b & (1 << i)) MOSI_HIGH(); else MOSI_LOW();
        SCK_HIGH();
        delay_us(1);
        SCK_LOW();
        delay_us(1);
    }
}

static uint8_t spi_read_byte(void)
{
    uint8_t b = 0;
    for (int i = 7; i >= 0; i--) {
        SCK_HIGH();
        delay_us(1);
        if (MISO_READ()) b |= (1 << i);
        SCK_LOW();
        delay_us(1);
    }
    return b;
}

static uint8_t w25_wait(void)
{
    uint8_t sr;
    do {
        CS_LOW();
        spi_write_byte(CMD_RDSR);
        sr = spi_read_byte();
        CS_HIGH();
    } while (sr & 0x01);
    return sr;
}

void w25q64_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    CS_LOW();
    spi_write_byte(CMD_READ);
    spi_write_byte((addr >> 16) & 0xFF);
    spi_write_byte((addr >> 8) & 0xFF);
    spi_write_byte(addr & 0xFF);
    for (uint32_t i = 0; i < len; i++)
        buf[i] = spi_read_byte();
    CS_HIGH();
}

void w25q64_erase_sector(uint32_t addr)
{
    CS_LOW();
    spi_write_byte(CMD_WREN);
    CS_HIGH();

    CS_LOW();
    spi_write_byte(CMD_SE);
    spi_write_byte((addr >> 16) & 0xFF);
    spi_write_byte((addr >> 8) & 0xFF);
    spi_write_byte(addr & 0xFF);
    CS_HIGH();

    w25_wait();
}

void w25q64_write(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    while (len > 0) {
        CS_LOW();
        spi_write_byte(CMD_WREN);
        CS_HIGH();

        uint32_t chunk = len > 256 ? 256 : len;
        CS_LOW();
        spi_write_byte(CMD_PP);
        spi_write_byte((addr >> 16) & 0xFF);
        spi_write_byte((addr >> 8) & 0xFF);
        spi_write_byte(addr & 0xFF);
        for (uint32_t i = 0; i < chunk; i++)
            spi_write_byte(buf[i]);
        CS_HIGH();

        w25_wait();
        buf  += chunk;
        addr += chunk;
        len  -= chunk;
    }
}
