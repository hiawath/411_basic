#include "apMain.h"
#include "myHw.h"
#include <stdio.h>

/**
  * @brief  Application Main Entry Point (메인 루프)
  */
void apMain(void)
{
  /* SSD1306 초기 화면 프레임 및 타이틀 출력 */
  ssd1306Clear();
  ssd1306DrawRect(0, 0, SSD1306_WIDTH, SSD1306_HEIGHT, SSD1306_COLOR_WHITE);
  ssd1306DrawString(16, 6, "STM32F411RE", SSD1306_COLOR_WHITE);
  ssd1306DrawLine(10, 18, 118, 18, SSD1306_COLOR_WHITE);
  ssd1306DrawString(8, 22, "Internal Temp(DMA)", SSD1306_COLOR_WHITE);
  ssd1306Update();

  /* DMA 방식 ADC 온도 측정 시작 (샘플링 주기: 500ms = 0.5초) */
  adcStartDMA(500);

  uint32_t prev_uptime_tick = HAL_GetTick();
  uint32_t uptime_sec = 0;
  char str_buf[32];

  while (1)
  {
    /* ADC 샘플링 주기 관리 및 DMA 완료 시 온도 계산 */
    adcUpdate();

    /* 1. adcStartDMA 설정 주기에 따라 새로운 온도가 측정/계산되었을 때 화면의 온도 갱신 */
    if (adcIsUpdated())
    {
      float temp = adcGetTemp();
      uint32_t raw = adcGetRaw();

      /* SSD1306 온도 표시 영역(y=36) 갱신 */
      ssd1306FillRect(8, 36, 112, 10, SSD1306_COLOR_BLACK);
      snprintf(str_buf, sizeof(str_buf), "Temp: %.1f C", temp);
      ssd1306DrawString(8, 36, str_buf, SSD1306_COLOR_WHITE);
      ssd1306Update();

      /* UART 로그 출력 */
      printf("[ADC DMA] Raw: %lu, Temp: %.2f C\r\n", (unsigned long)raw, temp);
    }

    /* 2. 가동 시간(Uptime)은 1초마다 독립적으로 갱신 */
    if (HAL_GetTick() - prev_uptime_tick >= 1000)
    {
      prev_uptime_tick = HAL_GetTick();
      uptime_sec++;

      /* SSD1306 가동 시간 표시 영역(y=48) 갱신 */
      ssd1306FillRect(8, 48, 112, 10, SSD1306_COLOR_BLACK);
      snprintf(str_buf, sizeof(str_buf), "Uptime: %lus", (unsigned long)uptime_sec);
      ssd1306DrawString(8, 48, str_buf, SSD1306_COLOR_WHITE);
      ssd1306Update();

      /* UART 로그 출력 */
      printf("[SYS] Uptime: %lus\r\n", (unsigned long)uptime_sec);
    }

    HAL_Delay(5);
  }
}
