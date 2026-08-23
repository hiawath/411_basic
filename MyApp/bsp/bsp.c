#include "bsp.h"

/**
 * @brief  ARM Cortex-M DWT(Data Watchpoint and Trace) 사이클 카운터 활성화
 */
void bspInit(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief  ARM Cortex-M DWT 기반 초정밀 마이크로초(us) 딜레이 함수
 */
void delayUs(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (SystemCoreClock / 1000000);
  while ((DWT->CYCCNT - start) < ticks);
}
