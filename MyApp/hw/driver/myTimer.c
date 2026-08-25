#include "myTimer.h"
#include "stm32f4xx_hal_tim.h"
#include "tim.h"
#include <stdint.h>
static uint8_t s_current_duty = 50;

void timerInit(void){
    timerPwmStart();
    timerSetDuty(50);
}

void timerPwmStart(void){
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    //HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_1);
}

void timerPwmStop(void){
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
}

void timerSetDuty(uint8_t duty_percent)
{
  if (duty_percent > 100)
  {
    duty_percent = 100;
  }

  s_current_duty = duty_percent;

  /* ARR (Auto-reload register) 값 가져오기 (+1 하여 전체 주기 계산) */
  uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim3) + 1;
  uint32_t pulse = (period * duty_percent) / 100;

  /* CCR1 레지스터 값 갱신 */
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);
}
uint8_t timerGetDuty(void)
{
  return s_current_duty;
}

