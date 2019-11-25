#pragma once

typedef enum
{
    Resistor_100_Om = 1,
    Resistor_1_KOm = 2,
    Resistor_10_KOm = 3,
} ResistorSelectorEnum;


void ResistorSelectorInit();

ResistorSelectorEnum ResistorCurrent();

void ResistorSelect(ResistorSelectorEnum r);
