#pragma once

#include "measure/corrector.h"

typedef enum TASK_ENUM
{
    //По умолчанию постоянно суммируем данные и потом выводим результат на экран.
    TASK_DEFAULT_CONVOLUTION = 0,

    //Устанавливаем частоту, которую требует компьютер
    //Результатом является Z (пока без калибровки)
    //Результат передается по USB обратно на компьютер
    TASK_USB_SET_FREQ_AND_RECEIVE_CONVOLUTION,

    //Устанавливаем частоту, которую требует компьютер и результат передаем по USB
    //Результатом является содержимое отсэмплированного буфера ADC
    TASK_USB_SET_FREQ_AND_RECEIVE_ADC,
} TASK_ENUM;

void TaskSetFreq(int freq);
int TaskGetFreq();
void TaskQuant();

//Индекс, для StandartFreq функции
extern int g_freq_index;
