#pragma once
#include "main.h"
#include "measure/calculate_rc.h"
void SceneSingleFreqStart();
void SceneSingleFreqZx(complex Zx, ErrorZx* err);

typedef enum
{
    VM_LC,
    VM_Z_REAL_IMAG,
    VM_Z_ABS_ARG,
} VIEW_MODE;

extern bool view_parallel;
extern VIEW_MODE view_mode;
extern bool view_debug;
