#include "apMain.h"
#include "myHw.h"
#include <stdio.h>

/**
  * @brief  Application Main Entry Point (메인 루프)
  */
void apMain(void)
{
  /* SSD1306 화면 테스트 프로그램 호출 */
  ssd1306Test();

  uint32_t prev_tick = HAL_GetTick();
  uint32_t count = 0;
  char str_buf[32];

  while (1)
  {
    /* 1초마다 화면 하단 카운터 갱신 */
    if (HAL_GetTick() - prev_tick >= 1000)
    {
      prev_tick = HAL_GetTick();
      count++;

      // 카운터 영역 지우고 업데이트
      ssd1306FillRect(8, 48, 112, 10, SSD1306_COLOR_BLACK);
      snprintf(str_buf, sizeof(str_buf), "Uptime: %lus", (unsigned long)count);
      ssd1306DrawString(8, 48, str_buf, SSD1306_COLOR_WHITE);
      ssd1306Update();
    }

    HAL_Delay(10);
  }
}
