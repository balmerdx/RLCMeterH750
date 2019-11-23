#include "stm32h7xx_hal.h"
#include "main.h"
#include "data_processing.h"
#include "measure/measure_freq.h"

#include <string.h>

uint32_t adc_cpu_buffer[ADC_BUFFER_SIZE];
static bool start_buffer_filling = false;
static bool complete_buffer_filling = false;
static uint32_t buffer_pos = 0;

static bool start_measure_freq = false;
static bool stop_measure_freq = false;
static float measure_freq = 0;
MeasureFreqData measure_data;

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

    if(start_measure_freq)
    {
        for(uint32_t i=0; i<size; i++)
            MeasureFrequencyQuant(&measure_data, ADC1_DATA(data[i]));

        measure_freq = MeasureFrequencyValue(&measure_data);

        if(stop_measure_freq)
        {
            stop_measure_freq = false;
            start_measure_freq = false;
        }
    }
}


#undef ADC1_DATA
#undef ADC2_DATA

static void AdcStopAll()
{
    if(start_measure_freq)
    {
        stop_measure_freq = true;
        while(start_measure_freq)
            HAL_Delay(1);
    }

    while(start_buffer_filling)
        HAL_Delay(1);

}

void AdcStartBufferFilling()
{
    AdcStopAll();

    buffer_pos = 0;
    complete_buffer_filling = false;
    start_buffer_filling = true;
}

bool AdcBufferFillingComplete()
{
    return complete_buffer_filling;
}

void AdcStartMeasureFreq()
{
    AdcStopAll();

    //Предполагаем, что частота выше 20 Гц
    MeasureFrequencyStart(&measure_data, AdcSamplesPerSecond()/20, AdcSamplesPerSecond());
    start_measure_freq = true;
}

void AdcStopMeasureFreq()
{
    AdcStopAll();
}

float AdcMeasureFreq()
{
    return measure_freq;
}
