#include "myTimer.h"
#include "stm32f4xx_hal.h"
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
    if(duty_percent<0){
        duty_percent=0;
    }   
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

void timerSetDutyFloat(float duty_percent){
    if(duty_percent<0.0f){
        duty_percent=0;
    }
    if(duty_percent>100.0f){
        duty_percent=100.0f;
    }

    s_current_duty=(uint8_t)(duty_percent+0.5f);

  uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim3) + 1;
  uint32_t pulse = (uint32_t)(((float)period * duty_percent) / 100.0f);

  /* CCR1 레지스터 값 갱신 */
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);

}
uint8_t timerGetDuty(void)
{
  return s_current_duty;
}

void timerLedBreath(void){
    timerLedBreathUpdate(3000);
}
void timerLedBreathUpdate(uint32_t period_ms){
    static uint32_t last_tick=0;
    static uint32_t elapsed_time=0;

    if (period_ms==0){
        return;
    }
    uint32_t now=HAL_GetTick();
    uint32_t dt=now-last_tick;

    if(dt>=10){
        last_tick=now;
        elapsed_time=(elapsed_time+dt)%period_ms;

        uint32_t hal_period =period_ms/2;
        float normalized_progress; //

        if(elapsed_time<hal_period){
            normalized_progress=(float)elapsed_time/(float)hal_period;
        }
        else
        {
            normalized_progress=1.0f-((float)(elapsed_time-hal_period)/(float)hal_period);
        }
        

        float duty_gamma=normalized_progress*normalized_progress*100.0f;

        timerSetDutyFloat(duty_gamma);
    }

    
}

