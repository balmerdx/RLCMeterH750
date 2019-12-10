#include "main.h"
#include "menu.h"
#include "scene_calibration.h"
#include "scene_single_freq_menu.h"
#include "progress_bar.h"
#include "task.h"
#include "measure/corrector.h"

enum CalibrationMenuEnum
{
    CME_SHORT = 0,
    CME_OPEN,
    CME_100_Om,
    CME_1_KOm,
    CME_10_KOm,
    CME_COUNT, //Это не пункт меню, а конец калибровочной части меню

    CME_SAVE_AND_EXIT,
    CME_DISCARD_AND_EXIT,
};

static bool calibrated[CME_COUNT];
static int current_freq_index;

void SceneCalibrarionQuant();

void SceneCalibrarionStart()
{
    MenuReset("Calibration");
    MenuAdd("Short", CME_SHORT);
    MenuAdd("Open", CME_OPEN);
    MenuAdd("100 Om", CME_100_Om);
    MenuAdd("1 KOm", CME_1_KOm);
    MenuAdd("10 KOm", CME_10_KOm);
    MenuAdd("Save & Exit", CME_SAVE_AND_EXIT);
    MenuAdd("Discard & Exit", CME_DISCARD_AND_EXIT);
    MenuRedraw();

    UTF_SetFont(g_default_font);
    ProgressInit(0, UTFT_getDisplayYSize()-UTF_Height()*3-5, UTFT_getDisplayXSize(), UTF_Height());

    InterfaceGoto(SceneCalibrarionQuant);

    for(int i=0; i<CME_COUNT; i++)
        calibrated[i] = false;
}

void SceneCalibrarionQuant()
{
    MenuQuant();
    bool pressed = EncButtonPressed();

    if((MenuData()==CME_SAVE_AND_EXIT || MenuData()==CME_DISCARD_AND_EXIT) && pressed)
    {
        TaskSetDefaultResistor(Resistor_Auto);
        SceneSingleFreqMenuStart();
        return;
    }

    if(MenuData()<CME_COUNT && pressed)
    {
        current_freq_index = -1;
        ProgressSetVisible(true);
    }
}

void SceneCalibrarionZx(complex Zx)
{
    if(!ProgressVisible())
        return;

    if(current_freq_index>=0 && current_freq_index<FREQ_INDEX_MAX)
    {
        CorrectionOneFreq* of = g_corrections.freq + current_freq_index;
        of->freq = StandartFreq(current_freq_index);

        if(MenuIndex()==CME_SHORT)
        {
            of->short_100_Om.Zsm = Zx;
        }
    }

    current_freq_index++;
    if(current_freq_index>=FREQ_INDEX_MAX)
    {
        TaskSetDefaultResistor(Resistor_Auto);
        ProgressSetVisible(false);
        if(MenuIndex()==CME_SHORT)
            MenuSetNameAndUpdate(MenuIndexByData(CME_SHORT), "Short --ok");
        if(MenuIndex()==CME_OPEN)
            MenuSetNameAndUpdate(MenuIndexByData(CME_OPEN), "Open --ok");
        if(MenuIndex()==CME_100_Om)
            MenuSetNameAndUpdate(MenuIndexByData(CME_100_Om), "100 Om --ok");
        if(MenuIndex()==CME_1_KOm)
            MenuSetNameAndUpdate(MenuIndexByData(CME_1_KOm), "1 KOm --ok");
        if(MenuIndex()==CME_10_KOm)
            MenuSetNameAndUpdate(MenuIndexByData(CME_10_KOm), "10 KOm --ok");
        return;
    }


    ProgressSetPos(current_freq_index/(float)FREQ_INDEX_MAX);

    if(MenuIndex()==CME_SHORT)
        TaskSetDefaultResistor(Resistor_100_Om);
    if(MenuIndex()==CME_100_Om)
        TaskSetDefaultResistor(Resistor_100_Om);
    if(MenuIndex()==CME_1_KOm)
        TaskSetDefaultResistor(Resistor_1_KOm);
    if(MenuIndex()==CME_10_KOm)
        TaskSetDefaultResistor(Resistor_10_KOm);

    TaskSetFreq(StandartFreq(current_freq_index));

}
