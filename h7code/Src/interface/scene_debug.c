#include "main.h"
#include "menu.h"
#include <string.h>
#include "ili/DefaultFonts.h"

#include "scene_debug.h"
#include "scene_single_freq.h"
#include "scene_single_freq_menu.h"
#include "scene_graph.h"
#include "task.h"

enum SingleDebugEnum
{
    SDE_RETURN,
    SDE_VIEW_DEBUG,
    SDE_DEFAULT_RESISTOR,
};

static void SceneDebugMenuQuant();
static void SceneDebugDefRMenuStart();
static void SceneDebugDefRMenuQuant();

void SceneDebugMenuStart()
{
    UTFT_setFont(BigFont);
    MenuReset("Debug menu");
    MenuAdd("..", SDE_RETURN);
    MenuAdd(view_debug?"View normal":"View debug", SDE_VIEW_DEBUG);

    char str_r[20] = "Def R=";
    strncat(str_r, GetResistorName(TaskGetDefaultResistor()), sizeof(str_r)-1);
    MenuAdd(str_r, SDE_DEFAULT_RESISTOR);
    MenuRedraw();

    InterfaceGoto(SceneDebugMenuQuant);
}

void SceneDebugMenuQuant()
{
    MenuQuant();
    if(!EncButtonPressed())
        return;

    if(MenuData()==SDE_RETURN)
    {
        SceneSingleFreqMenuStart();
        return;
    }

    if(MenuData()==SDE_VIEW_DEBUG)
    {
        view_debug = !view_debug;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SDE_DEFAULT_RESISTOR)
    {
        SceneDebugDefRMenuStart();
        return;
    }

}

void SceneDebugDefRMenuStart()
{
    MenuReset("Debug menu");

    char str_r[20];
    for(int i=0; i<=Resistor_Last; i++)
    {
        strcpy(str_r, "Def R=");
        strncat(str_r, GetResistorName(i), sizeof(str_r)-1);
        MenuAdd(str_r, i);
    }

    MenuSetIndex(MenuIndexByData(TaskGetDefaultResistor()));
    MenuRedraw();

    InterfaceGoto(SceneDebugDefRMenuQuant);
}

void SceneDebugDefRMenuQuant()
{
    MenuQuant();
    if(!EncButtonPressed())
        return;

    TaskSetDefaultResistor(MenuData());
    SceneDebugMenuStart();
}
