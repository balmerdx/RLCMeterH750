#pragma once

void QuadEncInit();
int16_t QuadEncValue();


//return true если кнопка в текущий момент нажата.
bool QuadEncButton();

//return true если кнопка нажалась.
//последующие взовы этой функции будут возвращать false
//пока кнопку не отпустят и не нажмут заново.
bool EncButtonPressed();
