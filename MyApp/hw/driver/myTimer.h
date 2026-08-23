#pragma once

#include "main.h"
#include "tim.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 채널 정의 매크로 */
#define TIMER_CH1   TIM_CHANNEL_1   /* PA6 */
#define TIMER_CH2   TIM_CHANNEL_2   /* PA7 */

/**
 * @brief  TIM3 CH1(PA6), CH2(PA7) PWM 타이머 초기화 및 출력 시작
 */
void timerInit(void);

/**
 * @brief  특정 채널 PWM 출력 시작
 * @param  channel : TIM_CHANNEL_1 또는 TIM_CHANNEL_2
 */
void timerPwmStart(uint32_t channel);

/**
 * @brief  특정 채널 PWM 출력 정지
 * @param  channel : TIM_CHANNEL_1 또는 TIM_CHANNEL_2
 */
void timerPwmStop(uint32_t channel);

/**
 * @brief  특정 채널 LED 밝기(듀티비) 설정 (0% ~ 100%)
 * @param  channel : TIM_CHANNEL_1 또는 TIM_CHANNEL_2
 * @param  duty_percent : 0 ~ 100 (%)
 */
void timerSetDuty(uint32_t channel, uint8_t duty_percent);

/**
 * @brief  특정 채널 LED 밝기(듀티비) 설정 - 정밀 float (0.0% ~ 100.0%)
 * @param  channel : TIM_CHANNEL_1 또는 TIM_CHANNEL_2
 * @param  duty_percent : 0.0 ~ 100.0 (%)
 */
void timerSetDutyFloat(uint32_t channel, float duty_percent);

/**
 * @brief  CH1 (PA6) 듀티비 간편 설정
 */
void timerSetDutyCh1(uint8_t duty_percent);

/**
 * @brief  CH2 (PA7) 듀티비 간편 설정
 */
void timerSetDutyCh2(uint8_t duty_percent);

/**
 * @brief  현재 설정된 채널의 PWM 듀티비(%) 반환
 * @param  channel : TIM_CHANNEL_1 또는 TIM_CHANNEL_2
 * @return 듀티비 (%)
 */
uint8_t timerGetDuty(uint32_t channel);

/**
 * @brief  3초(3000ms) 기준으로 CH1(PA6)과 CH2(PA7) LED 밝기를 함께 오르락 내리락(Breathing)
 *         메인 루프(apMain)에서 주기적으로 호출하면 부드러운 밝기 변화가 실행됩니다.
 */
void timerLedBreath(void);

/**
 * @brief  사용자 지정 주기(ms) 기준으로 CH1, CH2 LED 밝기를 점진적으로 제어하는 비동기 함수
 * @param  period_ms : 1사이클 총 주기 (예: 3000ms = 3초 동안 0% -> 100% -> 0%)
 */
void timerLedBreathUpdate(uint32_t period_ms);

#ifdef __cplusplus
}
#endif
