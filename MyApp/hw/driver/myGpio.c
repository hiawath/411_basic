#include "myGpio.h"

void gpioInit(void){
    //
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

  if(GPIO_Pin==GPIO_PIN_13){
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  }
}