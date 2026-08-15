#include "myI2c.h"

void i2cInit(void)
{
  // 필요한 I2C 초기화 작업 (CubeMX에서 MX_I2C1_Init 호출 후 추가 작업 필요 시)
}

void i2cScan(void)
{
  uint8_t count = 0;

  printf("\r\n==================================\r\n");
  printf("     STM32 I2C Bus Scanner        \r\n");
  printf("==================================\r\n");
  printf("Scanning I2C1 bus...\r\n\r\n");

  /* 7비트 유효 주소 범위: 0x01 ~ 0x77 (1 ~ 119) */
  for (uint8_t i = 1; i < 128; i++)
  {
    /* HAL은 8비트 주소 체계를 사용하므로 (i << 1) 전달 */
    HAL_StatusTypeDef result = HAL_I2C_IsDeviceReady(&hi2c1, (uint8_t)(i << 1), 1, 10);

    if (result == HAL_OK)
    {
      printf(" [*] Device Found at 7-bit Addr: 0x%02X (HAL 8-bit: 0x%02X)\r\n", i, (i << 1));
      count++;
    }
  }

  if (count == 0)
  {
    printf(" [!] No I2C devices found.\r\n");
    printf("     Check wiring, pull-up resistors, and power supply.\r\n");
  }
  else
  {
    printf("\r\nDone! Total %d device(s) found.\r\n", count);
  }
  printf("==================================\r\n\r\n");
}
