#include "stm32xx_hal.h"
#include "CAN.h"
#include "CAN_Filter.h"

StaticTask_t task_buffer;
StackType_t task_stack[configMINIMAL_STACK_SIZE];

static void error_handler(void) {
  while(1) {}
}

static void success_handler(void) {
  GPIO_InitTypeDef led_init = {
    .Mode = GPIO_MODE_OUTPUT_PP,
    .Pull = GPIO_NOPULL,
    .Pin = GPIO_PIN_5
  };
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_GPIO_Init(GPIOA, &led_init);

  while(1){
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(500);
  }
}

#define TEST_ID_1 0x1
#define TEST_ID_2 0x3 // Undefine if you only want to test with 1 ID

// Sends two CAN frames with ID 0x1 and ID 0x3 respectively.
// If both are received correctly, blink LED - else, no LED.
static void task(void *pvParameters) {
  /* ==== USING CAN1 ==== */
  // create payload to send
  CAN_TxHeaderTypeDef tx_header = {0};   
  tx_header.StdId = TEST_ID_1;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.IDE = CAN_ID_STD;
  tx_header.DLC = 2;
  tx_header.TransmitGlobalTime = DISABLE;

  // send payload with first ID
  uint8_t tx_data[8] = {0};
  tx_data[0] = 0x01;
  tx_data[1] = 0x00;
  if (can_send(hcan1, &tx_header, tx_data, portMAX_DELAY) != CAN_SENT) error_handler();

  #ifdef TEST_ID_2
    // send two payloads with second ID
    tx_data[0] = 0x03;
    tx_header.StdId = TEST_ID_2;
    if (can_send(hcan1, &tx_header, tx_data, portMAX_DELAY) != CAN_SENT) error_handler();
  #endif

  // receive
  CAN_RxHeaderTypeDef rx_header = {0};
  uint8_t rx_data[8] = {0};
  can_status_t status;
  
  // receive what was sent to first ID
  status = can_recv(hcan1, TEST_ID_1, &rx_header, rx_data, portMAX_DELAY);
  if (status != CAN_RECV && rx_data[0] != 0x1) error_handler();
  #ifdef TEST_ID_2
    // receive what was sent to second ID
    status = can_recv(hcan1, TEST_ID_2, &rx_header, rx_data, portMAX_DELAY);
    if (status != CAN_RECV && rx_data[0] != 0x3) error_handler();
  #endif

  // make sure we don't receive from wrong ID and nonblocking works
  status = can_recv(hcan1, TEST_ID_1, &rx_header, rx_data, 0);
  if (status != CAN_EMPTY) error_handler();
  #ifdef TEST_ID_2
    status = can_recv(hcan1, TEST_ID_2, &rx_header, rx_data, 0);
    if (status != CAN_EMPTY) error_handler();
  #endif

  success_handler();
}


int main(void) {
  // initialize the HAL and system clock
  if (HAL_Init() != HAL_OK) error_handler();
  // SystemClock_Config();

  /* ---- FILTER TESTS ---- */ 
  CAN_FilterTypeDef  sFilterConfig = {0};
  
  /* ---- MASK MODE Tests ---- */
  // MASK MODE: Accept all IDs (pass in 0)              - blink LED for success
  // CAN_Filter_Mask_Init(&sFilterConfig, 0, 0);

  // MASK MODE: Filter only for TEST_ID_1 using Mask    - no LED (fail)
  // CAN_Filter_Mask_Init(&sFilterConfig, TEST_ID_1, 0xFFFF);

  // MASK MODE: Filter for a small range: 0x2 or 0x3    - no LED (fail) but a cool example
  // CAN_Filter_Mask_Init(&sFilterConfig, 0x2, 0xFFFE);  // mask = 0xFFFE means don't care about bit 0

  // MASK MODE: Filter for an ID we do not use          - no LED (fail)
  // CAN_Filter_Mask_Init(&sFilterConfig, TEST_ID_1 + 0x5, 0xFF);


  /* ---- LIST MODE Tests ---- */
  // LIST MODE: Filter for correct IDs using List mode  - blink LED for success
  uint16_t ids[2] = {TEST_ID_1, TEST_ID_2};
  CAN_Filter_List_Init(&sFilterConfig, ids, 2);

  // LIST MODE: Filter for different IDs using List     - no LED (fail)
  // uint16_t ids[2] = {TEST_ID_1 + 0x1, TEST_ID_2 + 0x1};
  // CAN_Filter_List_Init(&sFilterConfig, ids, 2);
  
  // LIST MODE: Filter for just 1 of the 2 IDS          - no LED (fail)
  // uint16_t ids[1] = {TEST_ID_1};
  // CAN_Filter_List_Init(&sFilterConfig, ids, 1);

  // setup can1 init
  hcan1->Init.Prescaler = 5;
  hcan1->Init.Mode = CAN_MODE_LOOPBACK;
  hcan1->Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1->Init.TimeSeg1 = CAN_BS1_6TQ;
  hcan1->Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1->Init.TimeTriggeredMode = DISABLE;
  hcan1->Init.AutoBusOff = DISABLE;
  hcan1->Init.AutoWakeUp = DISABLE;
  hcan1->Init.AutoRetransmission = ENABLE;
  hcan1->Init.ReceiveFifoLocked = DISABLE;
  hcan1->Init.TransmitFifoPriority = DISABLE;

  // initialize CAN1
  if (can_init(hcan1, &sFilterConfig) != CAN_OK) error_handler();
  if (can_start(hcan1) != CAN_OK) error_handler();

  xTaskCreateStatic(
                task,
                "task",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY + 2,
                task_stack,
                &task_buffer);

  vTaskStartScheduler();

  error_handler();

  return 0;
}
