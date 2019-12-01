#include "main.h"
#include <string.h>
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "hardware/quadrature_encoder.h"
#include "ili/UTFT.h"
#include "ili/DefaultFonts.h"
#include "hardware/AD9833_driver.h"
#include "hardware/dual_adc.h"
#include "hardware/select_resistor.h"
#include "data_processing.h"
#include "measure/measure_freq.h"
#include "measure/sin_cos.h"
#include "measure/calculate_rc.h"

extern uint32_t received_bytes;

volatile int64_t g_sum;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

bool SendAdcBuffer()
{
    const uint32_t packet_size_ints = 500;

    for(uint32_t pos = 0; pos<ADC_BUFFER_SIZE; pos += packet_size_ints)
    {
        uint32_t size = packet_size_ints;
        if(pos+size > ADC_BUFFER_SIZE)
            size = ADC_BUFFER_SIZE-pos;
        uint8_t result;
        for(int i=0; i<10; i++)
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

void SwitchToResistor(ResistorSelectorEnum r)
{
    if(ResistorCurrent()==r)
        return;
    ResistorSelect(r);
    HAL_Delay(10);
}

void SelectResistor(const ConvolutionResult* result, float measured_impedance)
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

    SwitchToResistor(select_resistor);

}

int StandartFreq(int idx)
{
//Сначала должны идти min_idx с меньшими значениями, а потом с большими.
#define R(min_idx, mul_idx) if(idx<(min_idx)+10) return (idx+1-(min_idx))*(mul_idx)
    if(idx<-20)
        return 0;
    R(-20, 10);
    R(-10, 100);
    R(0, 1000);
    R(10, 10000);
    R(20, 100000);
#undef R
    return idx*1000;
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USB_DEVICE_Init();

    QuadEncInit();

    static char buffer_cdc[128];

    SCB_EnableICache();
    SCB_EnableDCache();

    SinCosInit();
    ResistorSelectorInit();

    UTFT_InitLCD(UTFT_LANDSCAPE2);
    UTFT_fillScrW(VGA_BLACK);
    UTFT_setColorW(VGA_GREEN);
    UTFT_fillRect(10,10, 20,20);

    UTFT_setFont(BigFont);
    UTFT_setColorW(VGA_WHITE);
    AD9833_Init();
    UTFT_print("AD9833_Init", 20, 30);

    DualAdcInitAndStart();
    UTFT_print("ADC Started    ", 20, 30);

    uint32_t freqWord = 0;
    uint32_t convolution_time_ms = 50;

    uint16_t old_enc_value = 1234;
    bool old_enc_button = false;
    bool first = true;
    while (1)
    {
        uint16_t enc_value = QuadEncValue();
        bool enc_button = QuadEncButton();

        if(enc_value!=old_enc_value || enc_button!=old_enc_button)
        {
            old_enc_value = enc_value;
            old_enc_button = enc_button;

            int freq = StandartFreq((int16_t)enc_value);
            sprintf(buffer_cdc, "F=%i    ", freq);
            UTFT_print(buffer_cdc, 20, 50);
            freqWord = AD9833_CalcFreqWorld(freq);
            AD9833_SetFreqWorld(freqWord);

            if(first)
            {
                first = false;
            } else
            {
                HAL_Delay(2);
                AdcStartConvolution(freqWord, convolution_time_ms);
            }
        }

        if(AdcConvolutionComplete())
        {
            static ConvolutionResult result;
            result = AdcConvolutionResult();

            complex Zx;
            calculate(&result, &Zx);

            strcpy(buffer_cdc, "abs(a)=");
            float abs_a = cabs(result.sum_a);
            floatToString(buffer_cdc+strlen(buffer_cdc), 20, abs_a, 4, 10, false);
            UTFT_print(buffer_cdc, 20, 90);

            strcpy(buffer_cdc, "abs(b)=");
            floatToString(buffer_cdc+strlen(buffer_cdc), 20, cabs(result.sum_b), 4, 10, false);
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

            SelectResistor(&result, cabs(Zx));

            AdcStartConvolution(freqWord, convolution_time_ms);
        }

        HAL_Delay(250);

        if(received_bytes)
        {
            received_bytes = 0;
            //По любым приходящем запросам - отсылаем в ответ буффер с данными.
            AdcStartBufferFilling();

        }

        if(AdcBufferFillingComplete())
        {
            AdcClearBufferFillingComplete();
            SendAdcBuffer();
        }
    }
}


/**
  * @brief System Clock Configuration
  * @retval None
  *            System Clock source            = PLL (HSE BYPASS)
  *            SYSCLK(Hz)                     = 400 000 000 (CPU Clock)
  *            HCLK(Hz)                       = 200 000 000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 25 000 000
  *     48 MHz USB Clock
  *     88 MHz ADC Clock
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    // Supply configuration update enable
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    // Configure the main internal regulator output voltage
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
    // Macro to configure the PLL clock source
    __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 5;
    RCC_OscInitStruct.PLL.PLLN = 160;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    // Initializes the CPU, AHB and APB busses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
    PeriphClkInitStruct.PLL2.PLL2M = 25;
    PeriphClkInitStruct.PLL2.PLL2N = 176;
    PeriphClkInitStruct.PLL2.PLL2P = 2;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 2;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.PLL3.PLL3M = 25;
    PeriphClkInitStruct.PLL3.PLL3N = 192;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 4;
    PeriphClkInitStruct.PLL3.PLL3R = 2;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    //Enable USB Voltage detector
    HAL_PWREx_EnableUSBVoltageDetector();
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
