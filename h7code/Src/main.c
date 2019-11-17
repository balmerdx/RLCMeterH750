/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "test_loop_speed.h"
#include "hardware/quadrature_encoder.h"
#include "ili/UTFT.h"
#include "ili/DefaultFonts.h"
#include "hardware/AD9833_driver.h"
#include "hardware/dual_adc.h"

extern uint32_t received_bytes;
volatile uint32_t delta_ms;

/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

uint32_t testSpeed()
{
	int loop_count = 10000;
	initLoop();

	uint32_t start = HAL_GetTick();
	for(int i=0; i<loop_count; i++)
		speedLoop();
	uint32_t end = HAL_GetTick();

	return end-start;
}

#define ADC1_DATA(x)   ((x) & 0x0000FFFF)
#define ADC2_DATA(x)   ((x) >> 16)

float d1_errf = 0;
float d2_errf = 0;

void AdcConvertDataCallback(uint32_t* data, uint32_t size)
{
    int32_t d1_mid = 0;
    int32_t d2_mid = 0;
    for(uint32_t i=0; i<size; i++)
    {
        uint32_t d = data[i];
        d1_mid += ADC1_DATA(d);
        d2_mid += ADC2_DATA(d);
    }

    d1_mid /= size;
    d2_mid /= size;

    int32_t d1_err = 0;
    int32_t d2_err = 0;
    for(uint32_t i=0; i<size; i++)
    {
        uint32_t d = data[i];
        int32_t d1 = ADC1_DATA(d) - d1_mid;
        int32_t d2 = ADC2_DATA(d) - d2_mid;
        if(d1<0)
            d1 = - d1;
        if(d2<0)
            d2 = - d2;

        d1_err += d1;
        d2_err += d2;
    }

    d1_errf = (float)d1_err/(float)size;
    d2_errf = (float)d2_err/(float)size;
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
    //delta_ms = testSpeed();


    UTFT_InitLCD(UTFT_LANDSCAPE2);
    UTFT_fillScrW(VGA_BLACK);
    UTFT_setColorW(VGA_GREEN);
    UTFT_fillRect(10,10, 20,20);

    UTFT_setFont(BigFont);
    UTFT_setColorW(VGA_WHITE);
    AD9833_Init();
    UTFT_print("AD9833_Init", 20, 30);
    //AD9833_SetFreqWorld(AD9833_CalcFreqWorld(1000));
    //UTFT_print("Set complete", 20, 50);

    DualAdcInitAndStart();
    UTFT_print("ADC Started    ", 20, 30);

    uint16_t old_enc_value = 1234;
    bool old_enc_button = false;
    while (1)
    {
        uint16_t enc_value = QuadEncValue();
        bool enc_button = QuadEncButton();

        if(enc_value!=old_enc_value || enc_button!=old_enc_button)
        {
            old_enc_value = enc_value;
            old_enc_button = enc_button;

            int size_cdc = sprintf(buffer_cdc, "enc=%i s=%i    ", (int)enc_value, enc_button?1:0);
            received_bytes = 0;
            CDC_Transmit_FS((uint8_t*)buffer_cdc, size_cdc);

            UTFT_print(buffer_cdc, 20, 50);
            //uint32_t word = AD9833_CalcFreqWorld(enc_value*100);
            uint32_t word = AD9833_CalcFreqWorld(enc_value*10000);
            //uint32_t word = (1<<enc_value)|4096;
            AD9833_SetFreqWorld(word);
        }

        sprintf(buffer_cdc, "h=%i f=%i      ", half_conv, full_conv);
        UTFT_print(buffer_cdc, 20, 70);

        floatToString(buffer_cdc, 20, d1_errf, 4, 6, false);
        UTFT_print(buffer_cdc, 20, 90);

        floatToString(buffer_cdc, 20, d2_errf, 4, 6, false);
        UTFT_print(buffer_cdc, 20, 110);

        HAL_Delay(250);
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
    __HAL_RCC_GPIOH_CLK_ENABLE();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
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
