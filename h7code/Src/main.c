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
#include "hardware/m25p16.h"
#include "hardware/delay.h"
#include "data_processing.h"
#include "measure/measure_freq.h"
#include "measure/sin_cos.h"
#include "task.h"
#include "interface/scene_single_freq.h"

volatile int64_t g_sum;
extern volatile SPI_HandleTypeDef SpiHandle;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    HAL_Init();

    SystemClock_Config();

    DelayInit();
    MX_GPIO_Init();
    MX_USB_DEVICE_Init();

    QuadEncInit();

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

    m25p16_init();


    if(!m25p16_read_ram_id_and_check())
    {
        UTFT_print("Flash check FAIL", 20, 30);
        Error_Handler();
    }

    if(0)
    {
        m25p16_ram_erase64k(0x10000);

        uint32_t offset = 0x10000;
        uint16_t size = 128;
        static char buffer[128];
        for(int i=0; i<size; i++)
            buffer[i] = i;

        m25p16_write(offset, size, buffer);

        sprintf(buffer, "Flash write complete");
        UTFT_print(buffer, 20, 50);

        m25p16_read(offset, size, buffer);

        bool ok = true;
        for(int i=0; i<size; i++)
            if(buffer[i] != i)
                ok = false;

        if(ok)
            sprintf(buffer, "Flash read OK      ");
        else
            sprintf(buffer, "Flash read FAIL      ");
        UTFT_print(buffer, 20, 50);
        while(1);
    }

    DualAdcInitAndStart();
    UTFT_print("ADC Started    ", 20, 30);

    correctionLoad(0);

    InterfaceStart();
    TaskSetFreq(StandartFreq(g_freq_index));
    TaskStartConvolution();
    SceneSingleFreqStart();

    while (1)
    {
        TaskQuant();
        InterfaceQuant();
        HAL_Delay(1);
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

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB|RCC_PERIPHCLK_SPI3;
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
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
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
