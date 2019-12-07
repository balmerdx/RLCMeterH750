#include "main.h"
#include "interface.h"
#include "ili/DefaultFonts.h"
#include "hardware/quadrature_encoder.h"

static int16_t encValue = 0;
SceneQuantHandler g_quant_handler = NULL;

const uint32_t* g_default_font = NULL;

void InterfaceStart()
{
    UTFT_setBackColorW(VGA_BLACK);
    UTFT_clrScr();

    UTF_SetFont(g_default_font);
    UTFT_setColorW(VGA_WHITE);

    if(0)
    {
        uint16_t startMs = HAL_GetTick();
        UTFT_setColor(255, 255, 255);
        UTFT_setColor(255, 255, 255);
        int y = 0;
        UTFT_setFont(BigFont);
        UTFT_print(" !\"#$%&'()*+,-./", UTFT_CENTER, y); y+=UTFT_getFontYsize();
        UTFT_print("0123456789:;<=>?", UTFT_CENTER, y); y+=UTFT_getFontYsize();
        UTFT_print("@ABCDEFGHIJKLMNO", UTFT_CENTER, y); y+=UTFT_getFontYsize();
        UTFT_print("PQRSTUVWXYZ[\\]^_", UTFT_CENTER, y); y+=UTFT_getFontYsize();
        UTFT_print("`abcdefghijklmno", UTFT_CENTER, y); y+=UTFT_getFontYsize();
        UTFT_print("pqrstuvwxyz{|}~ ", UTFT_CENTER, y); y+=UTFT_getFontYsize();

        UTFT_setColor(255, 0, 0);
        UTFT_setFont(SmallFont);
        UTFT_print(" !\"#$%&'()*+,-./0123456789:;<=>?", UTFT_CENTER, y); y+=UTFT_getFontYsize();
        UTFT_print("@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_", UTFT_CENTER, y); y+=UTFT_getFontYsize();
        UTFT_print("`abcdefghijklmnopqrstuvwxyz{|}~ ", UTFT_CENTER, y); y+=UTFT_getFontYsize();

        UTFT_setColor(255, 255, 0);
        UTFT_setFont(FONT8x15);
        UTFT_print(" !\"#$%&'()*+,-./0123456789:;<=>?", UTFT_CENTER, y); y+=UTFT_getFontYsize();
        UTFT_print("@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_", UTFT_CENTER, y); y+=UTFT_getFontYsize();
        UTFT_print("`abcdefghijklmnopqrstuvwxyz{|}~ ", UTFT_CENTER, y); y+=UTFT_getFontYsize();

        UTFT_setColor(0, 0, 255);
        UTFT_setFont(SevenSegNumFont);
        UTFT_print("0123456789", UTFT_CENTER, y);

        uint16_t deltaMs = HAL_GetTick()-startMs;
        UTFT_setFont(BigFont);
        UTFT_printNumI(deltaMs, 0, 240-16, 5, ' ');
        while(1);
    }

    encValue = QuadEncValue();
}

void InterfaceQuant()
{
    if(g_quant_handler)
        (*g_quant_handler)();
}

void InterfaceGoto(SceneQuantHandler handler)
{
    EncClear();
    g_quant_handler = handler;
}

bool InterfaceIsActive(SceneQuantHandler handler)
{
    return g_quant_handler == handler;
}

void EncClear()
{
    if(EncValueChanged())
    {
        EncValueDelta();
    }

    EncButtonPressed();
}

bool EncValueChanged()
{
    return QuadEncValue() != encValue;
}

int EncValueDelta()
{
    int16_t v = QuadEncValue();
    int16_t delta = v - encValue;
    encValue = v;
    return delta;
}

void AddSaturated(int *value, int add, int max)
{
    *value = (*value + add)%max;
    if(*value<0)
        *value += max;

}
