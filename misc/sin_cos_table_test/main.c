#include <stdio.h>
#include <math.h>
#include "sin_cos.h"

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
