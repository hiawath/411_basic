#include "apMain.h"
#include "myLcd1602.h"
#include "myAdc.h"
#include "myUart.h"
#include "myDht11.h"
#include "myI2c.h"
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
  lcd1602Init();
  i2cScan();

 
}

float internal_temp=0;
dht11Data_t dht_data={0};
bool dht_status=false;
void apMain(void) {



  while (1) {
    adcUpdate();
    dht_status=dht11Read(&dht_data);
    internal_temp=adcGetTemp();

    lcd1602Clear();
    lcd1602Cursor(0, 0);
    lcd1602Printf("Temp %.2f/%.2f", internal_temp,dht_data.temperature);
    lcd1602Cursor(1, 0);
    lcd1602Printf("Humi %.2f", dht_data.humidity);

    HAL_Delay(250);
  }
}