#include "calculate_rc.h"
#include "hardware/select_resistor.h"

complex calculateZxm(ConvolutionResult* result)
{
    float resistor = 100;
    float toVolts = 3.0f;
    float gain_V;
    float gain_I;

    ResistorSelectorEnum r = ResistorCurrent();
    switch(r)
    {
    default:
    case Resistor_100_Om: resistor = 1e2; break;
    case Resistor_1_KOm: resistor = 1e3; break;
    case Resistor_10_KOm: resistor = 1e4; break;
    }

    gain_V = gain_I = 5.1*3;

    complex zV = result->sum_a;
    complex zI = result->sum_b;

    zV *= toVolts/gain_V;
    zI *= toVolts/gain_I;

    complex Zxm = (zV/zI)*resistor;
    return Zxm;
}

void convertZxmToVisualInfo(complex Zx, double F, bool parallel, VisualInfo* info)
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
        if(cabs(Zx)<1 && info->L<0 && info->L>-1e-9)
            info->is_inductance = true;
    }
}
