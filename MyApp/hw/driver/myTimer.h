#pragma once

#include "main.h"
#include "tim.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 3개 LED 식별 매크로 */
#define LED_1       1   /* TIM3 CH1 (PA6) */
#define LED_2       2   /* TIM3 CH2 (PA7) */
#define LED_3       3   /* TIM4 CH1 (PB6) */

/* 하위 호환 매크로 */
#define TIMER_CH1   LED_1
#define TIMER_CH2   LED_2

/**
 * @brief  3개의 타이머 채널 (PA6, PA7, PB6) PWM 초기화 및 출력 시작
 */
void timerInit(void);

/**
 * @brief  특정 LED의 PWM 출력 시작
 * @param  led_id : LED_1 (PA6), LED_2 (PA7), LED_3 (PB6)
 */
void timerPwmStart(uint8_t led_id);

/**
 * @brief  특정 LED의 PWM 출력 정지
 * @param  led_id : LED_1, LED_2, LED_3
 */
void timerPwmStop(uint8_t led_id);

/**
 * @brief  특정 LED 밝기(듀티비) 설정 (0% ~ 100%)
 * @param  led_id : LED_1, LED_2, LED_3
 * @param  duty_percent : 0 ~ 100 (%)
 */
void timerSetDuty(uint8_t led_id, uint8_t duty_percent);

/**
 * @brief  특정 LED 밝기(듀티비) 설정 - 정밀 float (0.0% ~ 100.0%)
 * @param  led_id : LED_1, LED_2, LED_3
 * @param  duty_percent : 0.0 ~ 100.0 (%)
 */
void timerSetDutyFloat(uint8_t led_id, float duty_percent);

/**
 * @brief  각 LED 듀티비 간편 설정 함수들
 */
void timerSetDutyLed1(uint8_t duty_percent);
void timerSetDutyLed2(uint8_t duty_percent);
void timerSetDutyLed3(uint8_t duty_percent);

/**
 * @brief  현재 설정된 특정 LED의 PWM 듀티비(%) 반환
 * @param  led_id : LED_1, LED_2, LED_3
 * @return 듀티비 (%)
 */
uint8_t timerGetDuty(uint8_t led_id);

/**
 * @brief  [3중 순차 파도 브리딩 (120도 위상차)]
 *         3초(3000ms) 주기로 LED1, LED2, LED3가 각각 1초(1000ms)씩 시차를 두고 
 *         순차적으로 부드럽게 물결치듯이 밝아졌다 어두워집니다.
 */
void timerLedBreath(void);
void timerLedBreathTrio(void);

/**
 * @brief  [3개 독립 주기 브리딩]
 *         LED 3개가 각각 서로 다른 독립된 주기로 완전히 별개로 호흡(Breathing)합니다.
 * @param  period1_ms : LED1 주기 (기본: 2000ms = 2.0초)
 * @param  period2_ms : LED2 주기 (기본: 3000ms = 3.0초)
 * @param  period3_ms : LED3 주기 (기본: 4000ms = 4.0초)
 */
void timerLedBreathIndependent(uint32_t period1_ms, uint32_t period2_ms, uint32_t period3_ms);

/**
 * @brief  [사용자 지정 3채널 시간차 브리딩]
 * @param  period_ms : 전체 주기
 * @param  offset1_ms : LED1 시작 시간차
 * @param  offset2_ms : LED2 시작 시간차
 * @param  offset3_ms : LED3 시작 시간차
 */
void timerLedBreathCustom(uint32_t period_ms, uint32_t offset1_ms, uint32_t offset2_ms, uint32_t offset3_ms);

#ifdef __cplusplus
}
#endif
