#include "main.h"
#include "menu.h"

#include "scene_single_freq_menu.h"
#include "scene_single_freq.h"
#include "scene_calibration.h"

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
};

void SceneSingleFreqMenuQuant();

void SceneSingleFreqMenuStart()
{
    MenuReset("Single freq menu");
    MenuAdd1("..", SFME_RETURN, "Return to scene");
    MenuAdd(view_parallel?"View serial":"View parallel", SFME_SERIAL_PARALLEL);
    MenuAdd("View LC", SFME_LC);
    MenuAdd("View Z real & imag", SFME_Z_REAL_IMAG);
    MenuAdd("View Z abs & arg", SFME_Z_ABS_ARG);
    MenuAdd(view_debug?"View normal":"View debug", SFME_VIEW_DEBUG);
    //MenuAdd("Select calibration", SFME_SELECT_CALIBRATION);
    MenuAdd("Set calibration", SFME_SET_CALIBRATION);

    MenuRedraw();

    InterfaceGoto(SceneSingleFreqMenuQuant);
}

void SceneSingleFreqMenuQuant()
{
    MenuQuant();
    bool pressed = EncButtonPressed();

    if(MenuData()==SFME_RETURN && pressed)
    {
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_SERIAL_PARALLEL && pressed)
    {
        view_parallel = !view_parallel;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_LC && pressed)
    {
        view_mode = VM_LC;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_Z_REAL_IMAG && pressed)
    {
        view_mode = VM_Z_REAL_IMAG;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_Z_ABS_ARG && pressed)
    {
        view_mode = VM_Z_ABS_ARG;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_VIEW_DEBUG && pressed)
    {
        view_debug = !view_debug;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SFME_SET_CALIBRATION && pressed)
    {
        SceneCalibrarionStart();
        return;
    }
}
