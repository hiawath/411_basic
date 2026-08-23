#include "myTimer.h"

/* TIM4 핸들러 (Core/Src/tim.c에 정의됨) */
extern TIM_HandleTypeDef htim4;

/**
 * @brief LED 채널 관리 구조체
 */
typedef struct {
  TIM_HandleTypeDef *htim;          /* 연결된 타이머 핸들 포인터 */
  uint32_t           channel;       /* 타이머 채널 (TIM_CHANNEL_1 ~ 4) */
  uint32_t           period_ms;     /* 브리딩 주기 (ms) */
  uint32_t           offset_ms;     /* 시작 시간차 오프셋 (ms) */
  bool               breath_enable; /* 브리딩 활성화 여부 */
  uint8_t            current_duty;  /* 현재 듀티비 (0~100%) */
} ledChannel_t;

/**
 * @brief 하드웨어 매핑 및 상태 관리 테이블
 *        신규 LED 추가 시 이 테이블에 1행만 추가하면 드라이버 전체가 자동 확장됩니다.
 */
static ledChannel_t s_led_table[LED_MAX_COUNT] = {
  [LED_1] = { &htim3, TIM_CHANNEL_1, 3000,    0, true, 0 }, /* PA6: 3초 주기, 0초 시작 */
  [LED_2] = { &htim3, TIM_CHANNEL_2, 3000, 1000, true, 0 }, /* PA7: 3초 주기, 1초 뒤 시작 */
  [LED_3] = { &htim4, TIM_CHANNEL_1, 3000, 2000, true, 0 }, /* PB6: 3초 주기, 2초 뒤 시작 */
};

/**
 * @brief TIM3 CH2 (PA7) 및 TIM4 CH1 (PB6) 하드웨어 자동 안전 초기화
 */
static void timerHardwareAutoInit(void)
{
  /* 1. TIM3 CH2 (PA7) 설정 */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStructA = {0};
  GPIO_InitStructA.Pin = GPIO_PIN_7;
  GPIO_InitStructA.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructA.Pull = GPIO_NOPULL;
  GPIO_InitStructA.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructA.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructA);

  TIM_OC_InitTypeDef sConfigOC3 = {0};
  sConfigOC3.OCMode = TIM_OCMODE_PWM1;
  sConfigOC3.Pulse = 0;
  sConfigOC3.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC3.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC3, TIM_CHANNEL_2);

  /* 2. TIM4 CH1 (PB6) 설정 */
  __HAL_RCC_TIM4_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStructB = {0};
  GPIO_InitStructB.Pin = GPIO_PIN_6;
  GPIO_InitStructB.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructB.Pull = GPIO_NOPULL;
  GPIO_InitStructB.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructB.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructB);

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 84 - 1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000 - 1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_PWM_Init(&htim4);

  TIM_OC_InitTypeDef sConfigOC4 = {0};
  sConfigOC4.OCMode = TIM_OCMODE_PWM1;
  sConfigOC4.Pulse = 0;
  sConfigOC4.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC4.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC4, TIM_CHANNEL_1);
}

/**
 * @brief  테이블에 등록된 모든 LED 타이머 PWM 초기화 및 시작
 */
void timerInit(void)
{
  timerHardwareAutoInit();

  for (int i = 0; i < LED_MAX_COUNT; i++)
  {
    if (s_led_table[i].htim != NULL)
    {
      HAL_TIM_PWM_Start(s_led_table[i].htim, s_led_table[i].channel);
      timerSetDuty((ledId_t)i, 0.0f);
    }
  }
}

/**
 * @brief  특정 LED PWM 출력 시작
 */
void timerPwmStart(ledId_t id)
{
  if (id < LED_MAX_COUNT && s_led_table[id].htim != NULL)
  {
    HAL_TIM_PWM_Start(s_led_table[id].htim, s_led_table[id].channel);
  }
}

/**
 * @brief  특정 LED PWM 출력 정지
 */
void timerPwmStop(ledId_t id)
{
  if (id < LED_MAX_COUNT && s_led_table[id].htim != NULL)
  {
    HAL_TIM_PWM_Stop(s_led_table[id].htim, s_led_table[id].channel);
  }
}

/**
 * @brief  특정 LED의 밝기(듀티비) 직접 설정 (0.0% ~ 100.0%)
 */
void timerSetDuty(ledId_t id, float duty_percent)
{
  if (id >= LED_MAX_COUNT || s_led_table[id].htim == NULL)
  {
    return;
  }

  if (duty_percent < 0.0f) duty_percent = 0.0f;
  if (duty_percent > 100.0f) duty_percent = 100.0f;

  s_led_table[id].current_duty = (uint8_t)(duty_percent + 0.5f);

  uint32_t period = __HAL_TIM_GET_AUTORELOAD(s_led_table[id].htim) + 1;
  uint32_t pulse = (uint32_t)((float)period * (duty_percent / 100.0f));

  __HAL_TIM_SET_COMPARE(s_led_table[id].htim, s_led_table[id].channel, pulse);
}

/**
 * @brief  현재 설정된 특정 LED의 PWM 듀티비(%) 반환
 */
uint8_t timerGetDuty(ledId_t id)
{
  if (id < LED_MAX_COUNT)
  {
    return s_led_table[id].current_duty;
  }
  return 0;
}

/**
 * @brief  특정 LED의 브리딩 파라미터 동적 설정
 */
void timerSetBreathParam(ledId_t id, uint32_t period_ms, uint32_t offset_ms, bool enable)
{
  if (id < LED_MAX_COUNT)
  {
    s_led_table[id].period_ms = period_ms;
    s_led_table[id].offset_ms = offset_ms;
    s_led_table[id].breath_enable = enable;
  }
}

/**
 * @brief  등록된 모든 LED를 N등분 균등 위상차로 일괄 설정하는 헬퍼 함수
 */
void timerSetBreathTrio(uint32_t period_ms)
{
  for (int i = 0; i < LED_MAX_COUNT; i++)
  {
    uint32_t offset = (period_ms * i) / LED_MAX_COUNT;
    timerSetBreathParam((ledId_t)i, period_ms, offset, true);
  }
}

/**
 * @brief  감마 2.0 보정 듀티비 계산 내부 헬퍼
 */
static float calculateBreathDuty(uint32_t current_time_in_period, uint32_t period_ms)
{
  if (period_ms == 0) return 0.0f;

  uint32_t half_period = period_ms / 2;
  float normalized_progress;

  if (current_time_in_period < half_period)
  {
    /* 전반부 (0.0 -> 1.0) */
    normalized_progress = (float)current_time_in_period / (float)half_period;
  }
  else
  {
    /* 후반부 (1.0 -> 0.0) */
    normalized_progress = 1.0f - ((float)(current_time_in_period - half_period) / (float)half_period);
  }

  /* 감마 2.0 곡선 (자연스러운 시각 인지 보정) */
  return (normalized_progress * normalized_progress * 100.0f);
}

/**
 * @brief  메인 루프에서 1번만 호출하는 일괄 브리딩 업데이트 함수
 *         모든 등록된 LED를 루프로 일괄 계산 및 하드웨어 출력
 */
void timerLedUpdate(void)
{
  static uint32_t last_tick = 0;
  static uint32_t elapsed_time = 0;

  uint32_t now = HAL_GetTick();
  uint32_t dt = now - last_tick;

  /* 10ms 단위 (100Hz) 미세 갱신 */
  if (dt >= 10)
  {
    last_tick = now;
    elapsed_time += dt;

    for (int i = 0; i < LED_MAX_COUNT; i++)
    {
      if (!s_led_table[i].breath_enable || s_led_table[i].period_ms == 0)
      {
        continue;
      }

      /* 개별 LED의 주기 내 경과 시간 계산 */
      uint32_t t = (elapsed_time + s_led_table[i].offset_ms) % s_led_table[i].period_ms;
      float duty = calculateBreathDuty(t, s_led_table[i].period_ms);

      timerSetDuty((ledId_t)i, duty);
    }
  }
}
