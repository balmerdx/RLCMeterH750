#include "main.h"
#include "scene_graph.h"
#include "scene_graph_menu.h"
#include "scene_single_freq.h"
#include "plot.h"
#include "progress_bar.h"
#include "task.h"

#define SCAN_POINTS 51

static void SceneGraphQuant();
static void SceneGraphDrawFreq();
static void StartNext();
static void SceneGraphDrawGraph();
static void SceneGraphDrawGraphName();
static void SceneGraphDrawBottom();

static Point g_points1[SCAN_POINTS];
static Point g_points2[SCAN_POINTS];
static complex g_data_array[SCAN_POINTS];
static ErrorZx g_error_array[SCAN_POINTS];

static bool scan_start_next = false;
static int scan_cur = -1;
static int scan_points = -1;
static int line_point_index = -1;

//Уровень сигнала ymul = pow(10e3, last_mul_y_pow10e3_level)
static int last_mul_y_pow10e3_level = 0;

static int pb_graph_y;
static int pb_bottom_info_y;
static int pb_freq_x;
static int pb_freq_width;
static int pb_graph_name_width;

static int pb_bottom_freq_x;
static int pb_bottom_freq_width;
static int pb_bottom_re_x;
static int pb_bottom_re_width;
static int pb_bottom_im_x;
static int pb_bottom_im_width;


//Scale для графика
float g_x_to_freq = 1.f;

//Если частота больше или равна 1 KHz, то пишем в килогерцах без дробной части
int printEvenFreq(char* str, int freq, int length)
{
    char* p = str;

    if(freq < 1000)
    {
        intToString(str, freq, length, ' ');
        strcat(str, " Hz");
    } else
    {
        intToString(str, freq/1000, length, ' ');
        strcat(p, " KHz");
    }

    return strlen(str);
}

bool IsGraphData()
{
    return scan_points==scan_cur && scan_points>0;
}

void SceneGraphStart()
{
    UTF_SetFont(font_condensed30);
    pb_graph_y = UTF_Height();
    pb_bottom_info_y = UTFT_getDisplayYSize() - UTF_Height();

    pb_freq_width = UTF_StringWidth("300 KHz-500 KHz")+2;
    pb_freq_x = UTFT_getDisplayXSize()-pb_freq_width;
    pb_graph_name_width = pb_freq_x;

    pb_bottom_freq_width = UTF_StringWidth("500000 Hz");
    pb_bottom_freq_x = UTFT_getDisplayXSize()-pb_bottom_freq_width;

    pb_bottom_re_x = 0 ;
    pb_bottom_re_width = pb_bottom_freq_x/2;
    pb_bottom_im_x = pb_bottom_re_width;
    pb_bottom_im_width = pb_bottom_freq_x-pb_bottom_im_x;

    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTFT_fillRectBack(0, 0, UTFT_getDisplayXSize()-1, pb_graph_y-1);

    PlotInit(0, pb_graph_y, UTFT_getDisplayXSize(), pb_bottom_info_y-pb_graph_y);

    SceneGraphDrawFreq();

    if(IsGraphData())
        SceneGraphDrawGraph();
    SceneGraphDrawGraphName();

    SceneGraphDrawBottom();

    InterfaceGoto(SceneGraphQuant);
}


void SceneGraphQuant()
{
    if(EncButtonPressed())
    {
        SceneGraphMenuStart();
        return;
    }

    if(EncValueChanged())
    {
        if(line_point_index<0 && scan_points>0)
            line_point_index = scan_points/2;

        PlotLineSetVisible(true);
        AddSaturated(&line_point_index, EncValueDelta(), scan_points);
        PlotLineSetPosX(g_points1[line_point_index].x);
        SceneGraphDrawBottom();
    }
}


void SceneGraphDrawFreq()
{
    char freq_str[30];

    char* p = freq_str;
    p+=printEvenFreq(p, g_settings.graph_min_freq, 1);
    *p++='-';
    printEvenFreq(p, g_settings.graph_max_freq, 1);

    UTF_SetFont(font_condensed30);
    UTFT_setColorW(VGA_BLACK);
    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTF_DrawStringJustify(pb_freq_x, 0, freq_str, pb_freq_width, UTF_CENTER);
}

void SceneGraphStartScan()
{
    ProgressInit(0, pb_bottom_info_y, UTFT_getDisplayXSize(), UTFT_getDisplayYSize()-pb_bottom_info_y-1);
    ProgressSetVisible(true);

    PlotLineSetVisible(false);
    line_point_index = -1;

    scan_start_next = true;
}

static float FreqFromIndex(int idx)
{
    return g_settings.graph_min_freq + (g_settings.graph_max_freq-g_settings.graph_min_freq)*idx/(float)(scan_points-1);
}

void SceneGraphResultZx()
{
    if(!InterfaceIsActive(SceneGraphQuant))
        return;

    if(scan_start_next)
    {
        scan_start_next = false;
        scan_cur = 0;
        scan_points = SCAN_POINTS;
        StartNext();
        return;
    }

    if(scan_cur>=scan_points)
        return;

    g_data_array[scan_cur] = g_Zx;
    g_error_array[scan_cur] = g_error;
    scan_cur++;

    ProgressSetPos(scan_cur/(float)scan_points);
    if(scan_cur==scan_points)
    {
        SceneGraphDrawGraph();
        SceneGraphDrawGraphName();
        SceneGraphDrawBottom();
        return;
    }

    StartNext();
}


void StartNext()
{
    float freq = FreqFromIndex(scan_cur);
    TaskSetFreq(freq);
}

static void AddMinMax(float* ymin, float* ymax,
                      float y, bool first)
{
    if(first)
    {
        *ymin = *ymax = y;
        return;
    }

    if(y < *ymin)
        *ymin = y;

    if(y > *ymax)
        *ymax = y;
}

void SceneGraphDrawGraphName()
{
    char* str = "NONE";
    switch(g_settings.graph_type)
    {
    case GRAPH_Z_RE: str = "Zreal"; break;
    case GRAPH_Z_IM: str = "Zimag"; break;
    case GRAPH_Z_RE_Z_IM: str = "Zreal & Zimag"; break;
    case GRAPH_Z_PHASE: str = "Zphase"; break;
    case GRAPH_Z_L:
        switch(last_mul_y_pow10e3_level)
        {
        default:
        case 0: str = "L (H)"; break;
        case -1: str = "L (KH)"; break;
        case 1: str = "L (mH)"; break;
        case 2: str = "L (uH)"; break;
        case 3: str = "L (nH)"; break;
        case 4: str = "L (pH)"; break;
        }
        break;
    case GRAPH_Z_C:
        switch(last_mul_y_pow10e3_level)
        {
        default:
        case 0: str = "C (F)"; break;
        case -1: str = "C (KF)"; break;
        case 1: str = "C (mF)"; break;
        case 2: str = "C (uF)"; break;
        case 3: str = "C (nF)"; break;
        case 4: str = "C (pF)"; break;
        }
        break;
    }

    UTFT_setColorW(VGA_WHITE);
    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTF_SetFont(font_condensed30);
    UTF_DrawStringJustify(0, 0, str, pb_graph_name_width, UTF_CENTER);
}

void SceneGraphDrawGraph()
{
    float ymin = 0, ymax = 0;
    float mul_x = 1e-3;
    if(g_settings.graph_max_freq <=1000)
        mul_x = 1;
    g_x_to_freq = 1/mul_x;

    for(int i=0; i<scan_points; i++)
    {
        float freq = FreqFromIndex(i);
        g_points1[i].x = freq*mul_x;
        g_points2[i].x = freq*mul_x;
        g_points1[i].y = 0;
        g_points2[i].y = 0;

        if(g_settings.graph_type==GRAPH_Z_RE || g_settings.graph_type==GRAPH_Z_IM || g_settings.graph_type==GRAPH_Z_RE_Z_IM)
        {
            g_points1[i].y = creal(g_data_array[i]);
            g_points2[i].y = cimag(g_data_array[i]);
        }

        if(g_settings.graph_type==GRAPH_Z_PHASE)
        {
            g_points1[i].y = carg(g_data_array[i])*(180/M_PI);
        }

        if(g_settings.graph_type==GRAPH_Z_L || g_settings.graph_type==GRAPH_Z_C)
        {
            static ErrorZx error;
            static VisualInfo info;
            convertZxmToVisualInfo(g_data_array[i], freq, g_settings.view_parallel, &error, &info);

            if(g_settings.graph_type==GRAPH_Z_L)
                g_points1[i].y = info.L>0 ? info.L : 0;

            if(g_settings.graph_type==GRAPH_Z_C)
                g_points1[i].y = info.C>0 ? info.C : 0;
        }

        if(g_settings.graph_type==GRAPH_Z_RE || g_settings.graph_type==GRAPH_Z_RE_Z_IM || g_settings.graph_type==GRAPH_Z_PHASE
           || g_settings.graph_type==GRAPH_Z_L || g_settings.graph_type==GRAPH_Z_C)
            AddMinMax(&ymin, &ymax, g_points1[i].y, i==0);

        if(g_settings.graph_type==GRAPH_Z_IM || g_settings.graph_type==GRAPH_Z_RE_Z_IM)
            AddMinMax(&ymin, &ymax, g_points2[i].y, i==0);
    }


    float ybig = fmaxf(fabsf(ymin), fabsf(ymax));
    float mul_y = 1.f;
    last_mul_y_pow10e3_level = 0;
    if(ybig>1e3)
    {
        last_mul_y_pow10e3_level = -1;
        mul_y = 1e-3f;
    }

    if(ybig<1)
    {
        last_mul_y_pow10e3_level = 1;
        mul_y = 1e3f;
    }

    if(ybig<1e-3)
    {
        last_mul_y_pow10e3_level = 2;
        mul_y = 1e6f;
    }

    if(ybig<1e-6)
    {
        last_mul_y_pow10e3_level = 3;
        mul_y = 1e9f;
    }

    if(ybig<1e-9)
    {
        last_mul_y_pow10e3_level = 4;
        mul_y = 1e12f;
    }

    ymin *= mul_y;
    ymax *= mul_y;
    PlotSetAxis(g_settings.graph_min_freq*mul_x, g_settings.graph_max_freq*mul_x, ymin, ymax);
    for(int i=0; i<scan_points; i++)
    {
        g_points1[i].y *= mul_y;
        g_points2[i].y *= mul_y;
    }

    if(g_settings.graph_type==GRAPH_Z_RE || g_settings.graph_type==GRAPH_Z_RE_Z_IM )
        PlotDrawGraph(0, g_points1, scan_points, VGA_RED);

    if(g_settings.graph_type==GRAPH_Z_IM || g_settings.graph_type==GRAPH_Z_RE_Z_IM )
        PlotDrawGraph(1, g_points2, scan_points, VGA_GREEN);

    if(g_settings.graph_type==GRAPH_Z_PHASE || g_settings.graph_type==GRAPH_Z_L || g_settings.graph_type==GRAPH_Z_C)
        PlotDrawGraph(0, g_points1, scan_points, VGA_AQUA);
}

void SceneGraphDrawBottom()
{
    UTFT_setBackColorW(VGA_BLACK);
    UTFT_setColorW(VGA_WHITE);
    UTF_SetFont(font_condensed30);
    bool valid = scan_points>0 &&
            line_point_index>=0 && line_point_index<scan_points;

    if(!valid)
    {
        UTFT_fillRectBack(0, pb_bottom_info_y, UTFT_getDisplayXSize()-1, UTFT_getDisplayYSize()-1);
        return;
    }

    char str[27];

    //Draw freq
    float freq = g_points1[line_point_index].x * g_x_to_freq;
    intToString(str, lroundf(freq), 6, ' ');
    strcat(str, " Hz");
    UTF_DrawStringJustify(pb_bottom_freq_x, pb_bottom_info_y, str, pb_bottom_freq_width, UTF_RIGHT);

    const int outstr_size = 10;
    char str_re[outstr_size];
    char str_im[outstr_size];
    char str_re_type[outstr_size];
    char str_im_type[outstr_size];

    FormatReIm(g_data_array[line_point_index], g_error_array+line_point_index, freq,
               str_re, str_im,
               str_re_type, str_im_type);

    strcpy(str, str_re);
    strcat(str, str_re_type);
    UTF_DrawStringJustify(pb_bottom_re_x, pb_bottom_info_y, str, pb_bottom_re_width, UTF_RIGHT);

    strcpy(str, str_im);
    strcat(str, str_im_type);
    UTF_DrawStringJustify(pb_bottom_im_x, pb_bottom_info_y, str, pb_bottom_im_width, UTF_RIGHT);


}
