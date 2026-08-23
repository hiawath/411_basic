#pragma once

#include "main.h"
#include "tim.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LED 채널 식별자 열거형
 *        새로운 타이머/LED가 추가되면 이 열거형에 ID를 추가하기만 하면 됩니다.
 */
typedef enum {
  LED_1 = 0,    /* PA6 (TIM3 CH1) */
  LED_2,        /* PA7 (TIM3 CH2) */
  LED_3,        /* PB6 (TIM4 CH1) */
  // 새로운 채널 추가 예시:
  // LED_4,     /* PB7 (TIM4 CH2) */
  LED_MAX_COUNT
} ledId_t;

/* 하위 호환 및 별칭 매크로 */
#define LED_PA6     LED_1
#define LED_PA7     LED_2
#define LED_PB6     LED_3
#define TIMER_CH1   LED_1
#define TIMER_CH2   LED_2

/**
 * @brief  테이블에 등록된 모든 LED 타이머 PWM 채널 및 핀 초기화
 */
void timerInit(void);

/**
 * @brief  특정 LED의 PWM 출력 시작
 * @param  id : ledId_t
 */
void timerPwmStart(ledId_t id);

/**
 * @brief  특정 LED의 PWM 출력 정지
 * @param  id : ledId_t
 */
void timerPwmStop(ledId_t id);

/**
 * @brief  특정 LED의 밝기(듀티비) 직접 설정 (0.0% ~ 100.0%)
 * @param  id : ledId_t
 * @param  duty_percent : 0.0 ~ 100.0 (%)
 */
void timerSetDuty(ledId_t id, float duty_percent);

/**
 * @brief  현재 설정된 특정 LED의 PWM 듀티비(%) 반환
 * @param  id : ledId_t
 * @return 듀티비 (0 ~ 100 %)
 */
uint8_t timerGetDuty(ledId_t id);

/**
 * @brief  특정 LED의 브리딩(Breathing) 파라미터 동적 설정
 * @param  id : ledId_t
 * @param  period_ms : 1사이클 전체 주기 (ms)
 * @param  offset_ms : 시작 시간차 / 위상 오프셋 (ms)
 * @param  enable : true면 자동 브리딩 활성화, false면 고정 듀티 유지
 */
void timerSetBreathParam(ledId_t id, uint32_t period_ms, uint32_t offset_ms, bool enable);

/**
 * @brief  등록된 모든 LED를 N등분 균등 위상차(파도 효과)로 일괄 설정하는 헬퍼 함수
 * @param  period_ms : 1사이클 전체 주기 (예: 3000ms)
 */
void timerSetBreathTrio(uint32_t period_ms);

/**
 * @brief  메인 루프(apMain)에서 1번만 호출하는 일괄 브리딩 업데이트 함수
 *         등록된 모든 활성 LED를 10ms(100Hz) 단위로 부드럽게 일괄 제어합니다.
 */
void timerLedUpdate(void);

/* 하위 호환 호칭 */
#define timerLedBreath()      timerLedUpdate()
#define timerLedBreathTrio()  timerLedUpdate()

#ifdef __cplusplus
}
#endif
