#include "apMain.h"
#include "myAdc.h"
#include "myUart.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"


#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;
void apInit(void) { 
  uartInit();
  HAL_ADC_Start_IT(&hadc1);
}

uint32_t adc_multi_values[3] = {0, 0, 0};

void apMain(void) {

  while (1) {
    HAL_Delay(10);
  }
}