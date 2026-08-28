#include "apMain.h"
#include "cmsis_os2.h"
#include "myHcSr04.h"
#include "myLcd1602.h"
#include "myAdc.h"
#include "myUart.h"
#include "myDht11.h"
#include "myI2c.h"
#include "stm32f411xe.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"
#include "myMpu6050.h"
#include "mySsd1306.h"
#include "myDs1302.h"
#include "stm32f4xx_hal_gpio.h"
#include "tim.h"
#include "myGpio.h"
#include "myTimer.h"
#include "myMcp2515.h"
#include <stdatomic.h>


#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TARGET_COUNT 1000000
#define HALF_COUNT   (TARGET_COUNT / 2)

/* 공유 전역 변수 */
volatile uint32_t g_shared_counter = 0;
volatile uint8_t task1_done = 0;
volatile uint8_t task2_done = 0;

osMutexId_t counterMutexHandle;
const osMutexAttr_t counterMutex_attributes = {
    .name = "counterMutex"
};


static mpu6050Data_t mpu_data={0};
extern ADC_HandleTypeDef hadc1;

static ds1302Time_t rtc_time={0};

static void canRxCallback(const mcp2515Msg_t *msg) {
  /* 인터럽트 발생 시 호출되는 콜백 */
  // printf in ISR or light-weight notification
}

void apInit(void) { 
  uartInit();
  adcInit();
  dht11Init();
  lcd1602Init();
  i2cScan();
  mpu6050Init();
  //ssd1306Init();
  ds1302Init();
  timerInit();

  /* MCP2515 CAN 컨트롤러 초기화 (500kbps, 8MHz 크리스탈) */
  if (mcp2515Init()) {
    printf("[CAN] MCP2515 Init Success!\r\n");
    mcp2515SetRxCallback(canRxCallback);
  } else {
    printf("[CAN] MCP2515 Init Failed!\r\n");
  }
}

float internal_temp=0;
dht11Data_t dht_data={0};
bool dht_status=false;
float distance_cm=0.0f;

void apMain(void) {

  uint32_t tick_1000=0;
  uint32_t tick_250=0;
  uint32_t tick_100=0;
  uint32_t tick_50=0;
  uint32_t tick_10=0;
  uint32_t current_tick=0;

  // ssd1306Clear();
  // ssd1306DrawRect(0, 0, SSD1306_WIDTH, SSD1306_HEIGHT, SSD1306_COLOR_WHITE);
  // ssd1306DrawString(8, 3, "STM32 MULTI-SENSOR", SSD1306_COLOR_WHITE);
  // ssd1306DrawLine(4, 13, 124,13, SSD1306_COLOR_WHITE);
  // ssd1306Update();

  // HAL_TIM_Base_Start_IT(&htim3);
  // HAL_TIM_Base_Start_IT(&htim4);



 
  //  timerSetDuty(50);

  while (1) {
    current_tick=HAL_GetTick();
    timerLedUpdate();

    /* CAN 인터럽트 수신 큐(FIFO) 처리 */
    mcp2515Msg_t rx_msg;
    while (mcp2515GetRxFifo(&rx_msg)) {
      printf("[CAN RX] ID: 0x%03lX (Ext:%d, RTR:%d), DLC: %d, Data: ",
             (unsigned long)rx_msg.id, rx_msg.is_ext, rx_msg.is_rtr, rx_msg.dlc);
      for (uint8_t i = 0; i < rx_msg.dlc; i++) {
        printf("%02X ", rx_msg.data[i]);
      }
      printf("\r\n");
    }

    if(current_tick-tick_1000>=1000){
      tick_1000=current_tick;
    }

    if(current_tick-tick_250>=250){
      tick_250=current_tick;
    }

    if(current_tick-tick_100>=100){
      tick_100=current_tick;
      if(mpu6050Read(&mpu_data)){
        printf(">acc_x:%.3f\r\n>acc_y:%.3f\r\n>acc_z:%.3f\r\n>gyro_x:%.3f\r\n>gyro_y:%.3f\r\n>gyro_z:%.3f\r\n",
          mpu_data.accel_x,mpu_data.accel_y,mpu_data.accel_z,mpu_data.gyro_x, mpu_data.gyro_y, mpu_data.gyro_z
        );
      }

    }
    if(current_tick-tick_50>=50){
      tick_50=current_tick;
    }

    if(current_tick-tick_10>=10){
      tick_10=current_tick;

      
      /* CAN 송신: 1초 주기로 count 데이터 전송 */
      static uint32_t can_tx_count = 0;
      mcp2515Msg_t tx_msg;
      tx_msg.id = 0x123;
      tx_msg.is_ext = false;
      tx_msg.is_rtr = false;
      tx_msg.dlc = 8;
      tx_msg.data[0] = (uint8_t)(can_tx_count >> 24);
      tx_msg.data[1] = (uint8_t)(can_tx_count >> 16);
      tx_msg.data[2] = (uint8_t)(can_tx_count >> 8);
      tx_msg.data[3] = (uint8_t)(can_tx_count & 0xFF);
      tx_msg.data[4] = 0xAA;
      tx_msg.data[5] = 0xBB;
      tx_msg.data[6] = 0xCC;
      tx_msg.data[7] = 0xDD;

      if (mcp2515SendMessage(&tx_msg)) {
        printf("[CAN TX] ID: 0x%03lX, Count: %lu, Data: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
               (unsigned long)tx_msg.id, (unsigned long)can_tx_count,
               tx_msg.data[0], tx_msg.data[1], tx_msg.data[2], tx_msg.data[3],
               tx_msg.data[4], tx_msg.data[5], tx_msg.data[6], tx_msg.data[7]);
      } else {
        printf("[CAN TX] Send Failed!\r\n");
      }
      can_tx_count++;
    }

  }
}
void StartDefaultTask(void *argument){
    apInit();

    // 뮤텍스 동적 생성
    counterMutexHandle = osMutexNew(&counterMutex_attributes);
    if (counterMutexHandle == NULL)
    {
        // 뮤텍스 생성 실패 처리 (메모리 부족 등)
        Error_Handler();
    }


    while(1){

      if (task1_done && task2_done)
      {
          printf("\r\n==================================\r\n");
          printf("Expected Target : %lu\r\n", (long unsigned int)TARGET_COUNT);
          printf("Actual Result   : %lu\r\n", g_shared_counter);
          printf("Loss Count      : %lu\r\n", TARGET_COUNT - g_shared_counter);
          printf("==================================\r\n");

          osThreadExit();
      }
      osDelay(1000);
    }

}


void StartTaskLED(void *argument){
  while(1){
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    ds1302GetDateTime(&rtc_time);
    //printf("sec : %d\r\n",rtc_time.sec);
    osDelay(500);
  }


}
void StartTaskCLI(void *argument){

  apMain();

}
void StartTaskHCS04(void *argument){
  while(1){
    hcSr04Read(&distance_cm);
    printf(">hcs:%6.1f\r\n",distance_cm);
    osDelay(250);
  }
}

void StartTaskDHT11(void *argument){
  /* 태스크 시작 시 1회만 화면 초기화 */
  lcd1602Clear();

  char line_buf[17];

  while(1){
    adcUpdate();
    dht_status=dht11Read(&dht_data);
    internal_temp=adcGetTemp();

    /* 1번째 줄: 16칸 고정 너비로 덮어쓰기 (공백 패딩) */
    snprintf(line_buf, sizeof(line_buf), "Temp: %4.1f/%4.1f ", internal_temp, dht_data.temperature);
    lcd1602Cursor(0, 0);
    lcd1602Print(line_buf);

    /* 2번째 줄: 16칸 고정 너비로 덮어쓰기 (공백 패딩) */
    snprintf(line_buf, sizeof(line_buf), "Humi: %4.1f%%     ", dht_data.humidity);
    lcd1602Cursor(1, 0);
    lcd1602Print(line_buf);

    printf(">Temp:%.2f\r\n", dht_data.temperature);
    printf(">Humi:%.2f\r\n", dht_data.humidity);

    /* DHT11 측정 주기 및 LCD 안정성에 맞추어 1초 대기 */
    osDelay(250);
  }

}


/* Task 1: 50만 번 증가 */
void StartTask01(void *argument){
    for (uint32_t i = 0; i < HALF_COUNT; i++)
	  {
      //osMutexAcquire(counterMutexHandle, osWaitForever);
      //g_shared_counter++; // [비원자적 연산] Read -> Modify -> Write
      //osMutexRelease(counterMutexHandle);
      atomic_fetch_add(&g_shared_counter, 1);
    }
    task1_done = 1;
    osThreadExit();
}

/* Task 2: 50만 번 증가 */
void StartTask02(void *argument){
    for (uint32_t i = 0; i < HALF_COUNT; i++)
    {
      // osMutexAcquire(counterMutexHandle, osWaitForever);
      //   g_shared_counter++; // [비원자적 연산] Read -> Modify -> Write
      // osMutexRelease(counterMutexHandle);
      atomic_fetch_add(&g_shared_counter, 1);
    }
    task2_done = 1;
    osThreadExit();
}