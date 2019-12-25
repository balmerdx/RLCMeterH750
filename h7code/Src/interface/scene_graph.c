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

static Point g_points[SCAN_POINTS];
static complex g_data[SCAN_POINTS];

static bool scan_start_next = false;
static int scan_cur;
static int scan_points;

static int pb_graph_y;
static int pb_bottom_info_y;
static int pb_freq_x;
static int pb_freq_width;

int g_min_freq = 100;
int g_max_freq = 10000;


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

void SceneGraphStart()
{
    scan_cur = scan_points = -1;

    UTF_SetFont(font_condensed30);
    pb_graph_y = UTF_Height();
    pb_bottom_info_y = UTFT_getDisplayYSize() - UTF_Height();

    pb_freq_width = UTF_StringWidth("300 KHz-500 KHz");
    pb_freq_x = UTFT_getDisplayXSize()-pb_freq_width;

    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTFT_fillRectBack(0, 0, UTFT_getDisplayXSize()-1, pb_graph_y-1);

    PlotInit(0, pb_graph_y, UTFT_getDisplayXSize(), pb_bottom_info_y-pb_graph_y);

    UTFT_setBackColorW(COLOR_BACKGROUND_BLUE);
    UTFT_fillRectBack(0, pb_bottom_info_y, UTFT_getDisplayXSize()-1, UTFT_getDisplayYSize()-1);

    SceneGraphDrawFreq();

    InterfaceGoto(SceneGraphQuant);
}


void SceneGraphQuant()
{
    if(EncButtonPressed())
    {
        SceneGraphMenuStart();
        return;
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

    scan_start_next = true;
}

static float FreqFromIndex(int idx)
{
    return g_min_freq + (g_max_freq-g_min_freq)*idx/(float)(scan_points-1);
}

void SceneGraphResultZx(complex Zxm)
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

    g_data[scan_cur] = Zxm;
    scan_cur++;

    ProgressSetPos(scan_cur/(float)scan_points);
    if(scan_cur==scan_points)
    {
        float ymin, ymax;
        float mul_x = 1e-3;
        if(g_max_freq <=1000)
            mul_x = 1;

        for(int i=0; i<scan_points; i++)
        {
            g_points[i].x = FreqFromIndex(i)*mul_x;
            g_points[i].y = cimag(g_data[i]);
        }

        ymin = ymax = g_points[0].y;
        for(int i=0; i<scan_points; i++)
        {
            float y = g_points[i].y;
            if(y < ymin)
                ymin = y;
            if(y > ymax)
                ymax = y;
        }

        PlotSetAxis(g_min_freq*mul_x, g_max_freq*mul_x, ymin, ymax);
        PlotDrawGraph(0, g_points, scan_points, VGA_GREEN);
        return;
    }

    StartNext();
}


void StartNext()
{
    float freq = FreqFromIndex(scan_cur);
    TaskSetFreq(freq);
}
