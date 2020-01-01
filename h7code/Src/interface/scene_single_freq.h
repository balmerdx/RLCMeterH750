#pragma once
#include "main.h"
#include "measure/calculate_rc.h"
void SceneSingleFreqStart();
void SceneSingleFreqZx();

typedef enum
{
    VM_LC,
    VM_Z_REAL_IMAG,
    VM_Z_ABS_ARG,
} VIEW_MODE;

extern bool view_parallel;
extern VIEW_MODE view_mode;
extern bool view_debug;

void FormatReIm(complex Zx,
                ErrorZx* error,
                double freq,
                char* str_re,
                char* str_im,
                char* str_re_type,
                char* str_im_type);
