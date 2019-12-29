#include "main.h"
#include "scene_graph.h"
#include "scene_graph_menu.h"
#include "plot.h"
#include "progress_bar.h"
#include "task.h"

#define SCAN_POINTS 51

static void SceneGraphQuant();
static void SceneGraphDrawFreq();
static void StartNext();
static void SceneGraphDrawGraph();
static void SceneGraphDrawGraphName();

static Point g_points1[SCAN_POINTS];
static Point g_points2[SCAN_POINTS];
static complex g_data[SCAN_POINTS];

static bool scan_start_next = false;
static int scan_cur = -1;
static int scan_points = -1;
static int line_point_index = -1;

static int pb_graph_y;
static int pb_bottom_info_y;
static int pb_freq_x;
static int pb_freq_width;
static int pb_graph_name_width;

int g_min_freq = 100;
int g_max_freq = 10000;

GraphType g_graph_type = GRAPH_Z_RE_Z_IM;

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

    pb_freq_width = UTF_StringWidth("300 KHz-500 KHz");
    pb_freq_x = UTFT_getDisplayXSize()-pb_freq_width;
    pb_graph_name_width = pb_freq_x;

    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTFT_fillRectBack(0, 0, UTFT_getDisplayXSize()-1, pb_graph_y-1);

    PlotInit(0, pb_graph_y, UTFT_getDisplayXSize(), pb_bottom_info_y-pb_graph_y);

    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTFT_fillRectBack(0, pb_bottom_info_y, UTFT_getDisplayXSize()-1, UTFT_getDisplayYSize()-1);

    SceneGraphDrawFreq();
    SceneGraphDrawGraphName();

    if(IsGraphData())
        SceneGraphDrawGraph();

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
    }
}


void SceneGraphDrawFreq()
{
    char freq_str[30];

    char* p = freq_str;
    p+=printEvenFreq(p, g_min_freq, 1);
    *p++='-';
    printEvenFreq(p, g_max_freq, 1);

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
    return g_min_freq + (g_max_freq-g_min_freq)*idx/(float)(scan_points-1);
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

    g_data[scan_cur] = g_Zxm;
    scan_cur++;

    ProgressSetPos(scan_cur/(float)scan_points);
    if(scan_cur==scan_points)
    {
        SceneGraphDrawGraph();
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
    switch(g_graph_type)
    {
    case GRAPH_Z_RE: str = "Zreal"; break;
    case GRAPH_Z_IM: str = "Zimag"; break;
    case GRAPH_Z_RE_Z_IM: str = "Zreal & Zimag"; break;
    case GRAPH_Z_PHASE: str = "Zphase"; break;
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
    if(g_max_freq <=1000)
        mul_x = 1;

    for(int i=0; i<scan_points; i++)
    {
        g_points1[i].x = FreqFromIndex(i)*mul_x;
        g_points2[i].x = FreqFromIndex(i)*mul_x;
        g_points1[i].y = 0;
        g_points2[i].y = 0;

        if(g_graph_type==GRAPH_Z_RE || g_graph_type==GRAPH_Z_IM || g_graph_type==GRAPH_Z_RE_Z_IM)
        {
            g_points1[i].y = creal(g_data[i]);
            g_points2[i].y = cimag(g_data[i]);
        }

        if(g_graph_type==GRAPH_Z_PHASE)
        {
            g_points1[i].y = carg(g_data[i])*(180/M_PI);
        }

        if(g_graph_type==GRAPH_Z_RE || g_graph_type==GRAPH_Z_RE_Z_IM || g_graph_type==GRAPH_Z_PHASE)
            AddMinMax(&ymin, &ymax, g_points1[i].y, i==0);

        if(g_graph_type==GRAPH_Z_IM || g_graph_type==GRAPH_Z_RE_Z_IM)
            AddMinMax(&ymin, &ymax, g_points2[i].y, i==0);
    }

    PlotSetAxis(g_min_freq*mul_x, g_max_freq*mul_x, ymin, ymax);

    if(g_graph_type==GRAPH_Z_RE || g_graph_type==GRAPH_Z_RE_Z_IM )
        PlotDrawGraph(0, g_points1, scan_points, VGA_RED);

    if(g_graph_type==GRAPH_Z_IM || g_graph_type==GRAPH_Z_RE_Z_IM )
        PlotDrawGraph(1, g_points2, scan_points, VGA_GREEN);

    if(g_graph_type==GRAPH_Z_PHASE)
        PlotDrawGraph(0, g_points1, scan_points, VGA_AQUA);
}
