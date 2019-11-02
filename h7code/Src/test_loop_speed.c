#include "test_loop_speed.h"

#include <math.h>

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


void speedLoop()
{
    for(int i=0; i<ARRAY_SIZE; i++)
    {
        float f = array_a[i]*array_b[i]+array_c[i];
        //if(f>10.0f || f<-10.0f)
        //    f *= 0.2f;

        array_c[i] = f;
    }
}

