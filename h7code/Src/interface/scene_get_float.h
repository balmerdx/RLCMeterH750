#pragma once
#include "interface.h"

//top_min, top_max - цифры от 0 до 9 разрешенные для этого знака.
//digits - количество знаков которые требуется ввести
//pow10 - старший разряд.
// pow10==0 - значит старший разряд это единицы.
// pow10==1 - значит старший разряд это десятки.
// pow10==-1 - значит старший разряд это десятые доли.

void SceneGetFloatStart(const char* menu_name, int top_min, int top_max, int digits, int pow10, SceneStartHandler prev_scene);

//Возвращает true если введено корректное число.
bool SceneGetFloatOk();

//Возвращает получившиеся число
float SceneGetFloat();
