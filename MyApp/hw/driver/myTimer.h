#pragma once

#include "main.h"
#include "tim.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  TIM3 CH1 (PA6) PWM 타이머 초기화 (기본 듀티 0% 또는 50% 시작)
 */
void timerInit(void);

/**
 * @brief  TIM3 CH1 PWM 출력 시작
 */
void timerPwmStart(void);

/**
 * @brief  TIM3 CH1 PWM 출력 정지
 */
void timerPwmStop(void);

/**
 * @brief  LED 밝기(듀티비) 설정 (0% ~ 100%)
 * @param  duty_percent : 0 ~ 100 (%)
 */
void timerSetDuty(uint8_t duty_percent);

/**
 * @brief  LED 밝기(듀티비) 설정 - 정밀 float (0.0% ~ 100.0%)
 * @param  duty_percent : 0.0 ~ 100.0 (%)
 */
void timerSetDutyFloat(float duty_percent);

/**
 * @brief  현재 설정된 PWM 듀티비(%) 반환
 * @return 듀티비 (%)
 */
uint8_t timerGetDuty(void);

/**
 * @brief  PWM 비교값(CCR1) 직접 설정
 * @param  pulse : 0 ~ ARR 값
 */
void timerSetPulse(uint32_t pulse);

/**
 * @brief  3초(3000ms) 기준으로 LED 밝기를 점진적으로 오르락 내리락(Breathing)하는 비동기 함수
 *         메인 루프(apMain)에서 주기적으로 호출하면 부드러운 밝기 변화가 실행됩니다.
 */
void timerLedBreath(void);

/**
 * @brief  사용자 지정 주기(ms) 기준으로 LED 밝기를 점진적으로 제어하는 비동기 함수
 * @param  period_ms : 전체 1사이클 주기 (예: 3000ms = 3초 동안 0% -> 100% -> 0%)
 */
void timerLedBreathUpdate(uint32_t period_ms);

#ifdef __cplusplus
}
#endif
