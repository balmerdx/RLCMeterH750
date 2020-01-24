#include "main.h"
#include "task.h"
#include "ili/UTFT.h"
#include "ili/DefaultFonts.h"
#include "hardware/AD9833_driver.h"
#include "hardware/dual_adc.h"
#include "hardware/select_resistor.h"
#include "data_processing.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "measure/calculate_rc.h"
#include "interface/scene_single_freq.h"
#include "interface/scene_calibration.h"
#include "interface/scene_graph.h"

#define USB_COMMAND_CONVOLUTION 0xABDE0001u
#define USB_COMMAND_ADC 0xABDE0002u

uint32_t convolution_time_ms_slow = 200;
uint32_t g_freqWord = 0;
ConvolutionResult g_result;
complex g_Zxm = 0;
complex g_Zx = 0;
ErrorZx g_error = {};
bool g_enable_correction = true;

#define USB_RECEIVED_DATA_SIZE 32
//Количество слов в буфере g_usb_received_data
static volatile uint32_t g_usb_received_words = 0;
//Смещение в буфере, до которого дочитали
static volatile uint32_t g_usb_received_offset = 0;
static uint32_t g_usb_received_data[USB_RECEIVED_DATA_SIZE];

static volatile bool force_next_task = false;
static uint32_t last_command = 0;

static int g_freq = 0;
static ResistorSelectorEnum g_choose_resistor = Resistor_Auto;
static bool g_changed_some = false;


void OnReceiveUsbData(uint8_t* Buf, uint32_t Len)
{
    const uint32_t sz = sizeof(g_usb_received_data[0]);
    while(g_usb_received_words < USB_RECEIVED_DATA_SIZE && Len>=sz)
    {
        memcpy(g_usb_received_data+g_usb_received_words, Buf, sz);
        Buf += sz;
        Len -= sz;
        g_usb_received_words++;
    }
}

bool SendAdcBuffer()
{
    const uint32_t packet_size_ints = 500;

    for(uint32_t pos = 0; pos<ADC_BUFFER_SIZE; pos += packet_size_ints)
    {
        uint32_t size = packet_size_ints;
        if(pos+size > ADC_BUFFER_SIZE)
            size = ADC_BUFFER_SIZE-pos;
        uint8_t result;
        for(int i=0; i<1000; i++)
        {
            result = CDC_Transmit_FS((uint8_t*)(adc_cpu_buffer+pos), size*sizeof(uint32_t));
            if(result!=USBD_BUSY)
                break;
            HAL_Delay(1);
        }

        if(result!=USBD_OK)
            return false;
    }

    return true;
}

void SendConvolutionResult(complex Zx)
{
    double x[] = {creal(Zx), cimag(Zx)};
    CDC_Transmit_FS((uint8_t*)(x), sizeof(x));
}

//return true if resistir changed
bool SelectResistor(const ConvolutionResult* result, float measured_impedance)
{
    if(g_choose_resistor != Resistor_Auto)
        return false;

    float abs_a = cabs(result->sum_a);
    float abs_b = cabs(result->sum_b);

    //Слишком большая амплитуда, надо переключиться на резистор меньших значений.
    ResistorSelectorEnum max_resistor = Resistor_10_KOm_Current_Boost;
    ResistorSelectorEnum select_resistor = Resistor_100_Om;
    if(abs_a < 2000 && abs_b < 2000)
    {
        max_resistor = Resistor_100_Om;
    } else
    if(abs_b > 15000)
    {
        if(ResistorCurrent()==Resistor_10_KOm)
            max_resistor = Resistor_1_KOm;
        if(ResistorCurrent()==Resistor_1_KOm)
            max_resistor = Resistor_100_Om;
    }

    if(measured_impedance < 15)
    {
        select_resistor = Resistor_100_Om_Voltage_Boost;
    } else
    if(measured_impedance > 90000)
    {
        select_resistor = Resistor_10_KOm_Current_Boost;
    } else
    if(measured_impedance > 9000)
    {
        select_resistor = Resistor_10_KOm;
    } else
    if(measured_impedance > 900)
    {
        select_resistor = Resistor_1_KOm;
    }

    if(select_resistor > max_resistor)
        select_resistor = max_resistor;

    if(ResistorCurrent()==select_resistor)
        return false;

    ResistorSelect(select_resistor);
    HAL_Delay(10);
    return true;
}

void TaskStartConvolution()
{
    if(g_changed_some)
    {
        g_changed_some = false;
        HAL_Delay(2);
    }

    AdcStartConvolution(g_freqWord, convolution_time_ms_slow);
}

void TaskSetDefaultResistor(ResistorSelectorEnum r)
{
    g_choose_resistor = r;
    if(r != Resistor_Auto)
    {
        g_changed_some = true;
        ResistorSelect(r);
    }
}


void TaskSetFreq(double freq)
{
    g_freq = freq;
    g_freqWord = AD9833_CalcFreqWorld(freq);
    AD9833_SetFreqWorld(g_freqWord);
    g_changed_some = true;
}

int TaskGetFreq()
{
    return g_freq;
}

void NextTask()
{
    if(g_usb_received_offset<g_usb_received_words)
    {
        uint32_t command = g_usb_received_data[g_usb_received_offset];
        if(command==USB_COMMAND_CONVOLUTION || command==USB_COMMAND_ADC)
        {
            last_command = command;
            uint32_t freq = g_usb_received_data[g_usb_received_offset+1];
            g_usb_received_offset += 2;

            TaskSetFreq(freq);
            if(command==USB_COMMAND_ADC)
            {
                HAL_Delay(4);
                AdcStartBufferFilling();
                return;
            }

            TaskStartConvolution();
            return;
        }
    }

    g_usb_received_offset = g_usb_received_words = 0;
    last_command = 0;
    TaskStartConvolution();
}

void TaskQuant()
{
    if(AdcConvolutionComplete())
    {
        g_result = AdcConvolutionResult();
        g_Zxm = calculateZxm(&g_result, &g_error);
        if(g_enable_correction)
            g_Zx = CorrectionMake(g_Zxm, ResistorCurrent(), g_freq);
        else
            g_Zx = g_Zxm;

        if(SelectResistor(&g_result, cabs(g_Zxm)))
        {
            TaskStartConvolution();

            //Временный, отладочный код, для того, чтобы увидеть бесконечные переключения резистора.
            SceneSingleFreqZx();
        } else
        {
            if(last_command==USB_COMMAND_CONVOLUTION)
            {
                SendConvolutionResult(g_Zx);
            }
            SceneSingleFreqZx();
            SceneCalibrarionZx(g_Zxm);
            SceneGraphResultZx();
            force_next_task = true;
        }
    }

    if(AdcBufferFillingComplete())
    {
        AdcClearBufferFillingComplete();
        SendAdcBuffer();
        force_next_task = true;
    }

    if(force_next_task)
    {
        force_next_task = false;
        NextTask();
    }
}

