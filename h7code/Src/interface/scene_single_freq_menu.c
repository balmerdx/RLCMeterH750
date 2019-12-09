#include "main.h"
#include "menu.h"

#include "scene_single_freq_menu.h"
#include "scene_single_freq.h"
#include "scene_calibration.h"

enum SingleFreqMenuEnum
{
    SFME_RETURN,
    SFME_SELECT_CALIBRATION,
    SFME_SET_CALIBRATION,
};

void SceneSingleFreqMenuQuant();

void SceneSingleFreqMenuStart()
{
    MenuReset("Single freq menu");
    MenuAdd1("..", SFME_RETURN, "Return to scene");
    MenuAdd("Select calibration", SFME_SELECT_CALIBRATION);
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

    if(MenuData()==SFME_SELECT_CALIBRATION && pressed)
    {
        SceneCalibrarionStart();
        return;
    }
}
