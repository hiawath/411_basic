#pragma once
#include "main.h"
#include "tim.h"
#include <stdint.h>

typedef enum{
    LED_1=0,
    LED_2,
    LED_3,

    LED_MAX_COUNT
} ledId_t;
void timerInit(void);
void timerPwmStart(void);
void timerPwmStop(void);

void timerSetDuty(ledId_t id, float duty_percent);
uint8_t timerGetDuty(ledId_t id);

void timerSetDutyFloat(float duty_percent);
void timerLedUpdate(void);