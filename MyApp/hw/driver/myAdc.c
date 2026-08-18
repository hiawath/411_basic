#include "myAdc.h"
#include "adc.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_ll_adc.h"
#include <stdint.h>

ADC_ChannelConfTypeDef sConfig={0};
uint32_t Adc_Ch0(void){
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 100);
  uint32_t temp_Adc=HAL_ADC_GetValue(&hadc1);

  HAL_ADC_Stop(&hadc1);
  return temp_Adc;
}
uint32_t Adc_Ch1(void){
    sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 100);
  uint32_t temp_Adc=HAL_ADC_GetValue(&hadc1);

  HAL_ADC_Stop(&hadc1);
  return temp_Adc;
}
uint32_t Adc_Ch4(void){
    sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 100);
  uint32_t temp_Adc=HAL_ADC_GetValue(&hadc1);

  HAL_ADC_Stop(&hadc1);
  return temp_Adc;
}