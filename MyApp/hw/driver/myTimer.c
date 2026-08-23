#include "myTimer.h"

/* TIM4 핸들러 (PB6, TIM4_CH1 제어용) */
TIM_HandleTypeDef htim4;

static uint8_t s_duty_led1 = 0;
static uint8_t s_duty_led2 = 0;
static uint8_t s_duty_led3 = 0;

/**
 * @brief  TIM3 CH2 (PA7) 하드웨어 핀 및 채널 보조 초기화
 */
static void timerCh2HardwareInit(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);
}

/**
 * @brief  TIM4 CH1 (PB6) 하드웨어 및 타이머 PWM 초기화
 */
static void timer4HardwareInit(void)
{
  /* 클럭 활성화 */
  __HAL_RCC_TIM4_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* PB6 -> TIM4_CH1 설정 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* TIM4 기본 PWM 설정 (1kHz 주파수: 84MHz / 84 / 1000 = 1kHz) */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 84 - 1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000 - 1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_PWM_Init(&htim4);

  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1);
}

/**
 * @brief  3개의 타이머 채널 (PA6, PA7, PB6) PWM 초기화 및 출력 시작
 */
void timerInit(void)
{
  /* TIM3 CH2 (PA7) 및 TIM4 CH1 (PB6) 하드웨어 설정 */
  timerCh2HardwareInit();
  timer4HardwareInit();

  /* 모든 채널 PWM 시작 */
  timerPwmStart(LED_1);
  timerPwmStart(LED_2);
  timerPwmStart(LED_3);

  /* 초기 듀티 0% */
  timerSetDuty(LED_1, 0);
  timerSetDuty(LED_2, 0);
  timerSetDuty(LED_3, 0);
}

/**
 * @brief  특정 LED PWM 출력 시작
 */
void timerPwmStart(uint8_t led_id)
{
  if (led_id == LED_1)
  {
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  }
  else if (led_id == LED_2)
  {
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  }
  else if (led_id == LED_3)
  {
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  }
}

/**
 * @brief  특정 LED PWM 출력 정지
 */
void timerPwmStop(uint8_t led_id)
{
  if (led_id == LED_1)
  {
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
  }
  else if (led_id == LED_2)
  {
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
  }
  else if (led_id == LED_3)
  {
    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
  }
}

/**
 * @brief  특정 LED 밝기(듀티비) 설정 (0% ~ 100%)
 */
void timerSetDuty(uint8_t led_id, uint8_t duty_percent)
{
  if (duty_percent > 100)
  {
    duty_percent = 100;
  }

  if (led_id == LED_1)
  {
    s_duty_led1 = duty_percent;
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim3) + 1;
    uint32_t pulse = (period * duty_percent) / 100;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
  }
  else if (led_id == LED_2)
  {
    s_duty_led2 = duty_percent;
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim3) + 1;
    uint32_t pulse = (period * duty_percent) / 100;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
  }
  else if (led_id == LED_3)
  {
    s_duty_led3 = duty_percent;
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim4) + 1;
    uint32_t pulse = (period * duty_percent) / 100;
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);
  }
}

/**
 * @brief  특정 LED 밝기(듀티비) 설정 - 정밀 float (0.0% ~ 100.0%)
 */
void timerSetDutyFloat(uint8_t led_id, float duty_percent)
{
  if (duty_percent < 0.0f)
  {
    duty_percent = 0.0f;
  }
  if (duty_percent > 100.0f)
  {
    duty_percent = 100.0f;
  }

  if (led_id == LED_1)
  {
    s_duty_led1 = (uint8_t)(duty_percent + 0.5f);
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim3) + 1;
    uint32_t pulse = (uint32_t)((float)period * (duty_percent / 100.0f));
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
  }
  else if (led_id == LED_2)
  {
    s_duty_led2 = (uint8_t)(duty_percent + 0.5f);
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim3) + 1;
    uint32_t pulse = (uint32_t)((float)period * (duty_percent / 100.0f));
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
  }
  else if (led_id == LED_3)
  {
    s_duty_led3 = (uint8_t)(duty_percent + 0.5f);
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim4) + 1;
    uint32_t pulse = (uint32_t)((float)period * (duty_percent / 100.0f));
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);
  }
}

void timerSetDutyLed1(uint8_t duty_percent) { timerSetDuty(LED_1, duty_percent); }
void timerSetDutyLed2(uint8_t duty_percent) { timerSetDuty(LED_2, duty_percent); }
void timerSetDutyLed3(uint8_t duty_percent) { timerSetDuty(LED_3, duty_percent); }

/**
 * @brief  현재 설정된 특정 LED의 PWM 듀티비(%) 반환
 */
uint8_t timerGetDuty(uint8_t led_id)
{
  if (led_id == LED_1) return s_duty_led1;
  if (led_id == LED_2) return s_duty_led2;
  if (led_id == LED_3) return s_duty_led3;
  return 0;
}

/**
 * @brief  감마 보정 듀티비 계산 내부 함수
 */
static float calculateBreathDuty(uint32_t current_time_in_period, uint32_t period_ms)
{
  uint32_t half_period = period_ms / 2;
  float normalized_progress;

  if (current_time_in_period < half_period)
  {
    normalized_progress = (float)current_time_in_period / (float)half_period;
  }
  else
  {
    normalized_progress = 1.0f - ((float)(current_time_in_period - half_period) / (float)half_period);
  }

  return (normalized_progress * normalized_progress * 100.0f);
}

/**
 * @brief  [3중 순차 파도 브리딩 (120도 위상차)]
 *         3초(3000ms) 주기로 LED1, LED2, LED3가 각각 1초(1000ms)씩 시차를 두고 순차 물결 이동
 */
void timerLedBreath(void)
{
  timerLedBreathTrio();
}

void timerLedBreathTrio(void)
{
  /* 3000ms 주기: LED1(0ms 시차), LED2(1000ms 시차), LED3(2000ms 시차) */
  timerLedBreathCustom(3000, 0, 1000, 2000);
}

/**
 * @brief  [사용자 지정 3채널 시간차 브리딩]
 */
void timerLedBreathCustom(uint32_t period_ms, uint32_t offset1_ms, uint32_t offset2_ms, uint32_t offset3_ms)
{
  static uint32_t last_tick = 0;
  static uint32_t elapsed_time = 0;

  if (period_ms == 0) return;

  uint32_t now = HAL_GetTick();
  uint32_t dt = now - last_tick;

  if (dt >= 10)
  {
    last_tick = now;
    elapsed_time = (elapsed_time + dt) % period_ms;

    uint32_t t1 = (elapsed_time + offset1_ms) % period_ms;
    uint32_t t2 = (elapsed_time + offset2_ms) % period_ms;
    uint32_t t3 = (elapsed_time + offset3_ms) % period_ms;

    timerSetDutyFloat(LED_1, calculateBreathDuty(t1, period_ms));
    timerSetDutyFloat(LED_2, calculateBreathDuty(t2, period_ms));
    timerSetDutyFloat(LED_3, calculateBreathDuty(t3, period_ms));
  }
}

/**
 * @brief  [3개 독립 주기 브리딩]
 *         LED 3개가 각각 서로 다른 독립된 주기(속도)로 완전히 따로 호흡(Breathing)
 */
void timerLedBreathIndependent(uint32_t period1_ms, uint32_t period2_ms, uint32_t period3_ms)
{
  static uint32_t last_tick = 0;
  static uint32_t t1 = 0, t2 = 0, t3 = 0;

  uint32_t now = HAL_GetTick();
  uint32_t dt = now - last_tick;

  if (dt >= 10)
  {
    last_tick = now;

    if (period1_ms > 0) t1 = (t1 + dt) % period1_ms;
    if (period2_ms > 0) t2 = (t2 + dt) % period2_ms;
    if (period3_ms > 0) t3 = (t3 + dt) % period3_ms;

    timerSetDutyFloat(LED_1, calculateBreathDuty(t1, period1_ms));
    timerSetDutyFloat(LED_2, calculateBreathDuty(t2, period2_ms));
    timerSetDutyFloat(LED_3, calculateBreathDuty(t3, period3_ms));
  }
}
