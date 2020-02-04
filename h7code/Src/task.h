#pragma once

#include "measure/corrector.h"
#include "data_processing.h"
#include "measure/calculate_rc.h"

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
void TaskSetFreq(double freq);
int TaskGetFreq();

//По умолчанию используется Resistor_Auto, чтобы автоматом
//выбирался резистор. Но для калибровки надо его задавать явно.
void TaskSetDefaultResistor(ResistorSelectorEnum r);
ResistorSelectorEnum TaskGetDefaultResistor();

//Запускает работу ADC
//Как правило вызова этой функции не требуется,
//Дальнейшие запуски происходят автоматически.
void TaskStartConvolution();

void TaskQuant();

bool IsUsbCommand();

//Результат последней конверсии.
extern ConvolutionResult g_result;
//Результат получившийся при измерении (без коррекции)
extern complex g_Zxm;

//Приблизительная ошибка, получишаяся при последнем измерении.
extern ErrorZx g_error;

//Результат получившийся после применения коррекции
extern complex g_Zx;

extern bool g_enable_correction;
