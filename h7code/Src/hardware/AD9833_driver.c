#include "stm32h7xx_hal.h"
#include "AD9833_driver.h"

void Error_Handler(void);

#define COMMAND_CONTROL (0<<14)
#define COMMAND_FREQ0 (1<<14)

#define CONTROL_B28 (1<<13)
#define CONTROL_RESET (1<<8)

//Hardware defines
#define TIMEOUT_MS 10

#define SPI_SCK_PIN GPIO_PIN_3
#define SPI_SCK_PORT GPIOB

#define SPI_MOSI_PIN GPIO_PIN_5
#define SPI_MOSI_PORT GPIOB

#define AD_SPI SPI1
#define SPI_AF GPIO_AF5_SPI1

#define AD_PORT_FSYNC	GPIOB
#define PIN_FSYNC		GPIO_PIN_4

#define GPIO_CLOCK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()


//static SPI_HandleTypeDef SpiHandle;

static void DelaySome()
{
    volatile int counter = 0;
    for(counter = 0; counter<10; counter++);
}

static void AD9833_HardwareInit()
{
    GPIO_CLOCK_ENABLE();
    GPIO_InitTypeDef  gpio = {};
/*
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = SPI_AF;

    gpio.Pin       = SPI_SCK_PIN;
    HAL_GPIO_Init(SPI_SCK_PORT, &gpio);

    gpio.Pin       = SPI_MOSI_PIN;
    HAL_GPIO_Init(SPI_MOSI_PORT, &gpio);

    gpio.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio.Alternate = 0;
    gpio.Pull      = GPIO_NOPULL;

    gpio.Pin       = PIN_FSYNC;
    HAL_GPIO_Init(AD_PORT_FSYNC, &gpio);
    HAL_GPIO_WritePin(AD_PORT_FSYNC, PIN_FSYNC, 1);

    SpiHandle.Instance               = AD_SPI;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES_TXONLY;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;//SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;//SPI_POLARITY_LOW;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_16BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_16BIT;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;
    SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
    SpiHandle.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;  // Recommanded setting to avoid glitches
    SpiHandle.Init.Mode              = SPI_MODE_MASTER;

    if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
    {
        Error_Handler();
    }
*/

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
/*
static void AD9833_Fsync(uint8_t val)
{
    HAL_GPIO_WritePin(AD_PORT_FSYNC, PIN_FSYNC, val);
}


static void AD9833_SPISend16(uint16_t data)
{
    AD9833_Fsync(0);
    DelaySome();
    if(HAL_SPI_Transmit(&SpiHandle, (uint8_t*)&data, 2, TIMEOUT_MS)!=HAL_OK)
    {
        Error_Handler();
    }
    AD9833_Fsync(1);
}
*/

/*
 * Bit1 передается как Bit2 этой функцией
static void AD9833_SPISend16(uint16_t data)
{
    HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 1);
    DelaySome();
    HAL_GPIO_WritePin(AD_PORT_FSYNC, PIN_FSYNC, 0);
    DelaySome();
    HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 0);

    for(uint8_t i=0; i<16; i++)
    {
        DelaySome();
        HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 1);

        HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, (data & 0x8000)? 1:0);
        data = data<<1;

        DelaySome();
        HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 0);
    }

    HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 1);
    DelaySome();
    HAL_GPIO_WritePin(AD_PORT_FSYNC, PIN_FSYNC, 1);
    HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, 0);
}
*/
static void AD9833_SPISend16(uint16_t data)
{
    HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 1);
    DelaySome();
    HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, (data & 0x8000)? 1:0);
    data = data<<1;
    HAL_GPIO_WritePin(AD_PORT_FSYNC, PIN_FSYNC, 0);
    DelaySome();
    HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 0);

    for(uint8_t i=0; i<15; i++)
    {
        HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, (data & 0x8000)? 1:0);
        data = data<<1;

        DelaySome();
        HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 1);

        DelaySome();
        HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 0);
    }

    HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, 1);
    DelaySome();
    HAL_GPIO_WritePin(AD_PORT_FSYNC, PIN_FSYNC, 1);
    HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, 0);
}

void AD9833_Init()
{
    AD9833_HardwareInit();
    AD9833_SPISend16(COMMAND_CONTROL|CONTROL_RESET);
    HAL_Delay(1); //Теоретически даже этого не требуется
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
