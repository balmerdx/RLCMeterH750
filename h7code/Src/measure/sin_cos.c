#include "sin_cos.h"
#include <math.h>

#define SIN_TABLE_SHIFT 9
#define SIN_TABLE_SIZE (1<<SIN_TABLE_SHIFT)
#define SIN_FRACTIONAL_SHIFT 30
#define PHASE_FRACTIONAL_SHIFT 20
static int32_t sin_table[SIN_TABLE_SIZE+1];

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
    //return a + (((int64_t)(b-a)*frac+(1<<(PHASE_FRACTIONAL_SHIFT-1)))>>PHASE_FRACTIONAL_SHIFT);
    return a + (((int64_t)(b-a)*frac)>>PHASE_FRACTIONAL_SHIFT);
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

    return lround(phase * (1<<(PHASE_FRACTIONAL_SHIFT+SIN_TABLE_SHIFT)));
}

float SinValueToFloat(int32_t val)
{
    return val/(float)(1<<SIN_FRACTIONAL_SHIFT);
}
