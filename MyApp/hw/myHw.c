#include "myHw.h"

void hwInit(void)
{
  gpioInit();
  uartInit();
  i2cInit();
  ssd1306Init();
  adcInit();
  ds1302Init();
  dht11Init();
  hcSr04Init();
}
