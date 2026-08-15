#pragma once

#include "main.h"
#include "adc.h"
#include <stdint.h>
#include <stdbool.h>

void adcInit(void);
uint32_t adcReadRaw(void);
float adcReadTemp(void);
