#include "apMain.h"
#include "myUart.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;
void apInit(void){
    uartInit();
}

void apMain(void){
    uint32_t adc_single_value=0;
    
    while(1){
        HAL_ADC_Start(&hadc1);

        if(HAL_ADC_PollForConversion(&hadc1, 100)==HAL_OK){
            adc_single_value=HAL_ADC_GetValue(&hadc1);
        }
        HAL_ADC_Stop(&hadc1);
        HAL_Delay(200);


    }
}