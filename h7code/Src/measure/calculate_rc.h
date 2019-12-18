#pragma once

#include <complex.h>
#include <stdbool.h>
#include "data_processing.h"

typedef struct ErrorZx
{
    float err_R;

    //is_big - если значение считается большим и тяготеет к бесконечности.
    //Иначе значение малое и тяготеет к нулю.
    bool is_big;
    bool is_inf;
} ErrorZx;

//Преобразует данные от ADC в Zxm
//Zxm - измеренное (но не откорректированное) комплексное сопротивление.
//После этого его надо откорректировать используя correctionMake
//error_real, error_imag - относительная ошибка результатов ДО калибровки
//После калибровки относительная ошибка может сногократно вырасти
//Хотя при этом абсолютная ошибка останется такойже.
//err_R - абсолютное значение (в омах) нашей ошибки.
complex calculateZxm(ConvolutionResult* result,
                     ErrorZx* err);

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

    bool is_inf;
} VisualInfo;

void convertZxmToVisualInfo(complex Zx, double F, bool parallel, ErrorZx error, VisualInfo* info);
