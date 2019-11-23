#include "stm32h7xx_hal.h"
#include "test_loop_speed.h"

#include <math.h>
#include "measure/sin_cos.h"
/*
#define ARRAY_SIZE 4096
float array_a[ARRAY_SIZE];
float array_b[ARRAY_SIZE];
float array_c[ARRAY_SIZE];


void initLoop()
{
    for(int i=0; i<ARRAY_SIZE; i++)
    {
        array_a[i] = cos((float)i/ARRAY_SIZE*1.23456f);
        array_b[i] = sinf((float)i/ARRAY_SIZE);
        array_c[i] = sinf((float)i/ARRAY_SIZE*2.09876f);
    }
}

void speedLoopMulAdd()
{
    for(int i=0; i<ARRAY_SIZE; i++)
    {
        float f = array_a[i]*array_b[i]+array_c[i];
        //if(f>10.0f || f<-10.0f)
        //    f *= 0.2f;

        array_c[i] = f;
    }
}


void speedLoopSinCos()
{
    for(int i=0; i<ARRAY_SIZE; i++)
    {
        float f = sinf(array_a[i])*cosf(array_b[i])+array_c[i];
        array_c[i] = f;
    }
}
*/

#define ARRAY_SIZE 4096
uint32_t array_ab[ARRAY_SIZE];

volatile int64_t g_sum = 0;

void initLoop()
{
    SinCosInit();
    for(int i=0; i<ARRAY_SIZE; i++)
    {
        uint32_t a = cosf((float)i/ARRAY_SIZE*1.23456f)*1024+1024;
        uint32_t b = sinf((float)i/ARRAY_SIZE)*1024+1024;
        array_ab[i] = a | (b<<16);
    }
}

void speedLoopSinCos()
{
    uint32_t delta_phase = FloatToSinPhase(0.001f);
    uint32_t phase = 0;
    int64_t sum_a_sin = 0;
    int64_t sum_a_cos = 0;
    int64_t sum_b_sin = 0;
    int64_t sum_b_cos = 0;

    SCB_InvalidateDCache_by_Addr((uint32_t *) &array_ab[0], 4*ARRAY_SIZE);

    for(int i=0; i<ARRAY_SIZE; i++)
    {
        int32_t ret_sin;
        int32_t ret_cos;
        SinCosInt(phase, &ret_sin, &ret_cos);

        uint32_t ab = array_ab[i];
        uint32_t a = ab &0xFFFF;
        uint32_t b = ab >> 16;
        sum_a_sin += (ret_sin*(int64_t)(a))>>16;
        sum_a_cos += (ret_cos*(int64_t)(a))>>16;
        sum_b_sin += (ret_sin*(int64_t)(b))>>16;
        sum_b_cos += (ret_cos*(int64_t)(b))>>16;

        phase += delta_phase;
    }

    g_sum = sum_a_sin + sum_a_cos + sum_b_sin + sum_b_cos;

}
