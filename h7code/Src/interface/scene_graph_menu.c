#include "main.h"
#include "menu.h"
#include <string.h>
#include "ili/DefaultFonts.h"

#include "scene_graph_menu.h"
#include "scene_single_freq.h"
#include "scene_graph.h"

enum SceneGraphMenuEnum
{
    SGME_START_SCAN,
    SGME_RETURN,
    SGME_TO_SINGLE,
    SGME_SET_FREQ_MIN,
    SGME_GRAPH,
};

static void SceneGraphMenuQuant();
static void SceneSelectFreqStart(bool min_freq);
static void SceneSelectFreqQuant();

static void SceneSelectGraphStart();
static void SceneSelectGraphQuant();

static int g_default_frequencies[] =
    {   10,   30,   100,   200,
      600, 3000, 10000, 30000,
      100000, 300000, 500000
    };
static bool g_current_is_min_freq;
static int8_t g_current_selected_min_freq_index;

void SceneGraphMenuStart()
{
    UTFT_setFont(BigFont);
    MenuReset("Graph menu");
    MenuAdd("Graphics", SGME_GRAPH);
    MenuAdd("Start scan", SGME_START_SCAN);
    MenuAdd("To single freq", SGME_TO_SINGLE);
    MenuAdd("Set freq", SGME_SET_FREQ_MIN);
    MenuAdd1("..", SGME_RETURN, "Return to graph");

    MenuRedraw();
    InterfaceGoto(SceneGraphMenuQuant);
}

void SceneGraphMenuQuant()
{
    MenuQuant();
    if(!EncButtonPressed())
        return;

    if(MenuData()==SGME_RETURN)
    {
        SceneGraphStart();
        return;
    }

    if(MenuData()==SGME_START_SCAN)
    {
        SceneGraphStart();
        SceneGraphStartScan();
        return;
    }

    if(MenuData()==SGME_TO_SINGLE)
    {
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SGME_SET_FREQ_MIN)
    {
        SceneSelectFreqStart(true);
        return;
    }

    if(MenuData()==SGME_GRAPH)
    {
        SceneSelectGraphStart();
        return;
    }

}

void SceneSelectFreqStart(bool min_freq)
{
    UTFT_setFont(BigFont);
    MenuReset(min_freq?"Select min freq":"Select max freq");
    g_current_is_min_freq = min_freq;

    int min_index;
    int count = sizeof(g_default_frequencies)/sizeof(g_default_frequencies[0]);
    if(min_freq)
    {
        count--;
        min_index = 0;
    } else
    {
        min_index = g_current_selected_min_freq_index;
    }

    char st[27];
    for(int i=min_index; i<count; i++)
    {
        int freq = g_default_frequencies[i];
        printEvenFreq(st, freq, 4);
        MenuAdd(st, i);
    }

    MenuRedraw();
    InterfaceGoto(SceneSelectFreqQuant);
}

void SceneSelectFreqQuant()
{
    MenuQuant();
    if(!EncButtonPressed())
        return;

    if(g_current_is_min_freq)
    {
        g_settings.graph_min_freq = g_default_frequencies[MenuData()];
        g_current_selected_min_freq_index = MenuData()+1;
        SceneSelectFreqStart(false);
        return;
    } else {
        g_settings.graph_max_freq = g_default_frequencies[MenuData()];
        SceneGraphStart();
        SceneGraphStartScan();
        return;
    }
}

void SceneSelectGraphStart()
{
    UTFT_setFont(BigFont);
    MenuReset("Select grpah");
    MenuAdd("real(Z) Zre", GRAPH_Z_RE);
    MenuAdd("imag(Z) Zim", GRAPH_Z_IM);
    MenuAdd("Zre & Zim", GRAPH_Z_RE_Z_IM);
    MenuAdd("phase(Z) Zarg", GRAPH_Z_PHASE);
    MenuAdd("L indictance", GRAPH_Z_L);
    MenuAdd("C capacitance", GRAPH_Z_C);

    MenuSetIndex(MenuIndexByData(g_settings.graph_type));

    MenuRedraw();
    InterfaceGoto(SceneSelectGraphQuant);
}

void SceneSelectGraphQuant()
{
    MenuQuant();
    if(!EncButtonPressed())
        return;

    g_settings.graph_type = MenuData();

    SceneGraphStart();
}
