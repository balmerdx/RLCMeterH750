#pragma once

typedef enum
{
    //Resistor_Auto нельзя передавать в ResistorSelect
    Resistor_Auto = 0,

    Resistor_100_Om = 1,
    Resistor_1_KOm = 2,
    Resistor_10_KOm = 3,
} ResistorSelectorEnum;

//Заодно эта функция инициалищирует и другие вещи
void ResistorSelectorInit();

ResistorSelectorEnum ResistorCurrent();

void ResistorSelect(ResistorSelectorEnum r);

//Текущее сопротивление резистора в омах
float ResistorValue();

//Коэффициенты усиления в I/V каскадах
float GetAmplifyI();
float GetAmplifyV();


bool BoostAmplifyI();
bool BoostAmplifyV();

void SetBoostAmplifyI(bool boost);
void SetBoostAmplifyV(bool boost);

float ADC_MaxVoltage();
