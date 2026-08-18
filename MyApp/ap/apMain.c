#include "apMain.h"
#include "myAdc.h"
#include "myUart.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"


#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;
void apInit(void) { uartInit(); }

uint32_t adc_multi_values[3] = {0, 0, 0};

void apMain(void) {

  while (1) {
      HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_multi_values[0]=HAL_ADC_GetValue(&hadc1);

    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_multi_values[1]=HAL_ADC_GetValue(&hadc1);

    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_multi_values[2]=HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);


    HAL_Delay(500);
  }
}