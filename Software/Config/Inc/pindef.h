#pragma once
/* File holds all pin, port, and channel definitions*/

/**
 * @brief object that holds pin num and port of a GPIO pin
 */
typedef struct {
    GPIO_TypeDef* port; // e.g., GPIOA
    uint16_t      pin;  // e.g., GPIO_PIN_3
} GpioPin_t;



// Strobe Connection (+3.3V)
#define STROBE_PIN  GPIO_PIN_12
#define STROBE_PORT GPIOC


// Indication Connection 
#define INDICATION_PIN  GPIO_PIN_2
#define INDICATION_PORT GPIOD

// Heartbeat LED
#define HEARTBEAT_LED_PIN  GPIO_PIN_3
#define HEARTBEAT_LED_PORT GPIOC


// Fan Cip
//------------------------------------------
#define FAN_SCL_PIN GPIO_PIN_8 
#define FAN_SCL_PORT GPIOC

#define FAN_SDA_PIN GPIO_PIN_9
#define FAN_SDA_PORT GPIOC

#define FAN_ALERT_PIN GPIO_PIN_0
#define FAN_ALERT_PORT GPIOC

#define FAN_I2C_CHANNEL I2C3
//------------------------------------------


// SHT4x
//------------------------------------------
#define SHT4x_SCL_PIN GPIO_PIN_6 
#define SHT4x_SCL_PORT GPIOC

#define SHT4x_SDA_PIN GPIO_PIN_7
#define SHT4x_SDA_PORT GPIOC

#define SHT4x_I2C_CHANNEL I2C4
//------------------------------------------


// ESP32
//------------------------------------------
#define ESP_RESET_PIN GPIO_PIN_1 
#define ESP_RESET_PORT GPIOB

#define ESP_UART_CHANNEL LPUART1
//------------------------------------------


// Status LED Shift-Reg PINDEF
//------------------------------------------
#define LED_SER_PIN GPIO_PIN_13  
#define LED_SER_PORT GPIOB

#define LED_RCLK_PIN GPIO_PIN_15 
#define LED_RCLK_PORT GPIOB

#define LED_SRCLK_PIN GPIO_PIN_14
#define LED_SRCLK_PORT GPIOB
//------------------------------------------


// ADC Precharge V
//------------------------------------------
#define ADC_PORT GPIOB
#define ADC1_PIN GPIO_PIN_12
#define ADC2_PIN GPIO_PIN_2
//------------------------------------------


// Car and BPS CAN 
//------------------------------------------
#define BPS_CAN_CHANNEL FDCAN1

#define CAR_CAN_CHANNEL FDCAN3
//------------------------------------------


// HV+ Contactor
//------------------------------------------
#define HV_PLUS_CONTROL_PIN GPIO_PIN_1
#define HV_PLUS_CONTROL_PORT GPIOA

#define HV_PLUS_SENSE_PIN GPIO_PIN_0
#define HV_PLUS_SENSE_PORT GPIOB
//------------------------------------------


// HV- Contactor
//------------------------------------------
#define HV_MINUS_CONTROL_PIN GPIO_PIN_0
#define HV_MINUS_CONTROL_PORT GPIOA

#define HV_MINUS_SENSE_PIN GPIO_PIN_3
#define HV_MINUS_SENSE_PORT GPIOA
//------------------------------------------


// Array Contactor
//------------------------------------------
#define ARRAY_CONTROL_PIN GPIO_PIN_2
#define ARRAY_CONTROL_PORT GPIOC

#define ARRAY_SENSE_PIN GPIO_PIN_2
#define ARRAY_SENSE_PORT GPIOA
//------------------------------------------


// Array Precharge Contactor
//------------------------------------------
#define ARRAY_PRE_CONTROL_PIN GPIO_PIN_9
#define ARRAY_PRE_CONTROL_PORT GPIOB            

#define ARRAY_PRE_SENSE_PIN GPIO_PIN_3
#define ARRAY_PRE_SENSE_PORT GPIOB
//------------------------------------------


// ESTOPs
//------------------------------------------
#define ESTOP1_PIN GPIO_PIN_5
#define ESTOP1_PORT GPIOB

#define ESTOP2_PIN GPIO_PIN_10
#define ESTOP2_PORT GPIOA

#define ESTOP3_PIN GPIO_PIN_4
#define ESTOP3_PORT GPIOB
//------------------------------------------






