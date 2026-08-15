#include "apMain.h"
#include "myHw.h"
#include "usart.h"
#include "i2c.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  /* Polling 방식으로 1바이트 전송 (전송 완료될 때까지 대기) */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

/* printf() 출력을 USART2로 리디렉션 */
int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}

uint8_t rx_data;
uint8_t rx_buf[RX_BUF_SIZE];

/**
  * @brief  Application Main Entry Point (메인 루프)
  */
void apMain(void)
{
  while (1)
  {
    /* 메인 루프 처리 (추후 필요한 주기적 작업 추가 가능) */
    HAL_Delay(10);
  }
}

/* UART 수신 콜백 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    if (rx_data == 'a')
      printf("Hello STM32 Cortex-M4 USART Polling!\r\n");
    else
      HAL_UART_Transmit(&huart2, rx_buf, RX_BUF_SIZE, 100);

    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buf, RX_BUF_SIZE);
  }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance == USART2)
  {
    if (rx_data == 'a')
      printf("Hello STM32 Cortex-M4 USART Polling!\r\n");
    else
      HAL_UART_Transmit(&huart2, rx_buf, Size, 100);

    HAL_UART_DMAStop(&huart2);
    memset(rx_buf, 0, Size);
  }
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buf, RX_BUF_SIZE);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  HAL_UART_Receive_DMA(&huart2, rx_buf, RX_BUF_SIZE);
}

/* I2C 버스 스캔 함수 */
void I2C_Scan(I2C_HandleTypeDef *hi2c)
{
  uint8_t count = 0;

  printf("\r\n==================================\r\n");
  printf("     STM32 I2C Bus Scanner        \r\n");
  printf("==================================\r\n");
  printf("Scanning I2C bus...\r\n\r\n");

  /* 7비트 유효 주소 범위: 0x01 ~ 0x77 (1 ~ 119) */
  for (uint8_t i = 1; i < 128; i++)
  {
    /* HAL은 8비트 주소 체계를 사용하므로 (i << 1) 전달 
       - Trials: 1회 시도
       - Timeout: 10ms */
    HAL_StatusTypeDef result = HAL_I2C_IsDeviceReady(hi2c, (uint8_t)(i << 1), 1, 10);

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
