#pragma once

#include "delay.h"

#define SPI_TIMEOUT 10

#define SPIx                            SPI3
#define SPIx_CLK_ENABLE()               __HAL_RCC_SPI3_CLK_ENABLE()
#define SPIx_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOA_CLK_ENABLE();__HAL_RCC_GPIOC_CLK_ENABLE()

#define SPIx_FORCE_RESET()              __HAL_RCC_SPI3_FORCE_RESET()
#define SPIx_RELEASE_RESET()            __HAL_RCC_SPI3_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                     GPIO_PIN_10
#define SPIx_SCK_GPIO_PORT               GPIOC
#define SPIx_SCK_AF                      GPIO_AF6_SPI3

#define SPIx_MISO_PIN                    GPIO_PIN_11
#define SPIx_MISO_GPIO_PORT              GPIOC
#define SPIx_MISO_AF                     GPIO_AF6_SPI3

#define SPIx_MOSI_PIN                    GPIO_PIN_12
#define SPIx_MOSI_AF                     GPIO_AF6_SPI3
#define SPIx_MOSI_GPIO_PORT              GPIOC

#define SPIx_CS_PIN                      GPIO_PIN_15
#define SPIx_CS_GPIO_PORT                GPIOA

/*
#define CS_ONE  SpiWaitNotBusy(); DelayUs(10); PORT_CS->BSRR = PIN_CS;//set cs pin to one
#define CS_ZERO PORT_CS->BRR = PIN_CS; //set cs pin to zero

#define xmit_spi(data) rw_spi(data)
#define recv_spi() rw_spi(0)
*/

SPI_HandleTypeDef SpiHandle = {};

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef  gpio;

    if(hspi->Instance == SPIx)
    {
        SPIx_GPIO_CLK_ENABLE();
        SPIx_CLK_ENABLE();

        gpio.Mode      = GPIO_MODE_OUTPUT_PP;
        gpio.Alternate = 0;
        gpio.Pull      = GPIO_NOPULL;
        gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
        gpio.Pin       = SPIx_CS_PIN;
        HAL_GPIO_Init(SPIx_CS_GPIO_PORT, &gpio);
        HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);

        gpio.Pin       = SPIx_SCK_PIN;
        gpio.Mode      = GPIO_MODE_AF_PP;
        gpio.Pull      = GPIO_PULLDOWN;
        gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
        gpio.Alternate = SPIx_SCK_AF;
        HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &gpio);

        gpio.Pin = SPIx_MISO_PIN;
        gpio.Alternate = SPIx_MISO_AF;
        HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &gpio);

        gpio.Pin = SPIx_MOSI_PIN;
        gpio.Alternate = SPIx_MOSI_AF;
        HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &gpio);
    }
}

/**
  * @brief SPI MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO configuration to its default state
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if(hspi->Instance == SPIx)
    {
        SPIx_FORCE_RESET();
        SPIx_RELEASE_RESET();

        HAL_GPIO_DeInit(SPIx_CS_GPIO_PORT, SPIx_CS_PIN);
        HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
        HAL_GPIO_DeInit(SPIx_MISO_GPIO_PORT, SPIx_MISO_PIN);
        HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);
    }
}

static void init_spi()
{
    SpiHandle.Instance               = SPIx;
    SpiHandle.Init.Mode              = SPI_MODE_MASTER;
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    SpiHandle.Init.CRCPolynomial     = 0;
    SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
    SpiHandle.Init.NSSPolarity       = SPI_NSS_POLARITY_LOW;
    SpiHandle.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_01DATA;

    SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
    SpiHandle.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;  // Recommanded setting to avoid glitches


    if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
    {
        Error_Handler();
    }
}

#define CS_ONE HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);

#define CS_ZERO HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);

static void xmit_spi_buffer(uint8_t* data, uint16_t size)
{
    HAL_SPI_Transmit(&SpiHandle, data, size, SPI_TIMEOUT);
}

static void recv_spi_buffer(uint8_t* data, uint16_t size)
{
    HAL_SPI_Receive(&SpiHandle, data, size, SPI_TIMEOUT);
}

static void xmit_spi(uint8_t data)
{
    HAL_SPI_Transmit(&SpiHandle, &data, 1, SPI_TIMEOUT);
}

static uint8_t recv_spi()
{
    uint8_t data_rx = 123;
    HAL_SPI_Receive(&SpiHandle, &data_rx, 1, SPI_TIMEOUT);
    return data_rx;
}
