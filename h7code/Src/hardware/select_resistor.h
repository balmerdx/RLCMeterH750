#pragma once

typedef enum
{
    //Resistor_Auto нельзя передавать в ResistorSelect
    Resistor_Auto = 0,

    //Очень низкие сопротивления измеряем
    //с повышенным коэффициентом усиления по напряжению
    Resistor_100_Om_Voltage_Boost,

    Resistor_100_Om,
    Resistor_1_KOm,
    Resistor_10_KOm,

    //Очень высокие сопротивления измеряем
    //с повышенным коэффициентом усиления по току
    Resistor_10_KOm_Current_Boost,

    Resistor_Last = Resistor_10_KOm_Current_Boost,
} ResistorSelectorEnum;

//Заодно эта функция инициалищирует и другие вещи
void ResistorSelectorInit();

ResistorSelectorEnum ResistorCurrent();

void ResistorSelect(ResistorSelectorEnum r);

//Max 10 chars!!!!
const char* GetResistorName(ResistorSelectorEnum r);

//Текущее сопротивление резистора в омах
float ResistorValue();

//Коэффициенты усиления в I/V каскадах
float GetAmplifyI();
float GetAmplifyV();


bool BoostAmplifyI();
bool BoostAmplifyV();

float ADC_MaxVoltage();
