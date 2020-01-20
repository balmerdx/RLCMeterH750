#include "main.h"
#include "menu.h"

#include "scene_get_float.h"

static bool g_ok = false;
static float g_value = 0;
static float g_mul_factor;
static int g_digits;
static int g_places;
static int g_cur_digit;
static int g_pow10;
static SceneStartHandler g_prev_scene;
static char g_menu_name[STATUSBAR_STR_LEN];

static void SceneGetFloatQuant();

static void InitMenu(int top_min, int top_max, int digits, int pow10)
{
    if(top_min<0)
        top_min = 0;
    if(top_max>9)
        top_max=9;

    float mul_factor = 1;
    if(pow10>0)
    {
        for(int i=0; i<pow10; i++)
            mul_factor *= 10.f;
    }

    if(pow10<0)
    {
        for(int i=0; i<-pow10; i++)
            mul_factor *= 0.1f;
    }

    g_mul_factor = mul_factor;

    char data[30];
    MenuReset(g_menu_name);
    for(int i=top_min; i<=top_max; i++)
    {
        floatToString(data, sizeof(data), g_value+i*mul_factor, g_places, digits+1, true);
        MenuAdd(data, i);
    }

    MenuAdd("Cancel", -1);
    if(g_cur_digit==0)
        MenuSetIndex(MenuIndexByData(top_max));
    MenuRedraw();
}

void SceneGetFloatStart(const char* menu_name, int top_min, int top_max, int digits, int pow10, SceneStartHandler prev_scene)
{
    strncpy(g_menu_name, menu_name, STATUSBAR_STR_LEN-1);
    g_ok = false;
    g_value = 0;
    g_digits = digits;
    g_cur_digit = 0;
    g_pow10 = pow10;
    g_prev_scene = prev_scene;
    g_places = digits-pow10-1;

    InitMenu(top_min, top_max, digits, pow10);

    InterfaceGoto(SceneGetFloatQuant);
}

void SceneGetFloatQuant()
{
    MenuQuant();
    if(!EncButtonPressed())
        return;

    int d = MenuData();
    if(d<0)
    {
        g_prev_scene();
        return;
    }

    g_value += d*g_mul_factor;
    g_pow10--;
    g_cur_digit++;

    if(g_cur_digit==g_digits)
    {
        g_ok = true;
        g_prev_scene();
    } else {
        InitMenu(0, 9, g_digits, g_pow10);
    }
}

bool SceneGetFloatOk()
{
    return g_ok;
}

float SceneGetFloat()
{
    return g_value;
}
