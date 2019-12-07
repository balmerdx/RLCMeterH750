#pragma once
#include <complex.h>

typedef enum SingleFreqViewMode
{
    VM_Z,
    VM_RX_RLC_SERIAL,
    VM_RX_RLC_PARALLEL,
    VM_COUNT
} SingleFreqViewMode;

void SceneSingleFreqStart();
void SceneSingleFreqZx(complex Zx);
