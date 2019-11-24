#pragma once
#include <stdint.h>

//Сколько бит после запятой в результатах SinCosInt
#define SIN_FRACTIONAL_SHIFT 24
#define SIN_PHASE_FRACTIONAL_SHIFT 21

void SinCosInit();
void SinCosInt(uint32_t phase, int32_t* ret_sin, int32_t* ret_cos);

//На входе - фаза в радианах.
//На выходе - фаза принимаемая на вход SinCosQ31
//Мы используем 20+9 бит дробной части, но переполнение
//не будет приводить к ошибкам в определении синуса.
uint32_t FloatToSinPhase(float phase);

float SinValueToFloat(int32_t val);
