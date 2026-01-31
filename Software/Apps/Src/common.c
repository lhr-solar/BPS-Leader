#include "Contactors.h"
#include "StatusLEDs.h"
#include "common.h"


// for actual faults
void faultHandler() {

    // kill RTOS 
    __disable_irq();
    
    // open every contactor, bypasses semaphore
    for (uint8_t contactor_num = 0; contactor_num < NUM_CONTACTORS; contactor_num++) {
        setContactor(contactor_num, OPEN, BLOCKING, EMERGENCY);
    }
    
    // turns on fault led
    setFaultLED(FAULT_LED, ON);
    
}

// for software errors
void errorHandler() {

  // kill RTOS 
    __disable_irq();

  // open every contactor, bypasses semaphore
  for (uint8_t contactor_num = 0; contactor_num < NUM_CONTACTORS; contactor_num++) {
    setContactor(contactor_num, OPEN, BLOCKING, EMERGENCY);
  }
    
  // turns on fault led, and blink DEBUG led to show the error was software
  setFaultLED(FAULT_LED, ON);
  while (true) {
    setFaultLED(DEBUG_LED, ON);
    HAL_Delay(1000);
    setFaultLED(DEBUG_LED, OFF);
    HAL_Delay(1000);
  }
}
 

 void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}