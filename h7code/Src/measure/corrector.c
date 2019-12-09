#include "main.h"
#include "corrector.h"
#include "hardware/m25p16.h"

//Число для CorrectionsAll::magic
#define MAGIC_DATA 0x3740

CorrectionsAll g_corrections;

int32_t StandartFreq(int idx)
{
//Сначала должны идти min_idx с меньшими значениями, а потом с большими.
#define R(min_idx, mul_idx) if(idx<(min_idx)+10) return (idx+1-(min_idx))*(mul_idx)
    if(idx<0)
        return 0;
    R(0, 10);
    R(9, 100);
    R(18, 1000);
    R(27, 10000);
    R(36, 100000);
#undef R
    return idx*1000;
}

static complex CorrectorShort(complex Zxm, complex Zstd, ZmShort* c)
{
    complex Zx = Zstd/(c->Zstdm-c->Zsm)*(Zxm-c->Zsm);
    return Zx;
}

complex CorrectorOpen(complex Zxm, complex Zstd, ZmOpen* c)
{
    complex Zx = Zstd*(1/c->Zstdm-1/c->Zom)*Zxm/(1-Zxm/c->Zom);
    return Zx;
}


bool correctionValid()
{
    return g_corrections.magic == MAGIC_DATA && g_corrections.size == sizeof(CorrectionsAll);
}

static int iabs(int a)
{
    return (a<0)?-a:a;
}

complex correctionMake(complex Zxm, ResistorSelectorEnum resistor, int32_t frequency)
{
    if(!correctionValid())
        return Zxm;

    CorrectionOneFreq* cf = g_corrections.freq+0;
    int min_delta = iabs(cf->freq - frequency);
    for(int i=0; i<FREQ_INDEX_MAX; i++)
    {
        CorrectionOneFreq* c_cur = g_corrections.freq+i;
        int cur_delta = iabs(c_cur->freq - frequency);
        if(cur_delta < min_delta)
        {
            min_delta = cur_delta;
            cf = c_cur;
        }
    }

    float Rxm = cabs(Zxm);
    if(resistor==Resistor_100_Om)
    {
        if(Rxm<100)
            return CorrectorShort(Zxm, 100, &cf->short_100_Om);
        return CorrectorOpen(Zxm, 100, &cf->open_100_Om);
    }

    if(resistor==Resistor_1_KOm)
    {
        return CorrectorOpen(Zxm, 1000, &cf->open_1_KOm);
    }

    if(resistor==Resistor_10_KOm)
    {
        return CorrectorOpen(Zxm, 10000, &cf->open_10_KOm);
    }

    return Zxm;
}

void correctionSave(uint32_t offset)
{
    g_corrections.magic = MAGIC_DATA;
    g_corrections.size = sizeof(CorrectionsAll);
    m25p16_write(offset, sizeof(g_corrections), &g_corrections);
}


void correctionLoad(uint32_t offset)
{
    m25p16_read(offset, sizeof(g_corrections), &g_corrections);
}
