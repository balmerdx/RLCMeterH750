#include "main.h"
#include "menu.h"
#include "scene_calibration.h"
#include "scene_single_freq_menu.h"
#include "scene_get_float.h"
#include "progress_bar.h"
#include "task.h"
#include "measure/corrector.h"

enum CalibrationMenuEnum
{
    CME_SHORT = 0,
    CME_OPEN,
    CME_10_Om,
    CME_100_Om,
    CME_1_KOm,
    CME_10_KOm,
    CME_100_KOm,
    CME_COUNT, //Это не пункт меню, а конец калибровочной части меню

    CME_SAVE_AND_EXIT,
    CME_DISCARD_AND_EXIT,
};

//10 Om, 100 Om
#define COUNT_SHORT_PASS 2

//100 Om, 1 KOm, 10 KOm, 10 KOm+
#define COUNT_OPEN_PASS 4

static bool calibrated[CME_COUNT];
static int current_index;

//Индекс меню после получения float
//Может быть равен только пунктам с резистором
static int g_menu_data_last = -1;

static char* menu_names[CME_COUNT] =
{
    "Short",
    "Open",
    "10 Om",
    "100 Om",
    "1 KOm",
    "10 KOm",
    "100 KOm",
};

static char* menu_names_ok[CME_COUNT] =
{
    "Short --ok",
    "Open --ok",
    "10 Om --ok",
    "100 Om --ok",
    "1 KOm --ok",
    "10 KOm --ok",
    "100 KOm --ok",
};

void SceneCalibrarionQuant();

void SceneCalibrarionStart()
{
    if(g_menu_data_last==-1)
    {
        g_corrections.magic = 0;
        g_corrections.r_10_Ohm = 10.f;
        g_corrections.r_100_Ohm = 100.f;
        g_corrections.r_1_KOhm = 1e3f;
        g_corrections.r_10_KOhm = 10e3f;
        g_corrections.r_100_KOhm = 100e3f;
        for(int i=0; i<CME_COUNT; i++)
            calibrated[i] = false;
    }

    MenuReset("Calibration");

    for(int i=0; i<CME_COUNT; i++)
        MenuAdd(calibrated[i]?menu_names_ok[i]:menu_names[i], i);

    MenuAdd("Save & Exit", CME_SAVE_AND_EXIT);
    MenuAdd("Discard & Exit", CME_DISCARD_AND_EXIT);

    if(g_menu_data_last>=0)
        MenuSetIndex(MenuIndexByData(g_menu_data_last));

    MenuRedraw();

    UTF_SetFont(g_default_font);
    ProgressInit(0, UTFT_getDisplayYSize()-UTF_Height()*3-5, UTFT_getDisplayXSize(), UTF_Height());

    InterfaceGoto(SceneCalibrarionQuant);

    if(g_menu_data_last>=0)
    {
        if(SceneGetFloatOk())
        {
            ProgressSetVisible(true);
            if(g_menu_data_last==CME_10_Om)
            {
                current_index = -1;
                g_corrections.r_10_Ohm = SceneGetFloat();
                TaskSetDefaultResistor(Resistor_100_Om_Voltage_Boost);
            }

            if(g_menu_data_last==CME_100_Om)
            {
                current_index = -1;
                g_corrections.r_100_Ohm = SceneGetFloat();
                TaskSetDefaultResistor(Resistor_100_Om);
            }

            if(g_menu_data_last==CME_1_KOm)
            {
                current_index = -1;
                g_corrections.r_1_KOhm = SceneGetFloat();
                TaskSetDefaultResistor(Resistor_1_KOm);
            }

            if(g_menu_data_last==CME_10_KOm)
            {
                current_index = -1;
                g_corrections.r_10_KOhm = SceneGetFloat();
                TaskSetDefaultResistor(Resistor_10_KOm);
            }

            if(g_menu_data_last==CME_100_KOm)
            {
                current_index = -1;
                g_corrections.r_100_KOhm = SceneGetFloat();
                TaskSetDefaultResistor(Resistor_10_KOm_Current_Boost);
            }
        }

        g_menu_data_last = -1;
    }
}

void SceneCalibrarionQuant()
{
    MenuQuant();
    bool pressed = EncButtonPressed();

    if((MenuData()==CME_SAVE_AND_EXIT || MenuData()==CME_DISCARD_AND_EXIT) && pressed)
    {
        if(MenuData()==CME_SAVE_AND_EXIT)
        {
            bool all_calibrated = true;
            for(int i=0; i<CME_COUNT; i++)
                all_calibrated = all_calibrated || calibrated[i];
            if(all_calibrated)
            {
                CorrectionSave(g_settings.correction_index);
            }
        }

        TaskSetDefaultResistor(Resistor_Auto);
        SceneSingleFreqMenuStart();
        return;
    }

    if(MenuData()<CME_COUNT && pressed)
    {
        current_index = -1;
        if(MenuData()==CME_SHORT)
        {
            ProgressSetVisible(true);
            TaskSetDefaultResistor(Resistor_100_Om);
        }

        if(MenuData()==CME_10_Om)
        {
            g_menu_data_last = MenuData();
            SceneGetFloatStart("Precise 10 Ohm",
                        0, 1, 5, 1, SceneCalibrarionStart);
            return;
        }

        if(MenuData()==CME_100_Om)
        {
            g_menu_data_last = MenuData();
            SceneGetFloatStart("Precise 100 Ohm",
                        0, 1, 5, 2, SceneCalibrarionStart);
            return;
        }

        if(MenuData()==CME_1_KOm)
        {
            g_menu_data_last = MenuData();
            SceneGetFloatStart("Precise 1 KOhm",
                        0, 1, 5, 3, SceneCalibrarionStart);
            return;
        }

        if(MenuData()==CME_10_KOm)
        {
            g_menu_data_last = MenuData();
            SceneGetFloatStart("Precise 10 KOhm",
                        0, 1, 5, 4, SceneCalibrarionStart);
            return;
        }

        if(MenuData()==CME_100_KOm)
        {
            g_menu_data_last = MenuData();
            SceneGetFloatStart("Precise 100 KOhm",
                        0, 1, 5, 5, SceneCalibrarionStart);
            return;
        }

        if(MenuData()==CME_OPEN)
        {
            ProgressSetVisible(true);
            TaskSetDefaultResistor(Resistor_100_Om);
        }
    }
}

void SceneCalibrarionZx(complex Zx)
{
    if(!InterfaceIsActive(SceneCalibrarionQuant))
        return;
    if(!ProgressVisible())
        return;

    int end_index = FREQ_INDEX_MAX;
    if(MenuData()==CME_SHORT)
        end_index = FREQ_INDEX_MAX*COUNT_SHORT_PASS;
    if(MenuData()==CME_OPEN)
        end_index = FREQ_INDEX_MAX*COUNT_OPEN_PASS;

    if(current_index>=0)
    {
        int current_freq_index = current_index%FREQ_INDEX_MAX;
        CorrectionOneFreq* of = g_corrections.freq + current_freq_index;
        of->freq = StandartFreq(current_freq_index);

        if(MenuData()==CME_SHORT)
        {
            if(ResistorCurrent()==Resistor_100_Om_Voltage_Boost)
                of->short_100_Om_Voltage_Boost.Zsm = Zx;
            if(ResistorCurrent()==Resistor_100_Om)
                of->short_100_Om.Zsm = Zx;
        }

        if(MenuData()==CME_10_Om)
        {
            of->short_100_Om_Voltage_Boost.Zstdm = Zx;
        }

        if(MenuData()==CME_100_Om)
        {
            of->short_100_Om.Zstdm = Zx;
            of->open_100_Om.Zstdm = Zx;
        }

        if(MenuData()==CME_1_KOm)
            of->open_1_KOm.Zstdm = Zx;
        if(MenuData()==CME_10_KOm)
            of->open_10_KOm.Zstdm = Zx;
        if(MenuData()==CME_100_KOm)
            of->open_10_KOm_Current_Boost.Zstdm = Zx;

        if(MenuData()==CME_OPEN)
        {
            if(ResistorCurrent()==Resistor_100_Om)
                of->open_100_Om.Zom = Zx;
            if(ResistorCurrent()==Resistor_1_KOm)
                of->open_1_KOm.Zom = Zx;
            if(ResistorCurrent()==Resistor_10_KOm)
                of->open_10_KOm.Zom = Zx;
            if(ResistorCurrent()==Resistor_10_KOm_Current_Boost)
                of->open_10_KOm_Current_Boost.Zom = Zx;
        }
    }

    current_index++;
    if(current_index%FREQ_INDEX_MAX==0 && MenuIndex()==CME_SHORT)
    {
        int r = current_index/FREQ_INDEX_MAX;
        if(r==0)
            TaskSetDefaultResistor(Resistor_100_Om);
        if(r==1)
            TaskSetDefaultResistor(Resistor_100_Om_Voltage_Boost);
    }

    if(current_index%FREQ_INDEX_MAX==0 && MenuIndex()==CME_OPEN)
    {
        int r = current_index/FREQ_INDEX_MAX;
        if(r==0)
            TaskSetDefaultResistor(Resistor_100_Om);
        if(r==1)
            TaskSetDefaultResistor(Resistor_1_KOm);
        if(r==2)
            TaskSetDefaultResistor(Resistor_10_KOm);
        if(r==3)
            TaskSetDefaultResistor(Resistor_10_KOm_Current_Boost);
    }

    if(current_index>=end_index)
    {
        TaskSetDefaultResistor(Resistor_Auto);
        ProgressSetVisible(false);
        if(MenuData()==CME_SHORT)
            MenuSetNameAndUpdate(MenuIndexByData(CME_SHORT), menu_names_ok[CME_SHORT]);
        if(MenuData()==CME_OPEN)
            MenuSetNameAndUpdate(MenuIndexByData(CME_OPEN), menu_names_ok[CME_OPEN]);
        if(MenuData()==CME_10_Om)
            MenuSetNameAndUpdate(MenuIndexByData(CME_10_Om), menu_names_ok[CME_10_Om]);
        if(MenuData()==CME_100_Om)
            MenuSetNameAndUpdate(MenuIndexByData(CME_100_Om), menu_names_ok[CME_100_Om]);
        if(MenuData()==CME_1_KOm)
            MenuSetNameAndUpdate(MenuIndexByData(CME_1_KOm), menu_names_ok[CME_1_KOm]);
        if(MenuData()==CME_10_KOm)
            MenuSetNameAndUpdate(MenuIndexByData(CME_10_KOm), menu_names_ok[CME_10_KOm]);
        if(MenuData()==CME_100_KOm)
            MenuSetNameAndUpdate(MenuIndexByData(CME_100_KOm), menu_names_ok[CME_100_KOm]);
        calibrated[MenuData()] = true;
        return;
    }

    ProgressSetPos(current_index/(float)end_index);
    TaskSetFreq(StandartFreq(current_index%FREQ_INDEX_MAX));
}
