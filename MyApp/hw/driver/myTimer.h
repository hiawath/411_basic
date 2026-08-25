#pragma once
#include "main.h"
#include "tim.h"
#include <stdint.h>

void timerInit(void);
void timerPwmStart(void);
void timerPwmStop(void);

void timerSetDuty(uint8_t duty_percent);
uint8_t timerGetDuty(void);
void timerLedBreath(void);
void timerLedBreathUpdate(uint32_t period_ms);
void timerSetDutyFloat(float duty_percent);