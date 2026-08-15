#include "apInit.h"
#include "apMain.h"
#include "myHw.h"
#include "i2c.h"
#include "usart.h"

void apInit(void)
{
  /* 하드웨어 레이어 초기화 */
  myHwInit();

  /* UART DMA 수신 설정 */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buf, RX_BUF_SIZE);

  /* 보드 부팅 및 I2C 버스 안정화를 위해 100ms 대기 */
  HAL_Delay(100);

  /* I2C1 버스 스캔 실행 */
  I2C_Scan(&hi2c1);
}