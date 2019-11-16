#include "stm32h7xx_hal.h"
#include "hw_ili9341.h"

void Error_Handler(void);

#define TIMEOUT_MS 10

#define SPI_SCK_PIN GPIO_PIN_13
#define SPI_SCK_PORT GPIOB

#define SPI_MISO_PIN GPIO_PIN_14
#define SPI_MISO_PORT GPIOB

#define SPI_MOSI_PIN GPIO_PIN_15
#define SPI_MOSI_PORT GPIOB

#define TFT_SPI SPI2
#define SPI_AF GPIO_AF5_SPI2

#define SPI_CLOCK_ENABLE() __HAL_RCC_SPI2_CLK_ENABLE()

#define TFT_PORT_RST	GPIOD
#define PIN_RST		GPIO_PIN_8

#define TFT_PORT_DC	GPIOB
#define PIN_DC		GPIO_PIN_12

#define TFT_PORT_CS	GPIOD
#define PIN_CS		GPIO_PIN_9

#define GPIO_CLOCK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE(); __HAL_RCC_GPIOD_CLK_ENABLE()

static SPI_HandleTypeDef SpiHandle;

#define BUFFER_SIZE 128
static uint8_t spi_tx_buffer[BUFFER_SIZE];
static int spi_tx_buffer_count; //количество байтов в spi_tx_buffer


void HwLcdInit()
{
    GPIO_CLOCK_ENABLE();
    SPI_CLOCK_ENABLE();
    GPIO_InitTypeDef  gpio = {};
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = SPI_AF;

    gpio.Pin       = SPI_SCK_PIN;
    HAL_GPIO_Init(SPI_SCK_PORT, &gpio);

    gpio.Pin       = SPI_MOSI_PIN;
    HAL_GPIO_Init(SPI_MOSI_PORT, &gpio);

    gpio.Pin       = SPI_MISO_PIN;
    gpio.Pull      = GPIO_PULLUP;
    HAL_GPIO_Init(SPI_MISO_PORT, &gpio);

    gpio.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio.Alternate = 0;
    gpio.Pull      = GPIO_NOPULL;

    gpio.Pin       = PIN_RST;
    HAL_GPIO_Init(TFT_PORT_RST, &gpio);

    gpio.Pin       = PIN_DC;
    HAL_GPIO_Init(TFT_PORT_DC, &gpio);

    gpio.Pin       = PIN_CS;
    HAL_GPIO_Init(TFT_PORT_CS, &gpio);

    SpiHandle.Instance               = TFT_SPI;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;
    SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
    SpiHandle.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;  /* Recommanded setting to avoid glitches */
    SpiHandle.Init.Mode              = SPI_MODE_MASTER;

    if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
    {
        Error_Handler();
    }

    spi_tx_buffer_count = 0;
}

void DelaySome()
{
    HAL_Delay(5);
}

void HwLcdPinCE(uint8_t on)
{
    HAL_GPIO_WritePin(TFT_PORT_CS, PIN_CS, on);
}

void HwLcdPinDC(uint8_t on)
{
    HAL_GPIO_WritePin(TFT_PORT_DC, PIN_DC, on);
}

void HwLcdPinRst(uint8_t on)
{
    HAL_GPIO_WritePin(TFT_PORT_RST, PIN_RST, on);
}

void HwLcdPinLed(uint8_t on)
{
}

void HwLcdSend(uint8_t data)
{
    if(HAL_SPI_Transmit(&SpiHandle, &data, 1, TIMEOUT_MS)!=HAL_OK)
    {
        Error_Handler();
    }
}

uint8_t HwLcdSendReceive(uint8_t data)
{
    uint8_t rx_data;
    if(HAL_SPI_TransmitReceive(&SpiHandle, &data, &rx_data, 1, TIMEOUT_MS)!=HAL_OK)
    {
        Error_Handler();
    }

    return rx_data;
}

void HwLcdSend16NoWait(uint16_t data)
{
/*
    HwLcdSend(data>>8);
    HwLcdSend(data);
*/
    if(spi_tx_buffer_count>=BUFFER_SIZE-1)
        HwLcdWait();

    spi_tx_buffer[spi_tx_buffer_count] = data>>8;
    spi_tx_buffer[spi_tx_buffer_count+1] = data;
    spi_tx_buffer_count += 2;
}

void HwLcdWait()
{
    if(spi_tx_buffer_count>0)
    if(HAL_SPI_Transmit(&SpiHandle, spi_tx_buffer, spi_tx_buffer_count, TIMEOUT_MS)!=HAL_OK)
    {
        Error_Handler();
    }

    spi_tx_buffer_count = 0;
}
