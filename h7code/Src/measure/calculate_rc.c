#include "calculate_rc.h"
#include "hardware/select_resistor.h"

#include <math.h>

//Минимальная число, при котором мы считаем, что точность недостаточна.
const float K_sum_min = 1;
const float K_max_error = 0.1;

complex calculateZxm(ConvolutionResult* result,
                     ErrorZx* err)
{
    float resistor = ResistorValue();
    float toVolts = ADC_MaxVoltage();
    float gain_V = GetAmplifyI();
    float gain_I = GetAmplifyV();

    complex zV = result->sum_a;
    complex zI = result->sum_b;

    float min_zI = 1.f;
    //Чтобы деления на 0 не было
    if(cabs(zI)<min_zI)
        zI = min_zI;

    zV *= toVolts/gain_V;
    zI *= toVolts/gain_I;

    complex Zxm = (zV/zI)*resistor;

    //Расчитываем абсолютные ошибки.
    float sum_a = cabs(result->sum_a);
    float sum_b = cabs(result->sum_b);
    float Rabs = cabs(Zxm);
    float sum_min = sum_a;
    err->is_big = false;
    if(sum_b < sum_min)
    {
        err->is_big = true;
        sum_min = sum_b;
    }

    float err_relative = K_sum_min/sum_min;
    err->err_R = Rabs*err_relative;
    err->err_Y = 1/Rabs*err_relative;

    return Zxm;
}

void convertZxmToVisualInfo(complex Zx, double F, bool parallel, ErrorZx* error, VisualInfo* info)
{
    double W = 2*M_PI*F;

    info->parallel = parallel;

    if(parallel)
    {
        complex Yx = 1/Zx;
        info->Rre = 1.0f/creal(Yx);
        info->Rim = 1.0f/cimag(Yx);
        info->C = cimag(Yx)/W;
        if(cimag(Yx)<-1e-10)
        {
            info->is_inductance = true;
            info->L = -1/(W*cimag(Yx));
        } else
        {
            info->is_inductance = false;
            info->L = 0;
        }
    } else
    {
        info->Rre = creal(Zx);
        info->Rim = cimag(Zx);
        info->L = cimag(Zx)/W;
        if(cimag(Zx)<-1e-10f)
        {
            info->is_inductance = false;
            info->C = -1/(W*cimag(Zx));
        } else
        {
            info->is_inductance = true;
            info->C = 0;
        }

        //если сопротивление маленькое и индуктивность немного отрицательная, то таки считаем что это ошибка калибрации
        //if(cabs(Zx)<1 && info->L<0 && info->L>-1e-9)
        //    info->is_inductance = true;
    }

    info->is_inf = isIninity(Zx, error);
}

bool isIninity(complex Zx, ErrorZx* err)
{
    bool is_inf = false;
    if(err->is_big)
    {
        float Y = 1/cabs(Zx);
        is_inf =  Y < err->err_Y;
    }

    return is_inf;
}
