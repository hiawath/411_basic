#include "apMain.h"
#include "myHw.h"
#include <stdio.h>

/**
  * @brief  Application Main Entry Point (메인 루프)
  */
void apMain(void)
{
  /* SSD1306 초기 화면 출력 */
  ssd1306Clear();
  ssd1306DrawRect(0, 0, SSD1306_WIDTH, SSD1306_HEIGHT, SSD1306_COLOR_WHITE);
  ssd1306DrawString(16, 6, "STM32F411RE", SSD1306_COLOR_WHITE);
  ssd1306DrawLine(10, 18, 118, 18, SSD1306_COLOR_WHITE);
  ssd1306DrawString(8, 22, "Internal Temp (IT)", SSD1306_COLOR_WHITE);
  ssd1306Update();

  /* 인터럽트 방식 ADC 온도 측정 시작 (기본 샘플링 주기: 500ms = 0.5초) */
  adcStartIT(500);

  uint32_t prev_tick = HAL_GetTick();
  uint32_t count = 0;
  char str_buf[32];

  while (1)
  {
    /* ADC 샘플링 주기 관리 (0.5초마다 인터럽트 변환 트리거) */
    adcUpdate();

    /* 1초마다 최신 측정 온도 값을 SSD1306 화면 및 UART로 전송 */
    if (HAL_GetTick() - prev_tick >= 1000)
    {
      prev_tick = HAL_GetTick();
      count++;

      /* 인터럽트 콜백에서 갱신된 최신 온도 및 Raw 값 취득 */
      float temp = adcGetTemp();
      uint32_t raw = adcGetRaw();

      /* UART printf 출력 */
      printf("[ADC IT] Raw: %lu, Temp: %.2f C, Uptime: %lus\r\n", (unsigned long)raw, temp, (unsigned long)count);

      /* SSD1306 화면 영역 갱신 */
      // 1. 온도 표시 (y=36)
      ssd1306FillRect(8, 36, 112, 10, SSD1306_COLOR_BLACK);
      snprintf(str_buf, sizeof(str_buf), "Temp: %.1f C", temp);
      ssd1306DrawString(8, 36, str_buf, SSD1306_COLOR_WHITE);

      // 2. 가동 시간 표시 (y=48)
      ssd1306FillRect(8, 48, 112, 10, SSD1306_COLOR_BLACK);
      snprintf(str_buf, sizeof(str_buf), "Uptime: %lus", (unsigned long)count);
      ssd1306DrawString(8, 48, str_buf, SSD1306_COLOR_WHITE);

      ssd1306Update();
    }

    HAL_Delay(5);
  }
}
