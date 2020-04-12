#include "main.h"
#include "menu.h"
#include <string.h>
#include "ili/DefaultFonts.h"

#include "scene_graph_menu.h"
#include "scene_single_freq.h"
#include "scene_graph.h"
#include "scene_get_float.h"

#include <stdio.h>

enum SceneGraphMenuEnum
{
    SGME_BAD,
    SGME_START_SCAN,
    SGME_RETURN,
    SGME_TO_SINGLE,
    SGME_SET_FREQ_MIN,
    SGME_FREQ_MIN,
    SGME_FREQ_MAX,
    SGME_GRAPH,
};

static void SceneGraphMenuQuant();
static void SceneSelectFreqStart(bool min_freq);
static void SceneSelectFreqQuant();

static void SceneSelectGraphStart();
static void SceneSelectGraphQuant();
static void OnFloatDialogResult();

static int g_default_frequencies[] =
    {   10,   33,   100,   330,
      1000, 3300, 10000, 33000,
      100000, 330000, 500000
    };
static bool g_current_is_min_freq;
static int8_t g_current_selected_min_freq_index;
static int last_sgme = SGME_BAD;

void OnFloatDialogResult()
{
    if(last_sgme==SGME_FREQ_MIN && SceneGetFloatOk())
    {
        g_settings.graph_min_freq = SceneGetFloat();
    }

    if(last_sgme==SGME_FREQ_MAX && SceneGetFloatOk())
    {
        g_settings.graph_max_freq = SceneGetFloat();
    }

    if(g_settings.graph_min_freq < 10)
        g_settings.graph_min_freq = 10;

    if(g_settings.graph_max_freq > 500000)
        g_settings.graph_max_freq = 500000;

    if(g_settings.graph_min_freq > g_settings.graph_max_freq)
        g_settings.graph_max_freq = g_settings.graph_min_freq+10;

    last_sgme = SGME_BAD;
}

void SceneGraphMenuStart()
{
    OnFloatDialogResult();

    char buf[32];
    UTFT_setFont(BigFont);
    MenuReset("Graph menu");
    MenuAdd("Graphics", SGME_GRAPH);
    MenuAdd("Start scan", SGME_START_SCAN);
    MenuAdd("To single freq", SGME_TO_SINGLE);
    MenuAdd("Set freq", SGME_SET_FREQ_MIN);

    sprintf(buf, "Freq min=%li", g_settings.graph_min_freq);
    MenuAdd(buf, SGME_FREQ_MIN);

    sprintf(buf, "Freq max=%li", g_settings.graph_max_freq);
    MenuAdd(buf, SGME_FREQ_MAX);

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

    if(MenuData()==SGME_FREQ_MIN)
    {
        last_sgme = MenuData();
        SceneGetFloatStart("Min freq", 0, 5, 5, 5, SceneGraphMenuStart);
        return;
    }

    if(MenuData()==SGME_FREQ_MAX)
    {
        last_sgme = MenuData();
        SceneGetFloatStart("Max freq", 0, 5, 5, 5, SceneGraphMenuStart);
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
    int32_t closest_data = min_index;
    int32_t closest_freq = 0;
    int32_t required_feeq = min_freq ? g_settings.graph_min_freq : g_settings.graph_max_freq;

    for(int i=min_index; i<count; i++)
    {
        int freq = g_default_frequencies[i];

        if(i==min_index || fabsf(freq-required_feeq)<fabsf(closest_freq-required_feeq))
        {
            closest_data = i;
            closest_freq = freq;
        }

        printEvenFreq(st, freq, 4);
        MenuAdd(st, i);
    }

    MenuSetIndex(MenuIndexByData(closest_data));

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
