// Wire the two CAN's together to use this test. On the first iteration, it will test the CAN Forwarding function. Screen UART.
// MUST UNCOMMENT LINE IN CAN FORWARDING TASK THAT ENABLES BENCHTOP TESTING

#include "common.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "DebugPrintf.h"
#include "BPS_Tasks.h"

StaticTask_t task_buffer;
StackType_t task_stack[TEST_TASK_STACK_SIZE];

#define can_delay_ms 10

static bool verifyData(uint8_t tx[], uint8_t rx[]) {
    for (uint8_t i = 0; i < 8; i++) {
        if (tx[i] != rx[i]) return false;
    }
    return true;
}

static void task(void *pvParameters) {

    debugPrintf_init();
    printf("printf initialized\r\n");
    CAN_Init();
    printf("CAN initialized successfully\r\n");

    int test_id = 0x321;

    bool first_iteration = true;

    // send x1234 to 0x11
    uint8_t tx_data[8] = {0};
    tx_data[0] = 0x12;
    tx_data[1] = 0x34;
    tx_data[2] = 0x56;
    tx_data[3] = 0x78;
    tx_data[4] = 0x9A;
    tx_data[5] = 0xBC;
    tx_data[6] = 0xDE;
    tx_data[7] = 0xFF;

    uint8_t fdcan1_rx_data[8] = {0};

    uint8_t fdcan1_rx_bounceback_data[8] = {0};

    uint8_t fdcan3_rx_data[8] = {0};

    while(1){

    if (bps_can_send(test_id, tx_data, FDCAN_DLC_BYTES_8, can_delay_ms) == CAN_ERR){
        printf("BPS CAN failed to send!\r\n");
        Error_Handler();
    }
    printf("BPS CAN Sent successfully!\r\n");

    vTaskDelay(pdMS_TO_TICKS(20));

    if((car_can_recv(test_id, tx_data, FDCAN_DLC_BYTES_8, can_delay_ms) != CAN_OK) && verifyData(fdcan1_rx_data, tx_data)){
        printf("CAR CAN failed to receive!\r\n");
        Error_Handler();
    }
    printf("CAR CAN Receieve successfully!\r\n");

    if (car_can_send(test_id, fdcan3_rx_data, FDCAN_DLC_BYTES_8, can_delay_ms) == CAN_ERR){
        printf("CAR CAN failed to send!\r\n");
        Error_Handler();
    }
    printf("CAR CAN Sent successfully!\r\n");

    vTaskDelay(pdMS_TO_TICKS(20));

    if((bps_can_recv(test_id, fdcan1_rx_data, FDCAN_DLC_BYTES_8, can_delay_ms) != CAN_OK) && verifyData(fdcan1_rx_data, tx_data)){
        printf("BPS CAN failed to receive!\r\n");
        Error_Handler();
    }
    printf("BPS CAN Receieve successfully!\r\n");

    vTaskDelay(pdMS_TO_TICKS(20));

    if (first_iteration) {
        if((bps_can_recv(test_id, fdcan1_rx_bounceback_data, FDCAN_DLC_BYTES_8, can_delay_ms) != CAN_OK) && verifyData(fdcan1_rx_data, tx_data)){
            printf("Bounce Back (CAN forwarding) failed!\r\n");
            Error_Handler();
        }
        printf("Bounce Back (CAN forwarding) successfull!\r\n");
        first_iteration = false;
    }

    toggleHeartbeat();
    vTaskDelay(pdMS_TO_TICKS(1000));

    }
}

int main(void) {
    HAL_Init();

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    // System clock config can change depending on the target MCU, since the clock tree can be different
    // If you need to use a different MCU, go to cubemx and generate a new system clock config function with the system clock being 80 Mhz
    // It especially varies with nucleo vs direct MCU
    // G473_SystemClockConfig();

    SystemClock_Config();
 
    LEDs_init(); // enable LED for LED_PORT

    // you can only send CAN messages within a FreeRTOS task
    xTaskCreateStatic(
                task,
                "task",
                TEST_TASK_STACK_SIZE,
                NULL,
                TEST_TASK_PRIORITY,
                task_stack,
                &task_buffer);

    xTaskCreateStatic(
                Task_CanRxForward,
                "CAN Forward Task",
                TASK_CAN_FORWARD_STACK_SIZE,
                NULL,
                TASK_CAN_FORWARD_PRIO,
                Task_Can_Forward_Stack,
                &Task_Can_Forward_Buffer);

    vTaskStartScheduler();
    while(1){

    }
    
    Error_Handler();
    return 0;
}


