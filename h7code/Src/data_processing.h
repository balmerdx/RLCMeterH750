#pragma once

#define ADC_BUFFER_SIZE 10000
extern uint32_t adc_cpu_buffer[ADC_BUFFER_SIZE];

//После того как AdcBufferFillingComplete()==true
//В массиве adc_cpu_buffer будут отсэмплированные данные
void AdcStartBufferFilling();
bool AdcBufferFillingComplete();


void AdcStartMeasureFreq();
void AdcStopMeasureFreq();
float AdcMeasureFreq();
