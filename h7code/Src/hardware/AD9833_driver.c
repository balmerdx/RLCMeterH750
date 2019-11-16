#include "stm32h7xx_hal.h"
#include "AD9833_driver.h"

void Error_Handler(void);

#define COMMAND_CONTROL (0<<14)
#define COMMAND_FREQ0 (1<<14)

#define CONTROL_B28 (1<<13)
#define CONTROL_RESET (1<<8)

//Hardware defines
#define SPI_SCK_PIN GPIO_PIN_3
#define SPI_SCK_PORT GPIOB

#define SPI_MOSI_PIN GPIO_PIN_5
#define SPI_MOSI_PORT GPIOB

#define AD_PORT_FSYNC	GPIOB
#define PIN_FSYNC		GPIO_PIN_4

#define GPIO_CLOCK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

static void DelaySome()
{
    volatile int counter = 0;
    //for(counter = 0; counter<10; counter++);
    for(counter = 0; counter<3; counter++);
}

static void AD9833_HardwareInit()
{
    GPIO_CLOCK_ENABLE();
    GPIO_InitTypeDef  gpio = {};

    gpio.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio.Alternate = 0;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;

    gpio.Pin       = SPI_SCK_PIN;
    HAL_GPIO_Init(SPI_SCK_PORT, &gpio);

    gpio.Pin       = SPI_MOSI_PIN;
    HAL_GPIO_Init(SPI_MOSI_PORT, &gpio);


    gpio.Pin       = PIN_FSYNC;
    HAL_GPIO_Init(AD_PORT_FSYNC, &gpio);

    HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 0);
    HAL_GPIO_WritePin(AD_PORT_FSYNC, PIN_FSYNC, 1);
}

static void AD9833_SPISend16(uint16_t data)
{
    HAL_GPIO_WritePin(AD_PORT_FSYNC, PIN_FSYNC, 0);
    HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 1);

    for(uint8_t i=0; i<16; i++)
    {
        DelaySome();
        HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 1);
        HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, (data & 0x8000)? 1:0);
        data = data<<1;

        DelaySome();
        HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 0);
    }

    DelaySome();
    HAL_GPIO_WritePin(AD_PORT_FSYNC, PIN_FSYNC, 1);
    HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 1);
    HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, 0);
}

void AD9833_Init()
{
    AD9833_HardwareInit();
    AD9833_SPISend16(COMMAND_CONTROL|CONTROL_RESET);
    AD9833_SPISend16(COMMAND_CONTROL|CONTROL_B28);
}

void AD9833_SetFreqWorld(uint32_t freq_reg)
{
    uint16_t msb = freq_reg>>14;
    uint16_t lsb = freq_reg&0x3FFF;//14 bit
    AD9833_SPISend16(COMMAND_FREQ0|lsb);
    AD9833_SPISend16(COMMAND_FREQ0|msb);
}

uint32_t AD9833_CalcFreqWorld(float freq)
{
    //freq = AD9833_MASTER_CLOCK*freq_reg/(1<<28)
    //freq_reg = (freq*(1<<28))/AD9833_MASTER_CLOCK
    if(freq<0)
        freq = 0;

    uint32_t freq_reg = (freq*(1<<28)+AD9833_MASTER_CLOCK/2)/AD9833_MASTER_CLOCK;

    if(freq_reg>=(1<<28))
        freq_reg = (1<<28)-1;

    return freq_reg;
}
