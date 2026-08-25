#include "myTimer.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "tim.h"
#include <stdbool.h>
#include <stdint.h>

static uint8_t s_current_duty = 50;
typedef struct _ledChannel_t{
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint32_t period_ms;
    uint32_t offset_ms;
    bool breath_enable;
    uint8_t current_duty;
}ledChannel_t;

static ledChannel_t s_led_table[LED_MAX_COUNT]={
    [LED_1]={&htim3, TIM_CHANNEL_1,3000, 0, true, 0},
    [LED_2]={&htim3, TIM_CHANNEL_2,3000, 1000, true, 0},
    [LED_3]={&htim4, TIM_CHANNEL_1,3000, 2000, true, 0},
};

void timerInit(void){
    for(int i=0;i<LED_MAX_COUNT;i++){
        if(s_led_table[i].htim!=NULL){
            HAL_TIM_PWM_Start(s_led_table[i].htim, s_led_table[i].channel);
            timerSetDuty((ledId_t)i, 0.0f);
        }
    }
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

void timerSetDuty(ledId_t id, float duty_percent)
{
    if(id>=LED_MAX_COUNT || s_led_table[id].htim==NULL){
        return;
    }

    if(duty_percent<0) duty_percent=0;
    if (duty_percent > 100) duty_percent = 100;
  
    s_led_table[id].current_duty=(uint8_t)(duty_percent+0.5f);
 

  /* ARR (Auto-reload register) 값 가져오기 (+1 하여 전체 주기 계산) */
  uint32_t period = __HAL_TIM_GET_AUTORELOAD(s_led_table[id].htim) + 1;
  uint32_t pulse = (uint32_t)(((float)period * duty_percent) / 100.0f);

  __HAL_TIM_SET_COMPARE(s_led_table[id].htim, s_led_table[id].channel, pulse);


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
uint8_t timerGetDuty(ledId_t id)
{
    if(id<LED_MAX_COUNT){
        return s_led_table[id].current_duty;
    }
  return 0;
}


static float calculateBreathDuty(uint32_t elapsed_time, uint32_t period_ms){
    if(period_ms==0) return 0.0f;

        uint32_t hal_period =period_ms/2;
        float normalized_progress; //

        if(elapsed_time<hal_period){
            normalized_progress=(float)elapsed_time/(float)hal_period;
        }
        else
        {
            normalized_progress=1.0f-((float)(elapsed_time-hal_period)/(float)hal_period);
        }
        

        return (normalized_progress*normalized_progress*100.0f);

}

void timerLedUpdate(void){
    static uint32_t last_tick=0;
    static uint32_t elapsed_time=0;

    uint32_t now=HAL_GetTick();
    uint32_t dt=now-last_tick;

    if(dt>=10){
        last_tick=now;
        elapsed_time+=dt;

        for(int i =0 ; i<LED_MAX_COUNT;i++){
            if (!s_led_table[i].breath_enable || s_led_table[i].period_ms==0){
                continue;
            }

            uint32_t t=(elapsed_time + s_led_table[i].offset_ms)%s_led_table[i].period_ms;
            float duty=calculateBreathDuty(t, s_led_table[i].period_ms);
            timerSetDuty((ledId_t)i, duty);
        }
    }
}

