#include <stdio.h>
#include <math.h>
#include "sin_cos.h"

/*
int main()
{
    SinCosInit();
    float err_max = 0;
    for(double phase=-4*M_PI; phase<4*M_PI; phase+=0.0001f)
    //for(double phase=0; phase<2*M_PI; phase+=0.0001f)
    {
        float s = sinf(phase);
        float c = cosf(phase);

        uint32_t phase_int = FloatToSinPhase(phase);
        int32_t ret_sin, ret_cos;
        SinCosInt(phase_int, &ret_sin, &ret_cos);
        float s1 = SinValueToFloat(ret_sin);
        float c1 = SinValueToFloat(ret_cos);

        float ds = fabsf(s-s1);
        float dc = fabsf(c-c1);
        float err = ds>dc?ds:dc;
        if(err > err_max)
        {
            err_max = err;
            printf("phase = %e err=%e\n", phase, err);
        }
    }

    printf("\nerr_max=%e\n", err_max);

    double freq = 100;
    uint64_t sps = 2000000;
    double samples_per_cycle = sps/freq;

#define ADDITIONAL_SHIFT 20

    uint64_t full_cycle = (1<<SIN_PHASE_FRACTIONAL_SHIFT);
    uint64_t delta_phase_convolution = (1ull<<(SIN_PHASE_FRACTIONAL_SHIFT+ADDITIONAL_SHIFT))/samples_per_cycle;
    uint64_t phase = 0;
    for(int i=0; i<samples_per_cycle; i++)
    {
        phase += delta_phase_convolution;
    }

    phase = phase>>ADDITIONAL_SHIFT;

    return 0;
}

*/


//Предполагаем, что самое большое, что может встретиться -9999
//Самое маленькое, что может встретиться -0.01
void PlotCalcTicks(float xmin, float xmax,
                   int* ptick_xmin, int* ptick_xmax, int* ptick_dx, float* ptick_mul, int* float_places)
{
    *ptick_xmin = 0;
    *ptick_xmax = 10;
    *ptick_dx = 1;
    *ptick_mul = 1;
    *float_places = 0;

    float dx = xmax-xmin;
    if(dx<0)
        dx = -dx;

    float ftick = 1;

    float c = 3.f;
    for(int i=-2; i<8; i++)
    {
        ftick = 1;
        if(i<0)
        {
            for(int j=0; j<-i; j++)
                ftick *= 0.1f;
            *float_places = -i;
        } else
        {
            for(int j=0; j<i; j++)
                ftick *= 10.f;
            *float_places = 0;
        }

        //if(dx>=c*ftick && dx<c*10*ftick)
        if(dx<c*10*ftick)
            break;
    }

    uint32_t count = lround(dx/ftick);

    *ptick_dx = 1;
    if(count>=c*5)
        *ptick_dx = 5;
    else
    if(count>=c*2)
        *ptick_dx = 2;

    *ptick_xmin = floor(xmin/(ftick* *ptick_dx))* *ptick_dx;
    *ptick_xmax = ceil(xmax/(ftick* *ptick_dx))* *ptick_dx;
    *ptick_mul = ftick;
}


int main()
{
    float xmin = 10;
    float xmax = 100;
    int tick_xmin = 0;
    int tick_xmax = 10;
    int tick_delta = 2;
    float tick_mul = 0.1f;
    int float_places = 1;

    PlotCalcTicks(xmin, xmax, &tick_xmin, &tick_xmax,
                  &tick_delta, &tick_mul, &float_places);

    float t_mul = tick_mul;
    for(int t=tick_xmin+tick_delta; t<=tick_xmax-tick_delta; t+=tick_delta)
    {
        printf("%f\n", t*t_mul);
    }
}
