#pragma once

#include <complex.h>
#include <stdbool.h>
#include "data_processing.h"

//Преобразует данные от ADC в Zxm
//Zxm - измеренное (но не откорректированное) комплексное сопротивление.
//После этого его надо откорректировать используя correctionMake
complex calculateZxm(ConvolutionResult* result);

typedef struct VisualInfo
{
    bool parallel;
    //parallel == true, Zx представлен в виде паралельно соединенных резисторов.
    //parallel == false, Zx представлен в виде последовательно соединенных резисторов.
    //один из резисторов чисто вещественный, другой чисто комплексный.
    //Вещественная часть - это либо ESR (жквивалентное последовательное сопротивление)
    // либо EPR - эквивалентное параллеьное сопротивление.
    float Rre;
    float Rim;
    //Если is_inductance == true, то значит у нас индуктивное сопротивление комплексное.
    //Если is_inductance == false, то значит у нас комплексное сопротивление конденсатор.
    bool is_inductance;
    float L;
    float C;
} VisualInfo;

void convertZxmToVisualInfo(complex Zx, double F, bool parallel, VisualInfo* info);
