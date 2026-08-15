#include "myHw.h"

void hwInit(void)
{
  gpioInit();
  uartInit();
  i2cInit();
  ssd1306Init();
}
