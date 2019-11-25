#pragma once

#include <complex.h>
#include "data_processing.h"

typedef complex float complexf;

void calculate(ConvolutionResult* result, complexf *Zx_result);
