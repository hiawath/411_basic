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
    osDelay(50);
  }
}

void StartTaskDHT11(void *argument){
  while(1){
    adcUpdate();
    dht_status=dht11Read(&dht_data);
    internal_temp=adcGetTemp();

    lcd1602Clear();
    lcd1602Cursor(0, 0);
    lcd1602Printf("Temp %.2f/%.2f", internal_temp,dht_data.temperature);
    lcd1602Cursor(1, 0);
    lcd1602Printf("Humi %.2f", dht_data.humidity);

    printf(">Temp:%.2f\r\n", dht_data.temperature);
    printf(">Humi:%.2f\r\n", dht_data.humidity);

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