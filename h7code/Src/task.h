#pragma once

#include "measure/corrector.h"
#include "data_processing.h"

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

//Устанавливает частоту генератора.
void TaskSetFreq(int freq);
int TaskGetFreq();

//По умолчанию используется Resistor_Auto, чтобы автоматом
//выбирался резистор. Но для калибровки надо его задавать явно.
void TaskSetDefaultResistor(ResistorSelectorEnum r);

//Запускает работу ADC
//Как правило вызова этой функции не требуется,
//Дальнейшие запуски происходят автоматически.
void TaskStartConvolution();

void TaskQuant();

//Индекс, для StandartFreq функции
extern int g_freq_index;

//Результат последней конверсии.
extern ConvolutionResult g_result;
extern complex g_Zxm;
