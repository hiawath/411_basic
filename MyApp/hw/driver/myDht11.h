#pragma once

#include "main.h"
#include "tim.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DHT11 Input Capture 핀 정의 (TIM2 CH1 / PA0) */
#define DHT11_PIN   GPIO_PIN_0
#define DHT11_PORT  GPIOA

typedef struct {
  float temperature;
  float humidity;
  bool  is_valid;
} dht11Data_t;

/**
 * @brief  DHT11 Input Capture 드라이버 초기화
 */
void dht11Init(void);

/**
 * @brief  DHT11 온습도 데이터 읽기 (Input Capture 기반)
 * @param  data : 수신된 데이터를 저장할 구조체 포인터
 * @retval true : 정상 수신, false : 통신 또는 체크섬 실패
 */
bool dht11Read(dht11Data_t *data);

/**
 * @brief  최근 성공적으로 측정된 온도/습도 반환
 */
float dht11GetTemperature(void);
float dht11GetHumidity(void);

/**
 * @brief  TIM2 Input Capture 인터럽트 콜백 처리 함수
 */
void dht11CaptureCallback(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif
