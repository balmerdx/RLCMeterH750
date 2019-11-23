#pragma once

void AdcStartBufferFilling();
bool AdcBufferFillingComplete();

#define ADC_BUFFER_SIZE 10000
extern uint32_t adc_cpu_buffer[ADC_BUFFER_SIZE];
