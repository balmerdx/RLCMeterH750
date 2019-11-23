#include "stm32h7xx_hal.h"
#include "data_processing.h"

#include <string.h>

uint32_t adc_cpu_buffer[ADC_BUFFER_SIZE];
static bool start_buffer_filling = false;
static bool complete_buffer_filling = false;
static uint32_t buffer_pos = 0;

#define ADC1_DATA(x)   ((x) & 0x0000FFFF)
#define ADC2_DATA(x)   ((x) >> 16)

void AdcConvertDataCallback(uint32_t* data, uint32_t size)
{
    if(start_buffer_filling)
    {
        uint32_t count = ADC_BUFFER_SIZE-buffer_pos;
        if(count > size)
            count = size;
        memcpy(adc_cpu_buffer+buffer_pos, data, count*sizeof(uint32_t));

        buffer_pos += count;
        if(buffer_pos >= ADC_BUFFER_SIZE)
        {
            start_buffer_filling = false;
            complete_buffer_filling = true;
        }
    }
}


#undef ADC1_DATA
#undef ADC2_DATA

void AdcStartBufferFilling()
{
    while(start_buffer_filling)
        HAL_Delay(1);

    buffer_pos = 0;
    complete_buffer_filling = false;
    start_buffer_filling = true;
}

bool AdcBufferFillingComplete()
{
    return complete_buffer_filling;
}
