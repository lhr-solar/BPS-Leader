#include "common.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "DebugPrintf.h"

StaticTask_t task_buffer;
StackType_t task_stack[512];

/*
configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY is the maximum FreeRTOS priority for an interrupt 
*/
#define FDCAN_NVIC_PRIO configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3

// This enables the can_fd_rx_callback_hook function to be called when data is recieved
#define FDCAN1_RECV_HOOK_EN
#define FDCAN3_RECV_HOOK_EN


// Initialize clock for heartbeat LED port

static void task(void *pvParameters) {

    debugPrintf_init();
    printf("printf initialized\n\r");
    ALL_CAN_Init();
    printf("CAN initialized successfully\n\r");

    bool toggle = true;
    int test_id = 0x321;
    FDCAN_TxHeaderTypeDef tx_header = {0};   
    tx_header.Identifier = test_id;
    tx_header.IdType = FDCAN_STANDARD_ID;
    tx_header.TxFrameType = FDCAN_DATA_FRAME;
    tx_header.DataLength = FDCAN_DLC_BYTES_8;
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    tx_header.FDFormat = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl = FDCAN_STORE_TX_EVENTS;
    tx_header.MessageMarker = 0;

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

    //FDCAN_RxHeaderTypeDef fdcan1_rx_header = {0};
    //uint8_t fdcan1_rx_data[8] = {0};

    // FDCAN_RxHeaderTypeDef fdcan2_rx_header = {0};
    // uint8_t fdcan2_rx_data[8] = {0};

    FDCAN_RxHeaderTypeDef fdcan3_rx_header = {0};
    uint8_t fdcan3_rx_data[8] = {0};

    while(1){

    if (can_fd_send(hfdcan1, &tx_header, tx_data, pdMS_TO_TICKS(20)) == CAN_ERR){
        printf("FDCAN1 failed to send!\n\r");
        Error_Handler();
    }
    printf("FDCAN1 Sent successfully!\n\r");
    /*/
    vTaskDelay(pdMS_TO_TICKS(1000));

        if (can_fd_send(hfdcan3, &tx_header, tx_data, pdMS_TO_TICKS(20)) == CAN_ERR){
        printf("FDCAN3 failed to send!\n\r");
        Error_Handler();
    }
    printf("FDCAN3 Sent successfully!\n\r");
    */
    vTaskDelay(pdMS_TO_TICKS(20));
    if(can_fd_recv(hfdcan3, test_id, &fdcan3_rx_header, fdcan3_rx_data, pdMS_TO_TICKS(20)) != CAN_OK){
        printf("FDCAN3 failed to receive!\n\r");
        Error_Handler();
    }
    printf("FDCAN1 Receieve successfully!\n\r");
    /*
    for(uint8_t i = 0; i < 8; i++){
        if(fdcan1_rx_data[i] != tx_data[i]){
            printf("FDCAN1 Data dont match!\n\r");
            Error_Handler();
        }
    }
    printf("FDCAN1 Data works!\n\r");   

    if (can_fd_send(hfdcan3, &tx_header, tx_data, portMAX_DELAY) == CAN_ERR){
        printf("FDCAN3 failed to send!\n\r");
        Error_Handler();
    }
    printf("FDCAN3 Sent successfully!\n\r");

    if(can_fd_recv(hfdcan3, test_id, &fdcan3_rx_header, fdcan3_rx_data, portMAX_DELAY) != CAN_OK){
        printf("FDCAN3 failed to receive!\n\r");
        Error_Handler();
    }
    printf("FDCAN3 Receieve successfully!\n\r");

    for(uint8_t i = 0; i < 8; i++){
        if(fdcan3_rx_data[i] != tx_data[i]){
            printf("FDCAN3 Data dont match!\n\r");
            Error_Handler();
        }
    }
    printf("FDCAN3 Data works!\n\r");   
    */
    
    setHeartbeat(toggle);
    toggle = !toggle;
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
                512,
                NULL,
                tskIDLE_PRIORITY + 2,
                task_stack,
                &task_buffer);

    
    vTaskStartScheduler();
    while(1){

    }
    
    Error_Handler();
    return 0;
}


