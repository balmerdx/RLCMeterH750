#include "stm32h7xx_hal.h"
#include "quadrature_encoder.h"

void Error_Handler(void);

//-------------------------------------------#DEFINE------------------------------------------
//Encoder PB6, PB7 TIM4
#define Codeur1_A           GPIO_PIN_6
#define Codeur1_B           GPIO_PIN_7
#define Codeur1_GPIO        GPIOB
#define Codeur1_AF          GPIO_AF2_TIM4
#define Codeur1_TIMER       TIM4

#define BUTTON_PIN1         GPIO_PIN_4
#define BUTTON_GPIO			GPIOD

static TIM_HandleTypeDef    tim_init;
static TIM_Encoder_InitTypeDef encoder;

void QuadEncInit()
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();


    tim_init.Instance = Codeur1_TIMER;
    tim_init.Init.Period = 0xffff;
    tim_init.Init.Prescaler = 0;
    tim_init.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim_init.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim_init.Init.RepetitionCounter = 0;

    GPIO_InitTypeDef gpio = {};

    {//Codeur1
        // 2 Inputs for A and B Encoder Channels
        gpio.Pin = Codeur1_A|Codeur1_B;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Speed = GPIO_SPEED_HIGH;
        gpio.Pull = GPIO_NOPULL;
        gpio.Alternate = Codeur1_AF;
        HAL_GPIO_Init(Codeur1_GPIO, &gpio);
     
        encoder.EncoderMode = TIM_ENCODERMODE_TI12;

        encoder.IC1Filter = 0x0F;
        encoder.IC1Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
        encoder.IC1Prescaler = TIM_ICPSC_DIV4;
        encoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;

        encoder.IC2Filter = 0x0F;
        encoder.IC2Polarity = TIM_INPUTCHANNELPOLARITY_FALLING;
        encoder.IC2Prescaler = TIM_ICPSC_DIV4;
        encoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;

        if (HAL_TIM_Encoder_Init(&tim_init, &encoder) != HAL_OK) {
            Error_Handler();
        }

        if(HAL_TIM_Encoder_Start(&tim_init, TIM_CHANNEL_ALL)!=HAL_OK){
            Error_Handler();
        }

    }

    //button
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Speed = GPIO_SPEED_LOW;
    gpio.Pull = GPIO_NOPULL;
    gpio.Pin = BUTTON_PIN1;
    HAL_GPIO_Init(BUTTON_GPIO, &gpio);
}

int16_t QuadEncValue()
{
    return (int16_t)Codeur1_TIMER->CNT;
}

bool QuadEncButton()
{
    return HAL_GPIO_ReadPin(BUTTON_GPIO, BUTTON_PIN1)==0;
}
