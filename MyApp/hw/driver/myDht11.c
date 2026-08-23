#include "myDht11.h"
#include "myTimer.h"
#include "bsp.h"
#include <string.h>
#include <stdio.h>

static float s_latest_temperature = 0.0f;
static float s_latest_humidity = 0.0f;

/* Input Capture 버퍼 및 상태 변수 */
static volatile uint16_t s_diff_buffer[48] = {0};
static volatile uint8_t  s_edge_count = 0;
static volatile uint32_t s_last_captured = 0;
static volatile bool     s_capture_done = false;

/**
 * @brief  PA0 핀을 GPIO Output Open-Drain 모드로 전환
 */
static void dht11SetPinOutput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DHT11_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
 * @brief  PA0 핀을 TIM2 CH1 Input Capture (AF1) 모드로 전환
 */
static void dht11SetPinCapture(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DHT11_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
 * @brief  DHT11 드라이버 초기화
 */
void dht11Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_TIM2_CLK_ENABLE();

  dht11SetPinOutput();
  HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);

  /* TIM2 CH1에 DHT11 Input Capture 콜백 등록 */
  timerAttachCaptureCallback(TIM2, TIM_CHANNEL_1, dht11CaptureCallback);
}

/**
 * @brief  TIM2 Input Capture 인터럽트 서비스 루틴
 *         인터럽트에서는 오직 초고속으로 펄스 간격(diff)만 버퍼에 기록합니다.
 */
void dht11CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance != TIM2 || htim->Channel != HAL_TIM_ACTIVE_CHANNEL_1)
  {
    return;
  }

  uint32_t current_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
  uint32_t diff = current_val - s_last_captured;
  s_last_captured = current_val;

  uint8_t count = s_edge_count;
  if (count < (sizeof(s_diff_buffer) / sizeof(s_diff_buffer[0])))
  {
    s_diff_buffer[count] = (uint16_t)diff;
  }

  s_edge_count++;

  /* 총 41개의 에지가 수신되면 즉시 캡처 완료 */
  if (s_edge_count >= 41)
  {
    HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
    s_capture_done = true;
  }
}

/**
 * @brief  DHT11 데이터 읽기
 */
bool dht11Read(dht11Data_t *data)
{
  uint8_t raw_bytes[5] = {0};

  /* 1. 상태 변수 초기화 */
  memset((void *)s_diff_buffer, 0, sizeof(s_diff_buffer));
  s_edge_count = 0;
  s_capture_done = false;

  /* 2. 시작 신호: PA0을 18ms 동안 LOW 유지 */
  dht11SetPinOutput();
  HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
  HAL_Delay(18);

  /* 3. 버스를 풀업으로 올리고 즉시 Input Capture 모드 활성화 */
  HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
  delayUs(20);
  dht11SetPinCapture();

  /* 4. 타이머 카운터 및 인터럽트 플래그 리셋 후 시작 */
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1 | TIM_FLAG_UPDATE);
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  s_last_captured = 0;
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

  /* 5. 캡처 완료 대기 (최대 30ms) */
  uint32_t timeout_tick = HAL_GetTick();
  while (!s_capture_done)
  {
    if (HAL_GetTick() - timeout_tick > 30)
    {
      HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);

      /* 버스 상태 복구 */
      dht11SetPinOutput();
      HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);

      printf("[DHT11 ERROR] Timeout! Edges:%d/41 | Diffs: %d, %d, %d, %d, %d\r\n",
             s_edge_count, s_diff_buffer[0], s_diff_buffer[1], s_diff_buffer[2], s_diff_buffer[3], s_diff_buffer[4]);
      return false;
    }
  }

  /* 6. 버스 상태 복구 */
  dht11SetPinOutput();
  HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);

  /* 7. 버퍼에서 40비트 데이터 복원 */
  /*
   * 데이터 시작 오프셋 찾기:
   * 센서 응답 신호(Low 80us + High 80us)는 약 130~200us의 길이를 가집니다.
   */
  uint8_t data_start_offset = 2; // 기본: diff[0]=시작, diff[1]=응답, diff[2~41]=데이터

  if (s_diff_buffer[0] >= 120 && s_diff_buffer[0] <= 220)
  {
    data_start_offset = 1;
  }
  else if (s_diff_buffer[1] >= 120 && s_diff_buffer[1] <= 220)
  {
    data_start_offset = 2;
  }

  for (uint8_t i = 0; i < 40; i++)
  {
    uint16_t diff = s_diff_buffer[data_start_offset + i];

    /* 
     * 판별 기준:
     * Bit 0: 약 76~80us (50us Low + 28us High)
     * Bit 1: 약 120~125us (50us Low + 70us High)
     * 임계값: 90us
     */
    if (diff > 90)
    {
      raw_bytes[i / 8] |= (1 << (7 - (i % 8)));
    }
  }

  /* 8. 체크섬 검증 */
  uint8_t checksum = raw_bytes[0] + raw_bytes[1] + raw_bytes[2] + raw_bytes[3];
  
  if (checksum != raw_bytes[4] || (raw_bytes[0] == 0 && raw_bytes[2] == 0))
  {
    printf("[DHT11] Checksum fail! Raw: %02X %02X %02X %02X %02X (Sum:%02X) | Offset:%d | Diffs: %d, %d, %d, %d, %d\r\n",
           raw_bytes[0], raw_bytes[1], raw_bytes[2], raw_bytes[3], raw_bytes[4], checksum, data_start_offset,
           s_diff_buffer[0], s_diff_buffer[1], s_diff_buffer[2], s_diff_buffer[3], s_diff_buffer[4]);
    return false;
  }

  /* 9. 온습도 데이터 변환 */
  s_latest_humidity = (float)raw_bytes[0] + ((float)raw_bytes[1] * 0.1f);
  s_latest_temperature = (float)raw_bytes[2] + ((float)raw_bytes[3] * 0.1f);

  if (data)
  {
    data->humidity = s_latest_humidity;
    data->temperature = s_latest_temperature;
    data->is_valid = true;
  }

  return true;
}

float dht11GetTemperature(void)
{
  return s_latest_temperature;
}

float dht11GetHumidity(void)
{
  return s_latest_humidity;
}
