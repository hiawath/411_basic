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
  ssd1306DrawString(12, 4, "STM32 RTC DS1302", SSD1306_COLOR_WHITE);
  ssd1306DrawLine(6, 14, 122, 14, SSD1306_COLOR_WHITE);
  ssd1306Update();

  /* DMA 방식 ADC 온도 측정 시작 (샘플링 주기: 500ms = 0.5초) */
  adcStartDMA(500);

  uint32_t prev_1s_tick = HAL_GetTick();
  uint32_t uptime_sec = 0;
  char str_buf[32];
  ds1302Time_t rtc_time = {0};

  while (1)
  {
    /* ADC 샘플링 주기 관리 및 DMA 완료 시 온도 계산 */
    adcUpdate();

    /* 1. adcStartDMA 설정 주기(0.5초 등)로 새 온도가 측정되면 즉시 화면의 온도 영역 갱신 */
    if (adcIsUpdated())
    {
      float temp = adcGetTemp();

      ssd1306FillRect(8, 42, 112, 9, SSD1306_COLOR_BLACK);
      snprintf(str_buf, sizeof(str_buf), "Temp: %.1f C", temp);
      ssd1306DrawString(8, 42, str_buf, SSD1306_COLOR_WHITE);
      ssd1306Update();
    }

    /* 2. 1초마다 DS1302 날짜/시간 및 가동 시간(Uptime) 갱신 */
    if (HAL_GetTick() - prev_1s_tick >= 1000)
    {
      prev_1s_tick = HAL_GetTick();
      uptime_sec++;

      /* DS1302 실시간 시계 읽기 */
      ds1302GetDateTime(&rtc_time);

      /* SSD1306 날짜 표시 (y=18) */
      ssd1306FillRect(8, 18, 112, 9, SSD1306_COLOR_BLACK);
      snprintf(str_buf, sizeof(str_buf), "%04d-%02d-%02d (%s)", 
               rtc_time.year, rtc_time.month, rtc_time.day, ds1302GetDayStr(rtc_time.day_of_week));
      ssd1306DrawString(8, 18, str_buf, SSD1306_COLOR_WHITE);

      /* SSD1306 시간 표시 (y=30) */
      ssd1306FillRect(8, 30, 112, 9, SSD1306_COLOR_BLACK);
      snprintf(str_buf, sizeof(str_buf), "Time: %02d:%02d:%02d", 
               rtc_time.hour, rtc_time.min, rtc_time.sec);
      ssd1306DrawString(8, 30, str_buf, SSD1306_COLOR_WHITE);

      /* SSD1306 가동 시간 표시 (y=53) */
      ssd1306FillRect(8, 53, 112, 9, SSD1306_COLOR_BLACK);
      snprintf(str_buf, sizeof(str_buf), "Uptime: %lus", (unsigned long)uptime_sec);
      ssd1306DrawString(8, 53, str_buf, SSD1306_COLOR_WHITE);

      ssd1306Update();

      /* UART 로그 출력 */
      printf("[RTC] %04d-%02d-%02d (%s) %02d:%02d:%02d | Temp: %.2f C | Uptime: %lus\r\n",
             rtc_time.year, rtc_time.month, rtc_time.day, ds1302GetDayStr(rtc_time.day_of_week),
             rtc_time.hour, rtc_time.min, rtc_time.sec,
             adcGetTemp(), (unsigned long)uptime_sec);
    }

    HAL_Delay(5);
  }
}
