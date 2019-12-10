#pragma once

typedef enum
{
    //Resistor_Auto нельзя передавать в ResistorSelect
    Resistor_Auto = 0,

    Resistor_100_Om = 1,
    Resistor_1_KOm = 2,
    Resistor_10_KOm = 3,
} ResistorSelectorEnum;


void ResistorSelectorInit();

ResistorSelectorEnum ResistorCurrent();

void ResistorSelect(ResistorSelectorEnum r);
