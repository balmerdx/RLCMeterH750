#include "main.h"
#include "scene_graph.h"
#include "scene_graph_menu.h"
#include "plot.h"

static void SceneGraphQuant();
static void SceneGraphDrawFreq();

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
