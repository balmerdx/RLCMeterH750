#include "stm32h7xx_hal.h"
#include "main.h"
#include "data_processing.h"
#include "measure/measure_freq.h"
#include "measure/sin_cos.h"
#include "hardware/AD9833_driver.h"
#include "hardware/dual_adc.h"

#include <string.h>

#define PHASE_ADDITIONAL_SHIFT 20
#define CONVOLUTION_SIN_DIV_SHIFT 16

uint32_t adc_cpu_buffer[ADC_BUFFER_SIZE];
static bool start_buffer_filling = false;
static bool complete_buffer_filling = false;
static uint32_t buffer_pos = 0;

static bool start_measure_freq = false;
static bool stop_measure_freq = false;
static float measure_freq = 0;
MeasureFreqData measure_data;

static uint64_t phase_convolution;
static uint64_t delta_phase_convolution;
static volatile bool start_convolution = false;
static volatile bool stop_convolution = false;
static volatile bool complete_covolution = false;
static volatile uint32_t g_mid_samples, g_mid_count;
static volatile uint32_t g_convolution_samples, g_convolution_count;
static volatile uint64_t g_mid_a;
static volatile uint64_t g_mid_b;
static volatile int64_t g_sum_a_sin;
static volatile int64_t g_sum_a_cos;
static volatile int64_t g_sum_b_sin;
static volatile int64_t g_sum_b_cos;

void MakeConvolution(uint32_t* data, uint32_t size);

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

    if(start_convolution)
    {
        MakeConvolution(data, size);

        bool complete = (g_mid_count==0 && g_convolution_count==0);
        if(stop_convolution || complete)
        {
            start_convolution = false;
            stop_convolution = false;
            complete_covolution = complete;
        }
    }
}

void MakeConvolution(uint32_t* data, uint32_t size)
{
    if(g_mid_count>0)
    {
        if(size > g_mid_count)
            size = g_mid_count;

        uint64_t mid_a = 0;
        uint64_t mid_b = 0;
        for(uint32_t i=0; i<size; i++)
        {
            uint32_t ab = data[i];
            mid_a += ADC1_DATA(ab);
            mid_b += ADC2_DATA(ab);
        }

        g_mid_a += mid_a;
        g_mid_b += mid_b;
        g_mid_count -= size;

        if(g_mid_count==0)
        {
            g_mid_a = g_mid_a/g_mid_samples;
            g_mid_b = g_mid_b/g_mid_samples;
        }

        return;
    }

    if(g_convolution_count>0)
    {
        if(size > g_convolution_count)
            size = g_convolution_count;

        uint64_t phase = phase_convolution;
        uint64_t delta_phase = delta_phase_convolution;
        int64_t sum_a_sin = 0;
        int64_t sum_a_cos = 0;
        int64_t sum_b_sin = 0;
        int64_t sum_b_cos = 0;

        int32_t mid_a = (int32_t)g_mid_a;
        int32_t mid_b = (int32_t)g_mid_b;

        for(uint32_t i=0; i<size; i++)
        {
            int32_t ret_sin;
            int32_t ret_cos;
            SinCosInt(phase>>PHASE_ADDITIONAL_SHIFT, &ret_sin, &ret_cos);

            uint32_t ab = data[i];
            int64_t a = ((int32_t)ADC1_DATA(ab))-mid_a;
            int64_t b = ((int32_t)ADC2_DATA(ab))-mid_b;
            sum_a_sin += (ret_sin*a)>>CONVOLUTION_SIN_DIV_SHIFT;
            sum_a_cos += (ret_cos*a)>>CONVOLUTION_SIN_DIV_SHIFT;
            sum_b_sin += (ret_sin*b)>>CONVOLUTION_SIN_DIV_SHIFT;
            sum_b_cos += (ret_cos*b)>>CONVOLUTION_SIN_DIV_SHIFT;

            phase += delta_phase;
        }

        g_convolution_count -= size;

        phase_convolution = phase;
        g_sum_a_sin += sum_a_sin;
        g_sum_a_cos += sum_a_cos;
        g_sum_b_sin += sum_b_sin;
        g_sum_b_cos += sum_b_cos;
    }
}

static void AdcStopAll()
{
    if(start_convolution)
    {
        stop_convolution = true;
        while(start_convolution)
            HAL_Delay(1);
    }

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

void AdcClearBufferFillingComplete()
{
    complete_buffer_filling = false;
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

void AdcStartConvolution(uint32_t freq_word, uint32_t time_ms)
{
    AdcStopAll();

    double freq = AD9833_CalcFreq(freq_word);
    if(freq<1)//Error! Frequency too low!
        return;

    uint64_t sps = AdcSamplesPerSecond();

    uint32_t required_samples = (AdcSamplesPerSecond()*time_ms)/1000;
    double samples_per_cycle = sps/freq;
    uint32_t mid_sample_divider = 10;

    g_convolution_count = g_convolution_samples = (1+(uint32_t)(required_samples/samples_per_cycle))*samples_per_cycle;
    g_mid_count = g_mid_samples = (1+(uint32_t)(required_samples/mid_sample_divider/samples_per_cycle))*samples_per_cycle;

    g_mid_a = 0;
    g_mid_b = 0;
    g_sum_a_sin = 0;
    g_sum_a_cos = 0;
    g_sum_b_sin = 0;
    g_sum_b_cos = 0;

    phase_convolution = 0;
    delta_phase_convolution = (1ull<<(SIN_PHASE_FRACTIONAL_SHIFT+PHASE_ADDITIONAL_SHIFT))/samples_per_cycle;

    complete_covolution = false;
    start_convolution = true;
}

bool AdcConvolutionComplete()
{
    return complete_covolution;
}

ConvolutionResult AdcConvolutionResult()
{
    double mul = 1./(1<<(SIN_FRACTIONAL_SHIFT - CONVOLUTION_SIN_DIV_SHIFT));
    ConvolutionResult r;
    r.mid_a = g_mid_a;
    r.mid_b = g_mid_b;
    r.sum_samples = g_convolution_samples;

    double sum_a_sin = (mul*g_sum_a_sin)/r.sum_samples;
    double sum_a_cos = (mul*g_sum_a_cos)/r.sum_samples;
    double sum_b_sin = (mul*g_sum_b_sin)/r.sum_samples;
    double sum_b_cos = (mul*g_sum_b_cos)/r.sum_samples;

    r.sum_a = sum_a_sin + sum_a_cos * I;
    r.sum_b = sum_b_sin + sum_b_cos * I;
    return r;
}
