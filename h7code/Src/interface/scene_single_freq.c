#include "main.h"
#include "scene_single_freq.h"
#include "task.h"

void SceneSingleFreqQuant();
void SceneSingleFreqDrawFreq();

static int freq_x;
static int freq_y;
static int freq_y_max;
static int freq_width;

static const char* s_freq = "Freq = ";
static const char* s_g_single_freq_khz = " Hz";
#define FONT_OFFSET_30TO59 24

void SceneSingleFreqStart()
{
    int y;
    UTFT_setColorW(VGA_WHITE);
    UTF_SetFont(font_condensed59);
    y = 5;
    freq_y = y;
    y += UTF_Height();
    freq_y_max = y;

    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTFT_fillRectBack(0, 0, UTFT_getDisplayXSize()-1, freq_y_max-1);
    UTFT_setBackColorW(VGA_BLACK);
    UTFT_fillRectBack(0, freq_y_max, UTFT_getDisplayXSize()-1, UTFT_getDisplayYSize()-1);

    int s_freq_width;
    {
        //Рассчитываем центр строки Freq=000000 Hz
        int width = 0;
        UTF_SetFont(font_condensed30);
        s_freq_width = UTF_StringWidth(s_freq);
        width +=  s_freq_width + UTF_StringWidth(s_g_single_freq_khz);
        UTF_SetFont(font_condensed59);
        freq_width = UTF_StringWidth("000000");
        width += freq_width;

        freq_x = (UTFT_getDisplayXSize()-width)/2+s_freq_width;
    }

    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTF_SetFont(font_condensed30);
    UTF_DrawString(freq_x-s_freq_width, freq_y+FONT_OFFSET_30TO59, s_freq);
    UTF_SetFont(font_condensed59);
    freq_width = UTF_StringWidth("000000");
    UTF_SetFont(font_condensed30);
    UTF_DrawString(freq_x+freq_width, freq_y+FONT_OFFSET_30TO59, s_g_single_freq_khz);

    SceneSingleFreqDrawFreq();
    InterfaceGoto(SceneSingleFreqQuant);
}

void SceneSingleFreqQuant()
{
    if(EncValueChanged())
    {
        AddSaturated(&g_freq_index, EncValueDelta(), FREQ_INDEX_MAX);
        TaskSetFreq(StandartFreq(g_freq_index));
        SceneSingleFreqDrawFreq();
    }

}

void SceneSingleFreqDrawFreq()
{
    UTF_SetFont(font_condensed59);
    UTFT_setColorW(VGA_WHITE);
    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTF_printNumI(TaskGetFreq(), freq_x, freq_y, freq_width, UTF_RIGHT);
}
