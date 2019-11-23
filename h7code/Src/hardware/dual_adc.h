#pragma once

//88/4=22 MHz adc clock. 11 clock per conversion = 2.5 (sampling time) + 8.5 (16 bit conversion time)
//2 MSps

void DualAdcInitAndStart();
void AdcConvertDataCallback(uint32_t* data, uint32_t size);

//Количество сэмплов в секунду
uint32_t AdcSamplesPerSecond();

extern volatile int half_conv;
extern volatile int full_conv;
extern volatile int adc_overrun;

#define ADCCONVERTEDVALUES_BUFFER_SIZE ((uint32_t)  2048)    /* Size of array containing ADC converted values */

// User can use this section to tailor ADCx instance under use and associated
//   resources

// ## Definition of ADC related resources ###################################
// Definition of ADCx clock resources
#define ADCx                            ADC1
#define ADCx_CLK_ENABLE()               __HAL_RCC_ADC12_CLK_ENABLE()

#define ADCx_FORCE_RESET()              __HAL_RCC_ADC12_FORCE_RESET()
#define ADCx_RELEASE_RESET()            __HAL_RCC_ADC12_RELEASE_RESET()

// Definition of ADCx channels
#define ADCx_CHANNELa                   ADC_CHANNEL_3

// Definition of ADCx channels pins
#define ADCx_CHANNELa_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define ADCx_CHANNELa_GPIO_PORT         GPIOA
#define ADCx_CHANNELa_PIN               GPIO_PIN_6

// Definition of ADCx DMA resources
#define ADCx_DMA_CLK_ENABLE()           __HAL_RCC_DMA1_CLK_ENABLE()
#define ADCx_DMA                        DMA1_Stream1

#define ADCx_DMA_IRQn                   DMA1_Stream1_IRQn
#define ADCx_DMA_IRQHandler             DMA1_Stream1_IRQHandler

// Definition of ADCx NVIC resources
#define ADCx_IRQn                       ADC_IRQn
#define ADCx_IRQHandler                 ADC_IRQHandler


// Definition of ADCy clock resources
#define ADCy                            ADC2
#define ADCy_CLK_ENABLE()               __HAL_RCC_ADC12_CLK_ENABLE()

#define ADCy_FORCE_RESET()              __HAL_RCC_ADC12_FORCE_RESET()
#define ADCy_RELEASE_RESET()            __HAL_RCC_ADC12_RELEASE_RESET()

// Definition of ADCy channels
#define ADCy_CHANNELa                   ADC_CHANNEL_5

// Definition of ADCy channels pins
#define ADCy_CHANNELa_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define ADCy_CHANNELa_GPIO_PORT         GPIOB
#define ADCy_CHANNELa_PIN               GPIO_PIN_1

// Definition of ADCy NVIC resources
#define ADCy_IRQn                       ADC_IRQn
#define ADCy_IRQHandler                 ADC_IRQHandler
