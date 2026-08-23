#include "myTimer.h"

static uint8_t s_duty_ch1 = 0;
static uint8_t s_duty_ch2 = 0;

/**
 * @brief  TIM3 CH2 (PA7) 하드웨어 핀 및 채널 보조 초기화
 *         (CubeMX에서 CH2가 미설정된 상태여도 안전하게 동작하도록 보장)
 */
static void timerCh2HardwareInit(void)
{
  /* GPIOA 클럭 활성화 */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* PA7 -> TIM3_CH2 Alternate Function 설정 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* TIM3 CH2 PWM 채널 설정 */
  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);
}

/**
 * @brief  TIM3 CH1(PA6), CH2(PA7) PWM 타이머 초기화
 */
void timerInit(void)
{
  /* CH2 (PA7) 핀 및 채널 설정 보장 */
  timerCh2HardwareInit();

  /* CH1 (PA6) 및 CH2 (PA7) PWM 출력 시작 */
  timerPwmStart(TIM_CHANNEL_1);
  timerPwmStart(TIM_CHANNEL_2);

  /* 초기 듀티 0% */
  timerSetDuty(TIM_CHANNEL_1, 0);
  timerSetDuty(TIM_CHANNEL_2, 0);
}

/**
 * @brief  특정 채널 PWM 출력 시작
 * @param  channel : TIM_CHANNEL_1 또는 TIM_CHANNEL_2
 */
void timerPwmStart(uint32_t channel)
{
  HAL_TIM_PWM_Start(&htim3, channel);
}

/**
 * @brief  특정 채널 PWM 출력 정지
 * @param  channel : TIM_CHANNEL_1 또는 TIM_CHANNEL_2
 */
void timerPwmStop(uint32_t channel)
{
  HAL_TIM_PWM_Stop(&htim3, channel);
}

/**
 * @brief  특정 채널 LED 밝기(듀티비) 설정 (0% ~ 100%)
 * @param  channel : TIM_CHANNEL_1 또는 TIM_CHANNEL_2
 * @param  duty_percent : 0 ~ 100 (%)
 */
void timerSetDuty(uint32_t channel, uint8_t duty_percent)
{
  if (duty_percent > 100)
  {
    duty_percent = 100;
  }

  if (channel == TIM_CHANNEL_1)
  {
    s_duty_ch1 = duty_percent;
  }
  else if (channel == TIM_CHANNEL_2)
  {
    s_duty_ch2 = duty_percent;
  }

  uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim3) + 1;
  uint32_t pulse = (period * duty_percent) / 100;

  __HAL_TIM_SET_COMPARE(&htim3, channel, pulse);
}

/**
 * @brief  특정 채널 LED 밝기(듀티비) 설정 - 정밀 float (0.0% ~ 100.0%)
 * @param  channel : TIM_CHANNEL_1 또는 TIM_CHANNEL_2
 * @param  duty_percent : 0.0 ~ 100.0 (%)
 */
void timerSetDutyFloat(uint32_t channel, float duty_percent)
{
  if (duty_percent < 0.0f)
  {
    duty_percent = 0.0f;
  }
  if (duty_percent > 100.0f)
  {
    duty_percent = 100.0f;
  }

  if (channel == TIM_CHANNEL_1)
  {
    s_duty_ch1 = (uint8_t)(duty_percent + 0.5f);
  }
  else if (channel == TIM_CHANNEL_2)
  {
    s_duty_ch2 = (uint8_t)(duty_percent + 0.5f);
  }

  uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim3) + 1;
  uint32_t pulse = (uint32_t)((float)period * (duty_percent / 100.0f));

  __HAL_TIM_SET_COMPARE(&htim3, channel, pulse);
}

/**
 * @brief  CH1 (PA6) 듀티비 간편 설정
 */
void timerSetDutyCh1(uint8_t duty_percent)
{
  timerSetDuty(TIM_CHANNEL_1, duty_percent);
}

/**
 * @brief  CH2 (PA7) 듀티비 간편 설정
 */
void timerSetDutyCh2(uint8_t duty_percent)
{
  timerSetDuty(TIM_CHANNEL_2, duty_percent);
}

/**
 * @brief  현재 설정된 채널의 PWM 듀티비(%) 반환
 * @param  channel : TIM_CHANNEL_1 또는 TIM_CHANNEL_2
 * @return 듀티비 (%)
 */
uint8_t timerGetDuty(uint32_t channel)
{
  if (channel == TIM_CHANNEL_2)
  {
    return s_duty_ch2;
  }
  return s_duty_ch1;
}

/**
 * @brief  3초(3000ms) 기준으로 CH1(PA6), CH2(PA7) LED 밝기를 함께 오르락 내리락(Breathing)
 */
void timerLedBreath(void)
{
  timerLedBreathUpdate(3000);
}

/**
 * @brief  지정된 주기(period_ms) 동안 CH1(PA6)과 CH2(PA7) LED 밝기가 
 *         0% -> 100% -> 0%로 자연스럽게 변화하는 함수
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

  /* 10ms 단위 100Hz 미세 갱신 */
  if (dt >= 10)
  {
    last_tick = now;
    elapsed_time = (elapsed_time + dt) % period_ms;

    uint32_t half_period = period_ms / 2;
    float normalized_progress;

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

    /* 자연스러운 밝기를 위한 2차 곡선 감마 보정 */
    float duty_gamma = normalized_progress * normalized_progress * 100.0f;

    /* PA6 (CH1) 및 PA7 (CH2) 동시 브리딩 제어 */
    timerSetDutyFloat(TIM_CHANNEL_1, duty_gamma);
    timerSetDutyFloat(TIM_CHANNEL_2, duty_gamma);
  }
}
