#include "calculate_rc.h"
#include "hardware/select_resistor.h"

void calculate(ConvolutionResult* result, complex* Zx_result)
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

    complex zV = result->sum_a;
    complex zI = result->sum_b;

    zV *= toVolts/gain_V;
    zI *= toVolts/gain_I;

    complex Zx = (zV/zI)*resistor;
    *Zx_result =Zx;
}
