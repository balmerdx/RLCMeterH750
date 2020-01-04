#include "main.h"
#include "corrector.h"
#include "hardware/store_to_spi_flash.h"

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


bool CorrectionValid()
{
    return g_corrections.magic == MAGIC_DATA && g_corrections.size == sizeof(CorrectionsAll);
}

static int CalibrationBisectF(int32_t freq)
{
    static int last_index = 0;
    if(last_index >= FREQ_INDEX_MAX)
        last_index = 0;
    if(g_corrections.freq[last_index].freq >= freq)
        last_index = 0;


    int ifound = 0;
    for(int i=last_index; i<FREQ_INDEX_MAX; i++)
    {
        if(g_corrections.freq[i].freq < freq)
        {
            ifound = i;
        } else
        {
            break;
        }
    }

    last_index = ifound;
    return ifound;
}


static ZmShort lerp_ZmShort(ZmShort* p1, ZmShort* p2, float a)
{
    ZmShort out;
    out.Zstdm = p1->Zstdm*(1-a)+p2->Zstdm*a;
    out.Zsm = p1->Zsm*(1-a)+p2->Zsm*a;
    return out;
}

static ZmOpen lerp_ZmOpen(ZmOpen* p1, ZmOpen* p2, float a)
{
    ZmOpen out;
    out.Zstdm = p1->Zstdm*(1-a)+p2->Zstdm*a;
    out.Zom = p1->Zom*(1-a)+p2->Zom*a;
    return out;
}

CorrectionOneFreq CalibrationInterpolate(int32_t freq)
{
    //Предполагаем, что все калибровочные данные валидные!
    CorrectionOneFreq data;
    memset(&data, 0, sizeof(data));

    int idx = CalibrationBisectF(freq);
    if(idx+1 == FREQ_INDEX_MAX)
    {
        data = g_corrections.freq[FREQ_INDEX_MAX-1];
        data.freq = freq;
        return data;
    }

    if(idx==0 && freq<g_corrections.freq[0].freq)
    {
        data = g_corrections.freq[0];
        data.freq = freq;
        return data;
    }

    CorrectionOneFreq* p0 = g_corrections.freq + idx;
    CorrectionOneFreq* p1 = g_corrections.freq + idx + 1;

    float a = (freq - p0->freq)/(float)(p1->freq - p0->freq);

    data.short_100_Om = lerp_ZmShort(&p0->short_100_Om, &p1->short_100_Om, a);
    data.open_100_Om = lerp_ZmOpen(&p0->open_100_Om, &p1->open_100_Om, a);
    data.open_1_KOm = lerp_ZmOpen(&p0->open_1_KOm, &p1->open_1_KOm, a);
    data.open_10_KOm = lerp_ZmOpen(&p0->open_10_KOm, &p1->open_10_KOm, a);

    return data;
}

complex CorrectionMake(complex Zxm, ResistorSelectorEnum resistor, int32_t frequency)
{
    if(!CorrectionValid())
        return Zxm;

    CorrectionOneFreq cf = CalibrationInterpolate(frequency);

    float Rxm = cabs(Zxm);
    if(resistor==Resistor_100_Om)
    {
        if(Rxm<100)
            return CorrectorShort(Zxm, 100, &cf.short_100_Om);
        return CorrectorOpen(Zxm, 100, &cf.open_100_Om);
    }

    if(resistor==Resistor_1_KOm)
    {
        return CorrectorOpen(Zxm, 1000, &cf.open_1_KOm);
    }

    if(resistor==Resistor_10_KOm)
    {
        return CorrectorOpen(Zxm, 10000, &cf.open_10_KOm);
    }

    return Zxm;
}

void CorrectionSave(uint32_t index)
{
    g_corrections.magic = MAGIC_DATA;
    g_corrections.size = sizeof(CorrectionsAll);
    SpiFlashWriteToFlash(FLASH_SECTOR_CORRECTION+index, sizeof(CorrectionsAll), &g_corrections);
}


void CorrectionLoad(uint32_t index)
{
    g_corrections.magic = 0;
    g_corrections.size = 0;
    SpiFlashReadFromFlash(FLASH_SECTOR_CORRECTION+index, sizeof(CorrectionsAll), &g_corrections);
}
