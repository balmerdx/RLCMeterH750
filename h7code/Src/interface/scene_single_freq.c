#include "scene_single_freq.h"
#include "task.h"
#include "srlc_format.h"
#include "scene_single_freq_menu.h"
#include "hardware/select_resistor.h"
#include <stdio.h>

static void SceneSingleFreqQuant();
static void SceneSingleFreqDrawFreq();
static void SceneSingleFreqDrawNames();
static void SceneSingleFreqDrawValues();
static void SceneSingleFreqDrawCurrentR();
static void SceneSingleFreqDrawDebug();

static int freq_x;
static int freq_y;
static int freq_y_max;
static int freq_width;

static const char* s_g_single_freq_khz = " Hz";
#define FONT_OFFSET_30TO59 24

static int pb_name_width;
static int pb_param_width;
static int pb_param_x;
static int pb_param1_name_y;
static int pb_param2_name_y;
static int pb_param1_value_y;
static int pb_param2_value_y;
static int pb_param_x_type;
static int pb_param_width_type;
static int pb_error_x;
static int pb_error_width;

bool view_debug = false;

static complex last_Zx;
static ErrorZx last_error;
static double last_freq;
static bool last_Zx_changed;
static int last_visible_freq;

static int info_current_r_x;
static int info_current_r_y;
static int info_current_r_width;
ResistorSelectorEnum last_current_r;

static const uint16_t REAL_BACK_COLOR = VGA_MAROON;
static const uint16_t IMAG_BACK_COLOR = VGA_TEAL;


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

void calcError(float* error_abs,
               float* error_real,
               float* error_imag,
               float* error_phase
               )
{
    float error_base;
    float Rabs = cabsf(last_Zx);

    if(last_error.is_big)
    {
        error_base = last_error.err_Y*Rabs*100;
    } else {
        error_base = last_error.err_R/Rabs*100;
    }

    *error_abs = error_base;

    float R  = fabsf(crealf(last_Zx));
    *error_real =  (R<1e-4f)?100:Rabs/R*error_base;

    R  = fabsf(cimagf(last_Zx));
    *error_imag =  (R<1e-4f)?100:Rabs/R*error_base;

    *error_phase = (*error_real>*error_imag)?*error_real:*error_imag;
}
//
void FillError(char* str, float error)
{
    if(error>99)
        error = 99;

    int places = 2;
    if(error>10)
        places = 0;
    else
    if(error>1)
        places = 1;

    strcpy(str, "±");
    str += strlen(str);
    str += floatToString(str, 10, error, places, 0, false);
    str[0] = '%';
    str[1] = 0;
}

void FormatReIm(complex Zx,
                ErrorZx* error,
                double freq,
                char* str_re,
                char* str_im,
                char* str_re_type,
                char* str_im_type)
{
    static VisualInfo info;
    convertZxmToVisualInfo(Zx, freq, g_settings.view_parallel, error, &info);

    float Rabs = cabsf(Zx);
    if(g_settings.view_mode == VM_Z_ABS_ARG)
    {
        formatR2(str_re, str_re_type, Rabs, Rabs);
    } else
    {
        formatR2(str_re, str_re_type, info.Rre, g_settings.view_parallel?fabsf(info.Rre):Rabs);
    }

    if(g_settings.view_mode == VM_Z_ABS_ARG)
    {
        float angle = cargf(last_Zx)*180/M_PI;
        floatToString(str_im, 16, angle, 2, 3, true);
        str_im_type[0] = 0;
    } else
    if(g_settings.view_mode == VM_LC)
    {
        if(info.is_inductance)
            formatL2(str_im, str_im_type, info.L);
        else
            formatC2(str_im, str_im_type, info.C);
    } else
    {
        formatR2(str_im, str_im_type, info.Rim, g_settings.view_parallel?fabsf(info.Rim):Rabs);
    }

    if(info.is_inf)
    {
        str_re[0] = 0;
        strcpy(str_re_type, "inf");
        str_im[0] = 0;
        strcpy(str_im_type, "inf");
    }
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
    y = 2;
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

    UTF_SetFont(font_condensed59);
    pb_param_width = UTF_StringWidth("-00.000");
    UTF_SetFont(font_condensed30);
    pb_param_width_type = UTF_StringWidth(" MOm");
    pb_error_width = UTF_StringWidth("±0.01%");
    pb_error_x = UTFT_getDisplayXSize() - pb_error_width;

    pb_name_width = pb_param_width+pb_param_width_type;
    pb_param_x = (UTFT_getDisplayXSize()-pb_name_width)/2-12;

    pb_param_x_type = pb_param_x+pb_param_width;

    UTF_SetFont(font_condensed30);
    pb_param1_name_y = y;
    y += UTF_Height();
    UTF_SetFont(font_condensed59);
    pb_param1_value_y = y;
    y += UTF_Height();
    UTF_SetFont(font_condensed30);
    pb_param2_name_y = y;
    y += UTF_Height();
    UTF_SetFont(font_condensed59);
    pb_param2_value_y = y;
    y += UTF_Height();

    UTFT_setBackColorW(REAL_BACK_COLOR);
    UTFT_fillRectBack(0, pb_param1_name_y, pb_param_x-1, pb_param2_name_y-1);
    UTFT_fillRectBack(pb_param_x+pb_name_width, pb_param1_name_y, UTFT_getDisplayXSize()-1, pb_param2_name_y-1);

    UTFT_setBackColorW(IMAG_BACK_COLOR);
    UTFT_fillRectBack(0, pb_param2_name_y, pb_param_x-1, y-1);
    UTFT_fillRectBack(pb_param_x+pb_name_width, pb_param2_name_y, UTFT_getDisplayXSize()-1, y-1);

    UTF_SetFont(font_condensed30);
    info_current_r_x = 0;
    info_current_r_y = UTFT_getDisplayYSize()-UTF_Height();
    info_current_r_width = UTF_StringWidth("Rc=10 KOm+");

    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    char buf[30];
    strcpy(buf, "cor=");
    if(!g_enable_correction)
        strncat(buf, "disabl", sizeof(buf)-1);
    else
    if(CorrectionValid())
        strncat(buf, g_correction_names[g_settings.correction_index], sizeof(buf)-1);
    else
    {
        UTFT_setColorW(VGA_RED);
        strncat(buf, "inval", sizeof(buf)-1);
    }
    UTF_DrawString(info_current_r_x + info_current_r_width+10, info_current_r_y, buf);
    UTFT_setColorW(VGA_WHITE);

    SceneSingleFreqDrawFreq();
    SceneSingleFreqDrawNames();
    SceneSingleFreqDrawCurrentR();
    InterfaceGoto(SceneSingleFreqQuant);
}

void SceneSingleFreqQuant()
{
    if(EncValueChanged())
    {
        AddSaturated(&g_settings.single_freq_index, EncValueDelta(), FREQ_INDEX_MAX);
        TaskSetFreq(StandartFreq(g_settings.single_freq_index));
    }

    if(last_visible_freq != TaskGetFreq())
        SceneSingleFreqDrawFreq();

    if(EncButtonPressed())
    {
        SceneSingleFreqMenuStart();
        return;
    }

    if(last_Zx_changed)
    {
        last_Zx_changed = false;
        if(view_debug)
            SceneSingleFreqDrawDebug();
        else
            SceneSingleFreqDrawValues();
    }

    if(ResistorCurrent()!=last_current_r)
        SceneSingleFreqDrawCurrentR();
}

void SceneSingleFreqZx()
{
    if(!InterfaceIsActive(SceneSingleFreqQuant))
        return;
    last_Zx = g_Zx;
    last_error = g_error;
    last_freq = TaskGetFreq();
    last_Zx_changed = true;
}

void SceneSingleFreqDrawFreq()
{
    UTF_SetFont(font_condensed30);
    UTFT_setColorW(VGA_WHITE);
    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTF_printNumI(TaskGetFreq(), freq_x, freq_y, freq_width, UTF_RIGHT);

    last_visible_freq = TaskGetFreq();
}

void SceneSingleFreqDrawNames()
{
    UTF_SetFont(font_condensed30);
    UTFT_setColorW(VGA_WHITE);

    char* str_re = "real(Z)";
    char* str_im = "imag(Z)";

    if(g_settings.view_mode == VM_LC)
    {
        str_re = g_settings.view_parallel?"EPR":"ESR";
        str_im = "L/C";
    }

    if(g_settings.view_mode == VM_Z_ABS_ARG)
    {
        str_re = "abs(Z)";
        str_im = "arg(Z)°";
    }

    UTFT_setBackColorW(REAL_BACK_COLOR);
    UTF_DrawStringJustify(pb_param_x, pb_param1_name_y, str_re, pb_name_width, UTF_CENTER);
    UTFT_setBackColorW(IMAG_BACK_COLOR);
    UTF_DrawStringJustify(pb_param_x, pb_param2_name_y, str_im, pb_name_width, UTF_CENTER);
}

void SceneSingleFreqDrawValues()
{
    const int outstr_size = 10;
    char str_re[outstr_size];
    char str_im[outstr_size];
    char str_re_type[outstr_size];
    char str_im_type[outstr_size];
    char err_re[outstr_size];
    char err_im[outstr_size];

    float error_abs, error_real, error_imag, error_phase;

    calcError(&error_abs, &error_real, &error_imag, &error_phase);

    if(g_settings.view_mode == VM_Z_ABS_ARG)
    {
        FillError(err_re, error_abs);
        FillError(err_im, error_phase);
    } else
    {
        FillError(err_re, error_real);
        FillError(err_im, error_imag);
    }

    UTFT_setColorW(VGA_WHITE);
    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);

    FormatReIm(last_Zx, &last_error, last_freq,
               str_re, str_im,
               str_re_type, str_im_type);


    UTFT_setBackColorW(REAL_BACK_COLOR);
    DrawNumberType(pb_param_x, pb_param1_value_y, str_re, str_re_type);
    UTF_DrawStringJustify(pb_error_x, pb_param1_value_y+FONT_OFFSET_30TO59, err_re, pb_error_width, UTF_LEFT);

    UTFT_setBackColorW(IMAG_BACK_COLOR);
    DrawNumberType(pb_param_x, pb_param2_value_y, str_im, str_im_type);
    UTF_DrawStringJustify(pb_error_x, pb_param2_value_y+FONT_OFFSET_30TO59, err_im, pb_error_width, UTF_LEFT);
}

void SceneSingleFreqDrawCurrentR()
{
    last_current_r = ResistorCurrent();

    char str_r[15] = "Rc=";
    strncat(str_r, GetResistorName(last_current_r), sizeof(str_r)-1);

    UTF_SetFont(font_condensed30);
    UTFT_setColorW(VGA_WHITE);
    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTF_DrawStringJustify(info_current_r_x, info_current_r_y, str_r, info_current_r_width, UTF_RIGHT);
}

void SceneSingleFreqDrawDebug()
{
    char buf[32];
    UTF_SetFont(font_condensed30);
    UTFT_setColorW(VGA_WHITE);
    int y;

    UTFT_setBackColorW(REAL_BACK_COLOR);

    y = pb_param1_name_y;
    strcpy(buf, "abs(a)=");
    floatToString(buf+strlen(buf), 20, cabs(g_result.sum_a), 1, 7, false);
    UTF_DrawStringJustify(pb_param_x, y, buf, pb_name_width, UTF_CENTER);

    y += UTF_Height();
    sprintf(buf, "mid_a=%i", (int)g_result.mid_a);
    UTF_DrawStringJustify(pb_param_x, y, buf, pb_name_width, UTF_CENTER);


    y += UTF_Height();
    strcpy(buf, "abs(Zxm)=");
    floatToString(buf+strlen(buf), 20, cabs(g_Zxm), 1, 7, false);
    UTF_DrawStringJustify(pb_param_x, y, buf, pb_name_width, UTF_CENTER);

    UTFT_setBackColorW(IMAG_BACK_COLOR);

    y = pb_param2_name_y;
    strcpy(buf, "abs(b)=");
    floatToString(buf+strlen(buf), 20, cabs(g_result.sum_b), 1, 7, false);
    UTF_DrawStringJustify(pb_param_x, y, buf, pb_name_width, UTF_CENTER);

    y += UTF_Height();
    sprintf(buf, "mid_b=%i", (int)g_result.mid_b);
    UTF_DrawStringJustify(pb_param_x, y, buf, pb_name_width, UTF_CENTER);

    y += UTF_Height();
    strcpy(buf, "err(im)=");
    floatToString(buf+strlen(buf), 20, g_error.err_R, 4, 7, false);
    UTF_DrawStringJustify(pb_param_x, y, buf, pb_name_width, UTF_CENTER);
}
