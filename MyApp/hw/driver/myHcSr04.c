#include "myHcSr04.h"

static float s_latest_distance = 0.0f;

/* Input Capture 상태 변수 */
static volatile uint32_t s_t_rising = 0;
static volatile uint32_t s_t_falling = 0;
static volatile bool     s_is_rising = true;
static volatile bool     s_capture_done = false;

/* 마이크로초 딜레이 (DWT 기반) */
static void delayUs(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (SystemCoreClock / 1000000);
  while ((DWT->CYCCNT - start) < ticks);
}

#include "myTimer.h"

/**
 * @brief  HC-SR04 초음파 센서 드라이버 초기화
 */
void hcSr04Init(void)
{
  /* DWT 사이클 카운터 활성화 */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_TIM2_CLK_ENABLE();

  /* Trig 핀 (PA8): Output Push-Pull */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = HCSR04_TRIG_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(HCSR04_TRIG_PORT, &GPIO_InitStruct);

  HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_RESET);

  /* TIM2 CH3에 HC-SR04 Input Capture 콜백 등록 */
  timerAttachCaptureCallback(TIM2, TIM_CHANNEL_3, hcSr04CaptureCallback);
}

/**
 * @brief  TIM2 CH3 Input Capture 인터럽트 서비스 루틴
 *         1. Rising Edge 감지 시: High 시작 시간(t_rising) 기록 후 Falling Edge로 전환
 *         2. Falling Edge 감지 시: High 종료 시간(t_falling) 기록 후 펄스 폭 계산
 */
void hcSr04CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance != TIM2 || htim->Channel != HAL_TIM_ACTIVE_CHANNEL_3)
  {
    return;
  }

  if (s_is_rising)
  {
    /* 1. Rising Edge (Echo High 시작) */
    s_t_rising = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
    s_is_rising = false;

    /* 다음 에지를 Falling Edge로 캡처하도록 변경 */
    __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING);
  }
  else
  {
    /* 2. Falling Edge (Echo High 종료) */
    s_t_falling = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);

    uint32_t pulse_width = s_t_falling - s_t_rising;

    /* 거리(cm) = 펄스시간(us) / 58.0 */
    s_latest_distance = (float)pulse_width / 58.0f;

    /* 캡처 인터럽트 정지 및 극성 복구 */
    HAL_TIM_IC_Stop_IT(htim, TIM_CHANNEL_3);
    __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
    s_is_rising = true;
    s_capture_done = true;
  }
}

/**
 * @brief  초음파 거리 측정 트리거 및 거리 계산
 */
bool hcSr04Read(float *distance_cm)
{
  s_is_rising = true;
  s_capture_done = false;

  /* 1. TIM2 CH3 Rising Edge 캡처 준비 */
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC3);
  __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);

  /* 2. Trig 핀에 10us HIGH 펄스 인가 */
  HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_RESET);
  delayUs(2);
  HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_SET);
  delayUs(10);
  HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_RESET);

  /* 3. 캡처 완료 대기 (최대 35ms: 400cm 도달 시간 약 23.2ms) */
  uint32_t timeout_tick = HAL_GetTick();
  while (!s_capture_done)
  {
    if (HAL_GetTick() - timeout_tick > 35)
    {
      HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_3);
      __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
      s_is_rising = true;
      return false;
    }
  }

  /* 4. 유효 거리 범위 검증 (2cm ~ 400cm) */
  if (s_latest_distance < 2.0f || s_latest_distance > 400.0f)
  {
    return false;
  }

  if (distance_cm)
  {
    *distance_cm = s_latest_distance;
  }

  return true;
}

float hcSr04GetDistance(void)
{
  return s_latest_distance;
}
