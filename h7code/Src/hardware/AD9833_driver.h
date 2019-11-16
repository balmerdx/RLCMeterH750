#pragma once

//25 MHz master clock
#define AD9833_MASTER_CLOCK 25000000

void AD9833_Init();

//freq = AD9833_MASTER_CLOCK*freq_reg/(1<<28)
void AD9833_SetFreqWorld(uint32_t freq_reg);

//freq - частота (герц)
uint32_t AD9833_CalcFreqWorld(float freq);
