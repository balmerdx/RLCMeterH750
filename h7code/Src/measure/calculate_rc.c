#include "calculate_rc.h"
#include "hardware/select_resistor.h"

void calculate(ConvolutionResult* result, complexf* Zx_result)
{
    float resistor = 100;
    float toVolts = 3.0f;
    float gain_V;
    float gain_I;

    ResistorSelectorEnum r = ResistorCurrent();
    switch(r)
    {
    case Resistor_100_Om: resistor = 1e2; break;
    case Resistor_1_KOm: resistor = 1e3; break;
    case Resistor_10_KOm: resistor = 1e4; break;
    }

    gain_V = gain_I = 5.1*3;

    complexf zV = result->sum_a_sin + result->sum_a_cos*I;
    complexf zI = result->sum_b_sin + result->sum_b_cos*I;

    zV *= toVolts/gain_V;
    zI *= toVolts/gain_I;

    complexf Zx = (zV/zI)*resistor;
    *Zx_result =Zx;
}
