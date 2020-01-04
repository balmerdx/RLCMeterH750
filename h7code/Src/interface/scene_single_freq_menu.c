#include "main.h"
#include "menu.h"

#include "scene_single_freq_menu.h"
#include "scene_single_freq.h"
#include "scene_calibration.h"
#include "scene_graph.h"
#include "ili/DefaultFonts.h"
#include "task.h"

enum SingleFreqMenuEnum
{
    SFME_RETURN,
    SFME_SERIAL_PARALLEL,
    SFME_LC,
    SFME_Z_REAL_IMAG,
    SFME_Z_ABS_ARG,
    SFME_SELECT_CALIBRATION,
    SFME_SET_CALIBRATION,
    SFME_VIEW_DEBUG,
    SFME_TO_GRAPH,
    SFME_ENABLE_CORRECTION,
};

void SceneSingleFreqMenuQuant();

void SceneSingleFreqMenuStart()
{
    UTFT_setFont(BigFont);
    MenuReset("Single freq menu");
    MenuAdd1("..", SFME_RETURN, "Return to scene");
    MenuAdd(g_settings.view_parallel?"View serial":"View parallel", SFME_SERIAL_PARALLEL);
    MenuAdd("View LC", SFME_LC);
    MenuAdd("View Z real & imag", SFME_Z_REAL_IMAG);
    MenuAdd("View Z abs & arg", SFME_Z_ABS_ARG);
    MenuAdd(view_debug?"View normal":"View debug", SFME_VIEW_DEBUG);
    //MenuAdd("Select calibration", SFME_SELECT_CALIBRATION);
    MenuAdd("Set calibration", SFME_SET_CALIBRATION);
    MenuAdd(g_enable_correction?"Correction - enabled":"Correction - disabled", SFME_ENABLE_CORRECTION);

    MenuAdd("Switch to graph", SFME_TO_GRAPH);

    MenuRedraw();

    InterfaceGoto(SceneSingleFreqMenuQuant);
}

void SceneSingleFreqMenuQuant()
{
    MenuQuant();
    if(!EncButtonPressed())
        return;

    if(MenuData()==SFME_RETURN)
    {
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_SERIAL_PARALLEL)
    {
        g_settings.view_parallel = !g_settings.view_parallel;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_LC)
    {
        g_settings.view_mode = VM_LC;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_Z_REAL_IMAG)
    {
        g_settings.view_mode = VM_Z_REAL_IMAG;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_Z_ABS_ARG)
    {
        g_settings.view_mode = VM_Z_ABS_ARG;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_VIEW_DEBUG)
    {
        view_debug = !view_debug;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_SET_CALIBRATION)
    {
        SceneCalibrarionStart();
        return;
    }

    if(MenuData()==SFME_TO_GRAPH)
    {
        SceneGraphStart();
        return;
    }

    if(MenuData()==SFME_ENABLE_CORRECTION)
    {
        g_enable_correction = !g_enable_correction;
        SceneSingleFreqStart();
        return;
    }
}
