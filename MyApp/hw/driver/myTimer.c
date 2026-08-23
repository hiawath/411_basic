#include "myTimer.h"

static uint8_t s_current_duty = 0;

/**
 * @brief  TIM3 CH1 (PA6) PWM 타이머 초기화
 */
void timerInit(void)
{
  /* TIM3 Channel 1 (PA6) PWM 신호 출력 시작 */
  timerPwmStart();

  /* 초기 듀티 0% 설정 */
  timerSetDuty(0);
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

/**
 * @brief  3초(3000ms) 기준으로 LED 밝기를 점진적으로 오르락 내리락(Breathing)하는 기본 함수
 *         메인 루프(apMain)에서 주기적으로 호출하면 부드러운 밝기 변화가 실행됩니다.
 */
void timerLedBreath(void)
{
  timerLedBreathUpdate(3000);
}

/**
 * @brief  지정된 주기(period_ms) 동안 LED 밝기가 0% -> 100% -> 0%로 자연스럽게 변화하는 함수
 *         - 사람 눈의 인지 곡선(감마 보정)을 적용하여 어두운 구간에서도 매우 부드럽고 자연스럽게 전환
 *         - 10ms 단위의 미세 보간을 통해 계단 현상 없이 연속적인 밝기 제어
 * @param  period_ms : 1사이클 총 주기 (예: 3000 = 3초)
 */
void timerLedBreathUpdate(uint32_t period_ms)
{
  static uint32_t last_tick = 0;
  static uint32_t elapsed_time = 0;

  if (period_ms == 0)
  {
    return;
  }

  uint32_t now = HAL_GetTick();
  uint32_t dt = now - last_tick;

  /* 약 10ms마다 갱신하여 부드러운 100Hz 갱신 속도 유지 */
  if (dt >= 10)
  {
    last_tick = now;
    elapsed_time = (elapsed_time + dt) % period_ms;

    uint32_t half_period = period_ms / 2;
    float normalized_progress; // 0.0 ~ 1.0

    if (elapsed_time < half_period)
    {
      /* 0.0초 ~ 1.5초 (점진적 밝아짐) */
      normalized_progress = (float)elapsed_time / (float)half_period;
    }
    else
    {
      /* 1.5초 ~ 3.0초 (점진적 어두워짐) */
      normalized_progress = 1.0f - ((float)(elapsed_time - half_period) / (float)half_period);
    }

    /* 
     * 자연스러운 밝기 인지를 위한 2차 곡선 감마 보정 (Gamma ~ 2.0)
     * 사람의 눈은 밝기 변화를 로그적으로 인식하므로, 
     * 제곱 곡선을 적용할 때 가장 자연스러운 Breathing 효과가 나타납니다.
     */
    float duty_gamma = normalized_progress * normalized_progress * 100.0f;

    timerSetDutyFloat(duty_gamma);
  }
}
