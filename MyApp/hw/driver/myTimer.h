#pragma once

#include "main.h"
#include "tim.h"



/**
 * @brief  TIM3 CH1 (PA6) PWM 타이머 초기화 (기본 듀티 50%로 시작)
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

