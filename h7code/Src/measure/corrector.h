#pragma once
#include <complex.h>
#include "hardware/select_resistor.h"

//Будем хранит во float что-бы меньше места занимала коррекция.
typedef complex float complexf;

//Стандартные частоты, на которых будем производить измерения корректирующих коэффициентов
#define FREQ_INDEX_MAX 58
int32_t StandartFreq(int idx);


typedef struct ZmOpen
{
    //measured load
    //Измеренное значение для всех номиналов резисторов от 100 Ом до 10 КОм
    complexf Zstdm;
    //measured open fixtures
    //Разомкнутые щупы
    complexf Zom;
} ZmOpen;

typedef struct ZmShort
{
    //measured load
    //Измеренное значение для 100 Ом (или 10 Ом) резистора.
    complexf Zstdm;
    //measured short
    //Измеренное значение для замкнутых щупов.
    complexf Zsm;
} ZmShort;

typedef struct CorrectionOneFreq
{
    int32_t freq;
    //Применяется для интервала менее 15 Ом
    ZmShort short_100_Om_Voltage_Boost;
    //Применяется для интервала менее 100 Ом
    ZmShort short_100_Om;
    //Применяется для интервала от 100 Ом до 1 КОм
    ZmOpen open_100_Om;
    //Применяется для интервала от 1 КОм до 10 КОм
    ZmOpen open_1_KOm;
    //Применяется для интервала от 10 КОм и выше
    ZmOpen open_10_KOm;
    //Применяется для интервала от 100 КОм и выше
    ZmOpen open_10_KOm_Current_Boost;
} CorrectionOneFreq;

//Размер около 3-х килобайт
//Т.е. мы с запасам укладываемся в 64 КБ блок
typedef struct CorrectionsAll
{
    uint16_t magic; //Магическое значение, которое будет меняться при смене формата
    uint16_t size; //sizeof(CorrectionAll)

    float r_10_Ohm;
    float r_100_Ohm;
    float r_1_KOhm;
    float r_10_KOhm;
    float r_100_KOhm;

    CorrectionOneFreq freq[FREQ_INDEX_MAX];
} CorrectionsAll;

extern CorrectionsAll g_corrections;

bool CorrectionValid();
//Применяем коррекцию к Zxm и возвращаем результат.
complex CorrectionMake(complex Zxm, ResistorSelectorEnum resistor, int32_t frequency);

void CorrectionSave(uint32_t index);
void CorrectionLoad(uint32_t index);
