#include "myAdc.h"

/* STM32F411 내부 온도 센서 특성 파라미터 (데이터시트 기준) */
#define V25_MV       760.0f  /* 25도에서의 전압: 약 0.76V (760mV) */
#define AVG_SLOPE    2.5f    /* 기울기: 2.5 mV/°C */
#define VREF_MV      3300.0f /* ADC 기준 전압: 3.3V (3300mV) */
#define ADC_MAX_VAL  4095.0f /* 12비트 ADC 해상도 */

void adcInit(void)
{
  // 필요한 ADC 추가 초기화
}

/**
  * @brief  폴링 방식으로 ADC1의 Raw 값을 읽어 반환
  * @retval 12비트 ADC 변환 결과값 (0 ~ 4095)
  */
uint32_t adcReadRaw(void)
{
  uint32_t adc_val = 0;

  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
  {
    adc_val = HAL_ADC_GetValue(&hadc1);
  }
  HAL_ADC_Stop(&hadc1);

  return adc_val;
}

/**
  * @brief  내부 온도 센서 ADC 값을 읽어 섭씨 온도(°C)로 환산
  * @retval 섭씨 온도 (°C)
  */
float adcReadTemp(void)
{
  uint32_t raw = adcReadRaw();
  float vsense_mv = ((float)raw * VREF_MV) / ADC_MAX_VAL;
  float temperature = ((vsense_mv - V25_MV) / AVG_SLOPE) + 25.0f;

  return temperature;
}
