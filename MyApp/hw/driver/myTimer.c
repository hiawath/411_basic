#include "myTimer.h"

static uint8_t s_current_duty = 50;

/**
 * @brief  TIM3 CH1 (PA6) PWM 타이머 초기화
 */
void timerInit(void)
{
  /* TIM3 Channel 1 (PA6) PWM 신호 출력 시작 */
  timerPwmStart();

  /* 기본 밝기를 듀티 50%로 설정 */
  timerSetDuty(50);
}

/**
 * @brief  TIM3 CH1 PWM 출력 시작
 */
void timerPwmStart(void)
{
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

/**
 * @brief  TIM3 CH1 PWM 출력 정지
 */
void timerPwmStop(void)
{
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

/**
 * @brief  LED 밝기(듀티비) 설정 (0% ~ 100%)
 * @param  duty_percent : 0 ~ 100 (%)
 */
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
}

/**
 * @brief  LED 밝기(듀티비) 설정 - 정밀 float (0.0% ~ 100.0%)
 * @param  duty_percent : 0.0 ~ 100.0 (%)
 */
void timerSetDutyFloat(float duty_percent)
{
  if (duty_percent < 0.0f)
  {
    duty_percent = 0.0f;
  }
  if (duty_percent > 100.0f)
  {
    duty_percent = 100.0f;
  }

  s_current_duty = (uint8_t)(duty_percent + 0.5f);

  uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim3) + 1;
  uint32_t pulse = (uint32_t)((float)period * (duty_percent / 100.0f));

  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
}

/**
 * @brief  현재 설정된 PWM 듀티비(%) 반환
 * @return 듀티비 (%)
 */
uint8_t timerGetDuty(void)
{
  return s_current_duty;
}

/**
 * @brief  PWM 비교값(CCR1) 직접 설정
 * @param  pulse : 0 ~ ARR 값
 */
void timerSetPulse(uint32_t pulse)
{
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
}
