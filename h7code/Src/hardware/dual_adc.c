#include "stm32h7xx_hal.h"
#include "dual_adc.h"
void Error_Handler();
void ADC_InitVREFBUF();

// ADC dual mode interleaved conversion results (ADC master and ADC slave results concatenated on data register
ALIGN_32BYTES(__IO uint32_t   aADCDualConvertedValues[ADCCONVERTEDVALUES_BUFFER_SIZE] __attribute__((section(".d1_data"))));
ADC_HandleTypeDef    AdcHandle_master;
ADC_HandleTypeDef    AdcHandle_slave;

volatile int half_conv = 0;
volatile int full_conv = 0;
volatile int adc_overrun = 0;

uint32_t AdcSamplesPerSecond()
{
    return 2000000;
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef          GPIO_InitStruct;
    static DMA_HandleTypeDef  DmaHandle;

    ADC_InitVREFBUF();

    //##-1- Enable peripherals and GPIO Clocks #################################
    // Enable clock of GPIO associated to the peripheral channels
    ADCx_CHANNELa_GPIO_CLK_ENABLE();
    ADCy_CHANNELa_GPIO_CLK_ENABLE();

    // Enable clock of ADCx peripheral
    ADCx_CLK_ENABLE();
    // ADC Periph interface clock configuration
    __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_PLL2);

    if (hadc->Instance == ADCx)
    {
        // Enable clock of DMA associated to the peripheral
        ADCx_DMA_CLK_ENABLE();

        //##-2- Configure peripheral GPIO ##########################################
        // Configure GPIO pins of the selected ADC channels
        GPIO_InitStruct.Pin = ADCx_CHANNELa_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ADCx_CHANNELa_GPIO_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = ADCy_CHANNELa_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ADCy_CHANNELa_GPIO_PORT, &GPIO_InitStruct);

        //##-3- Configure the DMA ##################################################
        // Configure DMA parameters (ADC master)
        DmaHandle.Instance = ADCx_DMA;

        DmaHandle.Init.Request             = DMA_REQUEST_ADC1;
        DmaHandle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        DmaHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
        DmaHandle.Init.MemInc              = DMA_MINC_ENABLE;
        DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;       // Transfer from ADC by word to match with ADC configuration: Dual mode, ADC master contains conversion results on data register (32 bits) of ADC master and ADC slave
        DmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;       // Transfer to memory by word to match with buffer variable type: word
        DmaHandle.Init.Mode                = DMA_CIRCULAR;              // DMA in circular mode to match with ADC configuration: DMA continuous requests
        DmaHandle.Init.Priority            = DMA_PRIORITY_HIGH;

        // Deinitialize  & Initialize the DMA for new transfer
        HAL_DMA_DeInit(&DmaHandle);
        HAL_DMA_Init(&DmaHandle);

        // Associate the initialized DMA handle to the ADC handle
        __HAL_LINKDMA(hadc, DMA_Handle, DmaHandle);

        //##-4- Configure the NVIC #################################################

        // NVIC configuration for DMA interrupt (transfer completion or error)
        // Priority: high-priority
        HAL_NVIC_SetPriority(ADCx_DMA_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(ADCx_DMA_IRQn);
    }

    // NVIC configuration for ADC interrupt
    // Priority: high-priority
    HAL_NVIC_SetPriority(ADCx_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADCx_IRQn);
}

/**
  * @brief ADC MSP de-initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable clock of ADC peripheral
  *          - Revert GPIO associated to the peripheral channels to their default state
  *          - Revert DMA associated to the peripheral to its default state
  *          - Revert NVIC associated to the peripheral interruptions to its default state
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
    //##-1- Reset peripherals ##################################################
    ADCx_FORCE_RESET();
    ADCx_RELEASE_RESET();

    //##-2- Disable peripherals and GPIO Clocks ################################
    // De-initialize GPIO pin of the selected ADC channel
    HAL_GPIO_DeInit(ADCx_CHANNELa_GPIO_PORT, ADCx_CHANNELa_PIN);
    HAL_GPIO_DeInit(ADCy_CHANNELa_GPIO_PORT, ADCy_CHANNELa_PIN);

    //##-3- Disable the DMA ####################################################
    // De-Initialize the DMA associated to the peripheral
    if(hadc->DMA_Handle != NULL)
    {
        HAL_DMA_DeInit(hadc->DMA_Handle);
    }

    //##-4- Disable the NVIC ###################################################
    // Disable the NVIC configuration for DMA interrupt
    HAL_NVIC_DisableIRQ(ADCx_DMA_IRQn);

    // Disable the NVIC configuration for ADC interrupt
    HAL_NVIC_DisableIRQ(ADCx_IRQn);
}


static void ADC_Config(void)
{
    ADC_ChannelConfTypeDef   sConfig;
    ADC_MultiModeTypeDef     MultiModeInit;

    // Configuration of ADC (master) init structure: ADC parameters and regular group
    AdcHandle_master.Instance = ADCx;

    if (HAL_ADC_DeInit(&AdcHandle_master) != HAL_OK)
    {
        Error_Handler();
    }

    AdcHandle_slave.Instance = ADCy;
    if (HAL_ADC_DeInit(&AdcHandle_slave) != HAL_OK)
    {
        Error_Handler();
    }

    AdcHandle_master.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV4;          // Asynchronous clock mode, input ADC clock divided by 4
    AdcHandle_master.Init.Resolution            = ADC_RESOLUTION_16B;            // 16-bit resolution for converted data
    AdcHandle_master.Init.ScanConvMode          = DISABLE;                       // Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1)
    AdcHandle_master.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;           // EOC flag picked-up to indicate conversion end
    AdcHandle_master.Init.LowPowerAutoWait      = DISABLE;                       // Auto-delayed conversion feature disabled
    AdcHandle_master.Init.ContinuousConvMode    = ENABLE;                        // Continuous mode to have maximum conversion speed (no delay between conversions)
    AdcHandle_master.Init.NbrOfConversion       = 1;                             // Parameter discarded because sequencer is disabled
    AdcHandle_master.Init.DiscontinuousConvMode = DISABLE;                       // Parameter discarded because sequencer is disabled
    AdcHandle_master.Init.NbrOfDiscConversion   = 1;                             // Parameter discarded because sequencer is disabled
    AdcHandle_master.Init.ExternalTrigConv      = ADC_SOFTWARE_START;            // Software start to trigger the 1st conversion manually, without external event
    AdcHandle_master.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE; // Parameter discarded because trigger of conversion by software start (no external event)
    AdcHandle_master.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR; // DMA circular mode selected
    AdcHandle_master.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;      // DR register is overwritten with the last conversion result in case of overrun
    AdcHandle_master.Init.OversamplingMode      = DISABLE;                       // No oversampling

    if (HAL_ADC_Init(&AdcHandle_master) != HAL_OK)
    {
        Error_Handler();
    }

    // Configuration of ADC (slave) init structure: ADC parameters and regular group
    AdcHandle_slave.Instance = ADCy;

    // Same configuration as ADC master, with continuous mode and external
    // trigger disabled since ADC master is triggering the ADC slave
    // conversions
    AdcHandle_slave.Init = AdcHandle_master.Init;
    AdcHandle_slave.Init.ContinuousConvMode    = DISABLE;
    AdcHandle_slave.Init.ExternalTrigConv      = ADC_SOFTWARE_START;

    if (HAL_ADC_Init(&AdcHandle_slave) != HAL_OK)
    {
        Error_Handler();
    }


    // Configuration of channel on ADC (master) regular group on sequencer rank 1
    // Note: Considering IT occurring after each number of
    //       "ADCCONVERTEDVALUES_BUFFER_SIZE" ADC conversions (IT by DMA end
    //       of transfer), select sampling time and ADC clock with sufficient
    //       duration to not create an overhead situation in IRQHandler.
    sConfig.Rank         = ADC_REGULAR_RANK_1;          // Rank of sampled channel number ADCx_CHANNEL
    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;    // Minimum sampling time
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;            // Single-ended input channel
    sConfig.OffsetNumber = ADC_OFFSET_NONE;             // No offset subtraction
    sConfig.Offset = 0;                                 // Parameter discarded because offset correction is disabled

    sConfig.Channel      = ADCx_CHANNELa;               // Sampled channel number
    if (HAL_ADC_ConfigChannel(&AdcHandle_master, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sConfig.Channel = ADCy_CHANNELa;
    if (HAL_ADC_ConfigChannel(&AdcHandle_slave, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    // Run the ADC calibration in single-ended mode
    if (HAL_ADCEx_Calibration_Start(&AdcHandle_master, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_ADCEx_Calibration_Start(&AdcHandle_slave, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    }

    // Configuration of multimode
    // Multimode parameters settings and set ADCy (slave) under control of
    // ADCx (master).
    MultiModeInit.Mode = ADC_DUALMODE_REGSIMULT;
    MultiModeInit.DualModeData = ADC_DUALMODEDATAFORMAT_32_10_BITS;  // ADC and DMA configured in resolution 32 bits to match with both ADC master and slave resolution
    MultiModeInit.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_1CYCLE;

    if (HAL_ADCEx_MultiModeConfigChannel(&AdcHandle_master, &MultiModeInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  Conversion complete callback in non blocking mode
  * @param  AdcHandle : ADC handle
  * @note   This example shows a simple way to report end of conversion
  *         and get conversion result. You can add your own implementation.
  * @note   When ADC_TRIGGER_FROM_TIMER is disabled, conversions are software-triggered
  *         and are too fast for DMA post-processing. Therefore, to reduce the computational
  *         load, the output buffer filled up by the DMA is post-processed only when
  *         ADC_TRIGGER_FROM_TIMER is enabled.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle)
{
    // Invalidate Data Cache to get the updated content of the SRAM on the second half of the ADC converted data buffer: 32 bytes
    SCB_InvalidateDCache_by_Addr((uint32_t *) &aADCDualConvertedValues[ADCCONVERTEDVALUES_BUFFER_SIZE/2], 4*ADCCONVERTEDVALUES_BUFFER_SIZE/2);

    //Use aADCDualConvertedValues  for (tmp_index = (ADCCONVERTEDVALUES_BUFFER_SIZE/2); tmp_index < ADCCONVERTEDVALUES_BUFFER_SIZE; tmp_index++)
    AdcConvertDataCallback((uint32_t *)aADCDualConvertedValues+ADCCONVERTEDVALUES_BUFFER_SIZE/2, ADCCONVERTEDVALUES_BUFFER_SIZE/2);

    full_conv++;
}

/**
  * @brief  Conversion DMA half-transfer callback in non blocking mode
  * @param  hadc: ADC handle
  * @note   When ADC_TRIGGER_FROM_TIMER is disabled, conversions are software-triggered
  *         and are too fast for DMA post-processing. Therefore, to reduce the computational
  *         load, the output buffer filled up by the DMA is post-processed only when
  *         ADC_TRIGGER_FROM_TIMER is enabled.
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    // Invalidate Data Cache to get the updated content of the SRAM on the first half of the ADC converted data buffer: 32 bytes
    SCB_InvalidateDCache_by_Addr((uint32_t *) &aADCDualConvertedValues[0], 4*ADCCONVERTEDVALUES_BUFFER_SIZE/2);

    //Use aADCDualConvertedValues for (tmp_index = 0; tmp_index < (ADCCONVERTEDVALUES_BUFFER_SIZE/2); tmp_index++)
    AdcConvertDataCallback((uint32_t *)aADCDualConvertedValues, ADCCONVERTEDVALUES_BUFFER_SIZE/2);

    half_conv++;
}

/**
  * @brief  ADC error callback in non blocking mode
  *        (ADC conversion with interruption or transfer by DMA)
  * @param  hadc: ADC handle
  * @note   When ADC_TRIGGER_FROM_TIMER is disabled, conversions are software-triggered
  *         and are too fast for DMA post-processing. Overrun issues are observed and to
  *         avoid ending up in the infinite loop of Error_Handler(), no call to this
  *         latter is done in case of HAL_ADC_ERROR_OVR error.
  * @retval None
  */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    // In case of ADC error, call main error handler
    if (HAL_IS_BIT_CLR(hadc->ErrorCode, HAL_ADC_ERROR_OVR))
    {
        Error_Handler();
    } else {
        hadc->ErrorCode = 0;
        adc_overrun++;
    }
}

static void ADC_Start(void)
{
    // Start ADCx and ADCy multimode conversion on regular group with transfer by DMA
    if (HAL_ADCEx_MultiModeStart_DMA(&AdcHandle_master,
                                     (uint32_t *)aADCDualConvertedValues,
                                      ADCCONVERTEDVALUES_BUFFER_SIZE
                                    ) != HAL_OK)
    {
        Error_Handler();
    }

}

void DualAdcInitAndStart()
{
    ADC_Config();
    ADC_Start();
}

void ADC_InitVREFBUF()
{
    __HAL_RCC_VREF_CLK_ENABLE();
    HAL_SYSCFG_VREFBUF_VoltageScalingConfig(SYSCFG_VREFBUF_VOLTAGE_SCALE1);
    HAL_SYSCFG_VREFBUF_HighImpedanceConfig(SYSCFG_VREFBUF_HIGH_IMPEDANCE_DISABLE);
    if (HAL_SYSCFG_EnableVREFBUF() != HAL_OK)
    {
        Error_Handler();
    }

    HAL_Delay(20);
}
