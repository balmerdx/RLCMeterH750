#include "stm32h7xx_hal.h"
#include "delay.h"

static TIM_HandleTypeDef    tim_init;

void DelayInit()
{
    __HAL_RCC_TIM15_CLK_ENABLE();
    tim_init.Instance = TIM15;
    tim_init.Init.Period = 0xFFFF;
    tim_init.Init.Prescaler = (HAL_RCC_GetPCLK1Freq()*2)/1000000-1; //1 us per tick
    tim_init.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim_init.Init.ClockDivision = 0;
    tim_init.Init.RepetitionCounter = 0;

    if (HAL_TIM_Base_Init(&tim_init) != HAL_OK)
        while(1);

    if (HAL_TIM_Base_Start(&tim_init) != HAL_OK)
        while(1);
}

void DelayDeinit()
{
    HAL_TIM_Base_DeInit(&tim_init);
    __HAL_RCC_TIM15_CLK_DISABLE();
}

void DelayUs(uint16_t countUs)
{
    uint16_t start = (uint16_t)TIM15->CNT;
    while(((uint16_t)(TIM15->CNT-start)) < countUs)
    {

    }
}

void DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

uint32_t TimeMs()
{
    return HAL_GetTick();
}
