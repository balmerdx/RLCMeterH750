#include "main.h"
#include "scene_single_freq.h"
#include "task.h"
#include "srlc_format.h"
#include "scene_single_freq_menu.h"
#include "hardware/select_resistor.h"

static void SceneSingleFreqQuant();
static void SceneSingleFreqDrawFreq();
static void SceneSingleFreqDrawNames();
static void SceneSingleFreqDrawValues();
static void SceneSingleFreqDrawCurrentR();

static int freq_x;
static int freq_y;
static int freq_y_max;
static int freq_width;

static const char* s_g_single_freq_khz = " Hz";
#define FONT_OFFSET_30TO59 24

static int pb_name_x;
static int pb_name_width;
static int pb_param_width;
static int pb_param_x;
static int pb_param1_y;
static int pb_param2_y;
static int pb_param_x_type;
static int pb_param_width_type;

static SingleFreqViewMode view_mode = VM_Z;
static complex last_Zx;
static bool last_Zx_changed;

static int info_current_r_x;
static int info_current_r_y;
static int info_current_r_width;
ResistorSelectorEnum last_current_r;


//Предполагается, что str, это строчка у которой может быть - вначале
//Если минуса нет, то оставляем под него пустое место.
int DrawNumberMinus(int x, int y, const char* str, int width)
{
    if(str[0]=='-')
    {
        return UTF_DrawStringJustify(x, y, str, width, UTF_LEFT);
    }

    int minus_width = UTF_StringWidth("-");
    UTFT_fillRectBack(x, y, x+minus_width-1, y+UTF_Height()-1);

    return UTF_DrawStringJustify(x+minus_width, y, str, width-minus_width, UTF_LEFT);
}

void DrawNumberType(int x, int y, const char* str_number, const char* str_type)
{
    UTF_SetFont(font_condensed59);
    int height_big = UTF_Height();

    UTF_DrawStringJustify(x, y, str_number, pb_param_width, UTF_RIGHT);

    UTF_SetFont(font_condensed30);
    int height_small = UTF_Height();
    int yadd = FONT_OFFSET_30TO59;
    int x1 = pb_param_x_type;
    int x2 = UTF_DrawStringJustify(x1, y+yadd, str_type, pb_param_width_type, UTF_LEFT);

    UTFT_fillRectBack(x1, y, x2-1, y+yadd-1);
    UTFT_fillRectBack(x1, y+yadd+height_small, x2-1, y+height_big-1);
}

void SceneSingleFreqStart()
{
    last_Zx = 0;
    last_Zx_changed = false;

    int y;
    UTFT_setColorW(VGA_WHITE);
    UTF_SetFont(font_condensed30);
    y = 5;
    freq_y = y;
    y += UTF_Height();
    freq_y_max = y;

    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTFT_fillRectBack(0, 0, UTFT_getDisplayXSize()-1, freq_y_max-1);
    UTFT_setBackColorW(VGA_BLACK);
    UTFT_fillRectBack(0, freq_y_max, UTFT_getDisplayXSize()-1, UTFT_getDisplayYSize()-1);

    {
        //Рассчитываем центр строки 000000 Hz
        int width = 0;
        UTF_SetFont(font_condensed30);
        width +=  UTF_StringWidth(s_g_single_freq_khz);
        freq_width = UTF_StringWidth("000000");
        width += freq_width;

        freq_x = UTFT_getDisplayXSize()-width;
    }

    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTF_SetFont(font_condensed30);
    UTF_DrawString(freq_x+freq_width, freq_y, s_g_single_freq_khz);
    UTF_DrawStringJustify(0, freq_y, "RLCMeterH7", freq_x, UTF_CENTER);

    pb_name_x = 10;
    pb_name_width = 54;
    pb_param_x = pb_name_x+pb_name_width;
    UTF_SetFont(font_condensed59);
    pb_param_width = UTF_StringWidth("-00.000");
    UTF_SetFont(font_condensed30);
    pb_param_x_type = pb_param_x+pb_param_width;
    pb_param_width_type = UTF_StringWidth(" MOm");
    UTF_SetFont(font_condensed59);

    pb_param1_y = y;
    y += UTF_Height();
    pb_param2_y = y;
    y += UTF_Height();

    UTF_SetFont(font_condensed30);
    info_current_r_x = 0;
    info_current_r_y = UTFT_getDisplayYSize()-UTF_Height();
    info_current_r_width = UTF_StringWidth("Rc=10 KOm");

    UTF_DrawString(info_current_r_x + info_current_r_width+10, info_current_r_y, correctionValid()?"valid":"inval");

    SceneSingleFreqDrawFreq();
    SceneSingleFreqDrawNames();
    SceneSingleFreqDrawCurrentR();
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

    if(EncButtonPressed())
    {
        SceneSingleFreqMenuStart();
        return;
    }

    if(last_Zx_changed)
    {
        last_Zx_changed = false;
        SceneSingleFreqDrawValues();
    }

    if(ResistorCurrent()!=last_current_r)
        SceneSingleFreqDrawCurrentR();
}

void SceneSingleFreqZx(complex Zx)
{
    if(!InterfaceIsActive(SceneSingleFreqQuant))
        return;
    last_Zx = Zx;
    last_Zx_changed = true;
}

void SceneSingleFreqDrawFreq()
{
    UTF_SetFont(font_condensed30);
    UTFT_setColorW(VGA_WHITE);
    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTF_printNumI(TaskGetFreq(), freq_x, freq_y, freq_width, UTF_RIGHT);
}

void SceneSingleFreqDrawNames()
{
    UTF_SetFont(font_condensed30);
    UTFT_setColorW(VGA_WHITE);
    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);

    char* str_re = "RE=";
    char* str_im = "IM=";

    UTF_DrawStringJustify(pb_name_x, pb_param1_y+FONT_OFFSET_30TO59, str_re, pb_name_width, UTF_RIGHT);
    UTF_DrawStringJustify(pb_name_x, pb_param2_y+FONT_OFFSET_30TO59, str_im, pb_name_width, UTF_RIGHT);

}

void SceneSingleFreqDrawValues()
{
    const int outstr_size = 10;
    char str_re[outstr_size];
    char str_im[outstr_size];
    char str_re_type[outstr_size];
    char str_im_type[outstr_size];

    UTFT_setColorW(VGA_WHITE);
    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);

    float Rabs = cabsf(last_Zx);
    formatR2(str_re, str_re_type, crealf(last_Zx), Rabs);
    formatR2(str_im, str_im_type, cimagf(last_Zx), Rabs);

    UTFT_setBackColorW(VGA_BLACK);
    DrawNumberType(pb_param_x, pb_param1_y, str_re, str_re_type);
    DrawNumberType(pb_param_x, pb_param2_y, str_im, str_im_type);
}

void SceneSingleFreqDrawCurrentR()
{
    last_current_r = ResistorCurrent();

    char* str_r = "Rc=100 Om";
    if(last_current_r == Resistor_1_KOm)
        str_r = "Rc=1 KOm";
    if(last_current_r == Resistor_10_KOm)
        str_r = "Rc=10 KOm";

    UTF_SetFont(font_condensed30);
    UTFT_setColorW(VGA_WHITE);
    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTF_DrawStringJustify(info_current_r_x, info_current_r_y, str_r, info_current_r_width, UTF_RIGHT);
}
