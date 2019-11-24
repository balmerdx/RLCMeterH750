#pragma once

#define ADC_BUFFER_SIZE 10000
extern uint32_t adc_cpu_buffer[ADC_BUFFER_SIZE];

typedef struct ConvolutionResult
{
    uint16_t mid_a;
    uint16_t mid_b;
    uint32_t sum_samples;
    double sum_a_sin;
    double sum_a_cos;
    double sum_b_sin;
    double sum_b_cos;
} ConvolutionResult;

//После того как AdcBufferFillingComplete()==true
//В массиве adc_cpu_buffer будут отсэмплированные данные
void AdcStartBufferFilling();
bool AdcBufferFillingComplete();


void AdcStartMeasureFreq();
void AdcStopMeasureFreq();
float AdcMeasureFreq();

//freq_word - частота, с которой работает генератор AD9833
//time_ms - предполагаемое время , которое будет происходить сэмплирование.
// это время округляется в большую сторону для того, чтобы целое количество тактов сигнала захватить
// к этому времени добавляется начальное время на расчет среднего значения сигнала.
void AdcStartConvolution(uint32_t freq_word, uint32_t time_ms);

bool AdcConvolutionComplete();
ConvolutionResult AdcConvolutionResult();
