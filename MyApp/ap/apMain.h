#pragma once

#include "main.h"

#define RX_BUF_SIZE 128
extern uint8_t rx_data;
extern uint8_t rx_buf[RX_BUF_SIZE];

void apMain(void);
void I2C_Scan(I2C_HandleTypeDef *hi2c);
