#include "stm32h7xx_hal.h"
#include "select_resistor.h"

#define S1_PIN  GPIO_PIN_7
#define S1_PORT GPIOE

#define S2_PIN  GPIO_PIN_8
#define S2_PORT GPIOE

static ResistorSelectorEnum res;

void ResistorSelectorInit()
{
    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitTypeDef  gpio = {};

    gpio.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio.Alternate = 0;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;

    gpio.Pin       = S1_PIN;
    HAL_GPIO_Init(S1_PORT, &gpio);

    gpio.Pin       = S2_PIN;
    HAL_GPIO_Init(S2_PORT, &gpio);

    ResistorSelect(Resistor_100_Om);
}

ResistorSelectorEnum ResistorCurrent()
{
    return res;
}

void ResistorSelect(ResistorSelectorEnum r)
{
    GPIO_PinState s1, s2;
    switch(r)
    {
    default:
        r = Resistor_100_Om;
    case Resistor_100_Om:
        s1 = 1;
        s2 = 0;
        break;
    case Resistor_1_KOm:
        s1 = 0;
        s2 = 1;
        break;
    case Resistor_10_KOm:
        s1 = 1;
        s2 = 1;
        break;
    }
    HAL_GPIO_WritePin(S1_PORT, S1_PIN, s1);
    HAL_GPIO_WritePin(S2_PORT, S2_PIN, s2);
    res = r;
}

