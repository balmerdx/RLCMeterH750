#include "stm32h7xx_hal.h"
#include "select_resistor.h"

#define S1_PIN  GPIO_PIN_7
#define S1_PORT GPIOE

#define S2_PIN  GPIO_PIN_8
#define S2_PORT GPIOE

#define KU_V_PIN  GPIO_PIN_2
#define KU_V_PORT GPIOA

#define KU_I_PIN  GPIO_PIN_3
#define KU_I_PORT GPIOA

static ResistorSelectorEnum res;

static bool boost_amplify_I;
static bool boost_amplify_V;

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

#ifdef PCB_V3
    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin       = KU_V_PIN;
    HAL_GPIO_Init(KU_V_PORT, &gpio);

    gpio.Pin       = KU_I_PIN;
    HAL_GPIO_Init(KU_I_PORT, &gpio);

    SetBoostAmplifyI(false);
    SetBoostAmplifyV(false);
#endif//PCB_V3
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

float ResistorValue()
{
    ResistorSelectorEnum r = ResistorCurrent();
    float resistor;
    switch(r)
    {
    default:
    case Resistor_100_Om: resistor = 1e2; break;
    case Resistor_1_KOm: resistor = 1e3; break;
    case Resistor_10_KOm: resistor = 1e4; break;
    }

    return resistor;
}

float GetAmplifyI()
{
#ifdef PCB_V3
    if(boost_amplify_I)
        return 5.9*11;
    return 5.9*2;
#else
    return 5.1*3;
#endif
}

float GetAmplifyV()
{
#ifdef PCB_V3
    if(boost_amplify_V)
        return 5.9*11;
    return 5.9*2;
#else
    return 5.1*3;
#endif
}

bool BoostAmplifyI()
{
    return boost_amplify_I;
}

bool BoostAmplifyV()
{
    return boost_amplify_V;
}

void SetBoostAmplifyI(bool boost)
{
    boost_amplify_I = boost;
    HAL_GPIO_WritePin(KU_I_PORT, KU_I_PIN, boost);
}

void SetBoostAmplifyV(bool boost)
{
    boost_amplify_V = boost;
    HAL_GPIO_WritePin(KU_V_PORT, KU_V_PIN, boost);
}

float ADC_MaxVoltage()
{
#ifdef PCB_V3
    return 2.5;
#else
    return 3.3;
#endif
}
