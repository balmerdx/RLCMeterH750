#include "main.h"
#include "menu.h"
#include "ili/DefaultFonts.h"
#include "scene_calibration_menu.h"
#include "scene_calibration.h"
#include "scene_single_freq.h"
#include "task.h"

enum SingleCalibrationEnum
{
    SCE_CORRECTION_0,
    SCE_CORRECTION_1,
    SCE_CORRECTION_2,
    SCE_CORRECTION_DISABLE,
    SCE_SET_CALIBRATION,
};

static void SceneCalibrationMenuQuant();

void SceneCalibrationMenuStart()
{
    UTFT_setFont(BigFont);
    char buf[30];
    strcpy(buf, "Calibration ");
    strncat(buf, g_enable_correction?g_correction_names[g_settings.correction_index]:"Disabled", sizeof(buf)-1);
    MenuReset(buf);

    for(int i=0; i<CORRECTION_COUNT; i++)
    {
        strcpy(buf, "Correction-");
        strncat(buf, g_correction_names[i], sizeof(buf)-1);
        MenuAdd(buf, SCE_CORRECTION_0+i);
    }

    MenuAdd("Correction-Disabled", SCE_CORRECTION_DISABLE);

    strcpy(buf, "Calibrate ");
    strncat(buf, g_correction_names[g_settings.correction_index], sizeof(buf)-1);
    MenuAdd(buf, SCE_SET_CALIBRATION);

    MenuRedraw();
    InterfaceGoto(SceneCalibrationMenuQuant);
}

void SceneCalibrationMenuQuant()
{
    MenuQuant();
    if(!EncButtonPressed())
        return;

    if(MenuData()==SCE_CORRECTION_0
    || MenuData()==SCE_CORRECTION_1
    || MenuData()==SCE_CORRECTION_2)
    {
        g_settings.correction_index = MenuData()-SCE_CORRECTION_0;
        g_enable_correction = true;
        CorrectionLoad(g_settings.correction_index);
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SCE_CORRECTION_DISABLE)
    {
        g_enable_correction = false;
        SceneSingleFreqStart();
        return;
    }

    if(MenuData()==SCE_SET_CALIBRATION)
    {
        SceneCalibrarionStart();
        return;
    }
}
