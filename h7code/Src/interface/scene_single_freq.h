#pragma once
#include "main.h"
#include "measure/calculate_rc.h"
void SceneSingleFreqStart();
void SceneSingleFreqZx();

extern bool view_debug;

void FormatReIm(complex Zx,
                ErrorZx* error,
                double freq,
                char* str_re,
                char* str_im,
                char* str_re_type,
                char* str_im_type);
