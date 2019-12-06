#pragma once

#include "delay.h"

#define SPI_SCK_PIN GPIO_PIN_3
#define SPI_SCK_PORT GPIOB
#define SPI_SCK_AF GPIO_AF0_SPI1

#define SPI_MISO_PIN GPIO_PIN_4
#define SPI_MISO_PORT GPIOB
#define SPI_MISO_AF GPIO_AF0_SPI1

#define SPI_MOSI_PIN GPIO_PIN_5
#define SPI_MOSI_PORT GPIOB
#define SPI_MOSI_AF GPIO_AF0_SPI1

#define SPI_NAME SPI1

#define SPI_GPIO_CLOCK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPI_CLOCK_ENABLE() __HAL_RCC_SPI1_CLK_ENABLE()
/*
#define SPIx SPI1
#define SPIx_SCK_PIN GPIO_PIN_5
#define SPIx_MISO_PIN GPIO_PIN_6
#define SPIx_MOSI_PIN GPIO_PIN_7
#define SPIx_AF GPIO_AF0_SPI1
#define SPIx_GPIO_PORT GPIOA
*/
#define PIN_CS GPIO_PIN_6
#define PORT_CS GPIOB
/*
#define CS_ONE  SpiWaitNotBusy(); DelayUs(10); PORT_CS->BSRR = PIN_CS;//set cs pin to one
#define CS_ZERO PORT_CS->BRR = PIN_CS; //set cs pin to zero

#define xmit_spi(data) rw_spi(data)
#define recv_spi() rw_spi(0)
*/
#define CS_ONE
#define CS_ZERO

#define xmit_spi(data)
#define recv_spi() 0

static void init_spi()
{
}
