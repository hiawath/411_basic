#include "apMain.h"
#include "myAdc.h"
#include "myUart.h"
#include "myDht11.h"
#include "stm32f411xe.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"


#include <stdint.h>
#include <stdio.h>
#include <string.h>


extern ADC_HandleTypeDef hadc1;
void apInit(void) { 
  uartInit();
  adcInit();
  dht11Init();
  
}

float internal_temp=0;
dht11Data_t dht_data={0};
bool dht_status=false;
void apMain(void) {

  while (1) {
    adcUpdate();

    dht_status=dht11Read(&dht_data);

    internal_temp=adcGetTemp();
    HAL_Delay(10);
  }
}