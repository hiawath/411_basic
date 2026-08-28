#include "myGpio.h"
#include "myMcp2515.h"
#include <stdio.h>


void gpioInit(void){
    //
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

  if(GPIO_Pin==GPIO_PIN_13){
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  }

  if(GPIO_Pin==MCP2515_INT_PIN){
    mcp2515IsrHandler();
  }
}




