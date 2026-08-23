#pragma once

#include "main.h"
#include <stdint.h>



/**
 * @brief  BSP (Board Support Package) 시스템 기본 하드웨어 초기화 (DWT 사이클 카운터 활성화)
 */
void bspInit(void);

/**
 * @brief  ARM Cortex-M DWT 기반 초정밀 마이크로초(us) 딜레이 함수
 *         컴파일러 최적화 옵션 및 시스템 클럭 변화에 영향받지 않는 1클럭 단위 정밀도 제공
 * @param  us : 지연 시간 (마이크로초)
 */
void delayUs(uint32_t us);


