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

#define USB_COMMAND_CONVOLUTION 0xABDE0001u
#define USB_COMMAND_ADC 0xABDE0002u

uint32_t convolution_time_ms_slow = 200;
uint32_t convolution_time_ms_fast = 50;
static char buffer_cdc[128];
uint32_t g_freqWord = 0;

#define USB_RECEIVED_DATA_SIZE 32
//Количество слов в буфере g_usb_received_data
static volatile uint32_t g_usb_received_words = 0;
//Смещение в буфере, до которого дочитали
static volatile uint32_t g_usb_received_offset = 0;
static uint32_t g_usb_received_data[USB_RECEIVED_DATA_SIZE];

static volatile bool force_next_task = false;
static uint32_t last_command = 0;

int g_freq = 0;
int g_freq_index = 18;


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
    float abs_b = cabs(result->sum_b);

    //Слишком большая амплитуда, надо переключиться на резистор меньших значений.
    ResistorSelectorEnum max_resistor = Resistor_10_KOm;
    ResistorSelectorEnum select_resistor = Resistor_100_Om;
    if(abs_b > 15000)
    {
        if(ResistorCurrent()==Resistor_10_KOm)
            max_resistor = Resistor_1_KOm;
        if(ResistorCurrent()==Resistor_1_KOm)
            max_resistor = Resistor_100_Om;
    }

    if(measured_impedance > 3000)
    {
        select_resistor = Resistor_10_KOm;
    } else
    if(measured_impedance > 300)
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

static void SetFreq(int freq)
{
    //sprintf(buffer_cdc, "F=%i    ", freq);
    //UTFT_print(buffer_cdc, 20, 50);
    g_freqWord = AD9833_CalcFreqWorld(freq);
    AD9833_SetFreqWorld(g_freqWord);
    HAL_Delay(2);

}

static void StartConvolution()
{
    AdcStartConvolution(g_freqWord, last_command?convolution_time_ms_fast:convolution_time_ms_slow);
}

void TaskSetFreq(int freq)
{
    g_freq = freq;
    SetFreq(freq);
    StartConvolution();
}

int TaskGetFreq()
{
    return g_freq;
}

void DrawResult(ConvolutionResult* result, complex Zx)
{
    strcpy(buffer_cdc, "abs(a)=");
    float abs_a = cabs(result->sum_a);
    floatToString(buffer_cdc+strlen(buffer_cdc), 20, abs_a, 4, 10, false);
    UTFT_print(buffer_cdc, 20, 90);

    strcpy(buffer_cdc, "abs(b)=");
    floatToString(buffer_cdc+strlen(buffer_cdc), 20, cabs(result->sum_b), 4, 10, false);
    UTFT_print(buffer_cdc, 20, 110);

    strcpy(buffer_cdc, "RE=");
    floatToString(buffer_cdc+strlen(buffer_cdc), 20, creal(Zx), 4, 10, false);
    UTFT_print(buffer_cdc, 20, 130);

    strcpy(buffer_cdc, "IM=");
    floatToString(buffer_cdc+strlen(buffer_cdc), 20, cimag(Zx), 4, 10, false);
    UTFT_print(buffer_cdc, 20, 150);

    if(ResistorCurrent()==Resistor_100_Om)
        strcpy(buffer_cdc, "R=100 Om");
    if(ResistorCurrent()==Resistor_1_KOm)
        strcpy(buffer_cdc, "R=1 KOm");
    if(ResistorCurrent()==Resistor_10_KOm)
        strcpy(buffer_cdc, "R=10 KOm");
    UTFT_print(buffer_cdc, 20, 180);
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

            SetFreq(freq);
            if(command==USB_COMMAND_ADC)
            {
                AdcStartBufferFilling();
                return;
            }

            StartConvolution();
            return;
        }
    }

    g_usb_received_offset = g_usb_received_words = 0;
    last_command = 0;
    StartConvolution();
}

void TaskQuant()
{
    if(AdcConvolutionComplete())
    {
        static ConvolutionResult result;
        result = AdcConvolutionResult();

        complex Zx;
        calculate(&result, &Zx);

        //DrawResult(&result, Zx);
        if(SelectResistor(&result, cabs(Zx)))
        {
            StartConvolution();
        } else
        {
            if(last_command==USB_COMMAND_CONVOLUTION)
            {
                SendConvolutionResult(Zx);
            }
            SceneSingleFreqZx(Zx);
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

