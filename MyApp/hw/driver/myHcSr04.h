#pragma once

#include "main.h"
#include "tim.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Trig 핀: PA8 (일반 GPIO Output) */
#define HCSR04_TRIG_PIN   GPIO_PIN_8
#define HCSR04_TRIG_PORT  GPIOA

/* Echo 핀: PB10 (TIM2 CH3 Input Capture) */
#define HCSR04_ECHO_PIN   GPIO_PIN_10
#define HCSR04_ECHO_PORT  GPIOB

/**
 * @brief  HC-SR04 초음파 센서 드라이버 초기화
 */
void hcSr04Init(void);

/**
 * @brief  초음파 거리 측정 트리거 및 거리 계산 (단위: cm)
 *         - 하드웨어 Input Capture 기반으로 1us 초정밀 측정
 * @param  distance_cm : 측정된 거리(cm)를 저장할 포인터
 * @retval true : 측정 성공, false : 타임아웃 또는 측정 범위(2~400cm) 초과
 */
bool hcSr04Read(float *distance_cm);

/**
 * @brief  최근 측정된 거리 값 반환 (cm)
 */
float hcSr04GetDistance(void);

/**
 * @brief  TIM2 CH3 Input Capture 인터럽트 콜백 처리 함수
 */
void hcSr04CaptureCallback(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif
