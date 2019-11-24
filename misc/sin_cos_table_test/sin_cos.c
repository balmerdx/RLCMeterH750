#include "sin_cos.h"
#include <math.h>
#include <assert.h>
#include <limits.h>

#define SIN_TABLE_SHIFT 9
#define SIN_TABLE_SIZE (1<<SIN_TABLE_SHIFT)
#define PHASE_FRACTIONAL_SHIFT (SIN_PHASE_FRACTIONAL_SHIFT-SIN_TABLE_SHIFT)
static int32_t sin_table[SIN_TABLE_SIZE+1];

/* Попробуем избавиться от int64_t вычислений в lerp
 * Ошибка 1.8835e-05 при использовании lerp.
 * У нас есть 512 значений, что теоретически обеспечивает более двух знаков после запятой.
 * Т.е. на frac часть достаточно 10 бит, возмем 12 для запаса.
 * sinf((2*M_PI)/SIN_TABLE_SIZE) = 0.01227 - максимальная разность a-b
 * Т.е. 36 бит SIN_FRACTIONAL_SHIFT может быть максимальным чтобы разность влезла.
 * 36 - 12 = 24 бита. Проверяем. Да, все укладывается в int32_t и точность
 * уменьшилась до 1.895428e-05. Т.е. в пределах прогрешности.
*/

void SinCosInit()
{
    for(int i=0; i<=SIN_TABLE_SIZE; i++)
        sin_table[i] = lround(sinf((2*M_PI*i)/SIN_TABLE_SIZE)*(1<<SIN_FRACTIONAL_SHIFT));
}

// Линейная интерполяция между a и b.
// Если бы frac изменялся от 0 до 1, то былобы return a+(b-a)*frac
// Но frac изменяеотся от 0 до (1<<PHASE_FRACTIONAL_SHIFT)
static int32_t lerp(int32_t a, int32_t b, int32_t frac)
{
    int64_t diff = ((int64_t)(b-a)*frac);
    assert(diff>=INT_MIN && diff<=INT_MAX);
    return a + ((b-a)*frac>>PHASE_FRACTIONAL_SHIFT);
}

void SinCosInt(uint32_t phase, int32_t* ret_sin, int32_t* ret_cos)
{
    int32_t sin_index = (phase >> PHASE_FRACTIONAL_SHIFT) & (SIN_TABLE_SIZE-1);
    int32_t cos_index = (sin_index+(1<<(SIN_TABLE_SHIFT-2))) & (SIN_TABLE_SIZE-1);
    int32_t frac = phase & ((1<<PHASE_FRACTIONAL_SHIFT)-1);
    *ret_sin = lerp(sin_table[sin_index], sin_table[sin_index+1], frac);
    *ret_cos = lerp(sin_table[cos_index], sin_table[cos_index+1], frac);
}

uint32_t FloatToSinPhase(float phase)
{
    phase = phase/(2*M_PI);

    if(phase<0)
        phase = fmodf(phase, 1);
    if(phase<0)
        phase += 1;

    return lround(phase * (1<<SIN_PHASE_FRACTIONAL_SHIFT));
}

float SinValueToFloat(int32_t val)
{
    return val/(float)(1<<SIN_FRACTIONAL_SHIFT);
}
