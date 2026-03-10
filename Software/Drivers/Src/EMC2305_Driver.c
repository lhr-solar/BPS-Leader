#include "EMC2305_Driver.h"

EMC2305_HandleTypeDef chip;

I2C_HandleTypeDef hi2c3;

void EMC2305_I2C_init(void)
{ 

  GPIO_InitTypeDef init = {0};
  
   RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C3;
    PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

  __HAL_RCC_I2C3_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  init.Pin = GPIO_PIN_8|GPIO_PIN_9;
  init.Mode = GPIO_MODE_AF_OD;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  init.Alternate = GPIO_AF8_I2C3;
  HAL_GPIO_Init(GPIOC, &init);


  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x00503D58;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_NVIC_SetPriority(I2C3_EV_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1, 0);
  HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);

  HAL_NVIC_SetPriority(I2C3_ER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1, 0);
  HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);

}

static void fan_init(EMC2305_Fan fan) {

    // Depends on the fan lol (should be in fan datasheet)
    if (EMC2305_SetPWMBaseFrequency(&chip, fan, EMC2305_PWM_19k53) != EMC2305_OK) {
        Error_Handler();
    };

    // Set minimum drive to 0%
    if (EMC2305_WriteReg(&chip, EMC2305_FAN_REG_ADDR(fan, EMC2305_REG_FAN1_MIN_DRIVE), 0x00) != EMC2305_OK) {
        Error_Handler();
    };

    // Set PID Gain to lowest (1x)
    if (EMC2305_WriteReg(&chip, EMC2305_FAN_REG_ADDR(fan, EMC2305_REG_GAIN1), 0x00) != EMC2305_OK) {
        Error_Handler();
    };

    // Set PWM output mode to open-drain (use false for push-pull)
    if (EMC2305_SetPWMOutputMode(&chip, fan, true) != EMC2305_OK) {
        Error_Handler();
    };
}

// CALL FROM TASK
void EMC2305_Driver_init() {

    vTaskDelay(pdMS_TO_TICKS(250));

    if (EMC2305_Init(&chip, &hi2c3, 0x4D) == EMC2305_ERR) {
        Error_Handler();
    };

    EMC2305_Global_Config cfg = {
        .alert_mask = false,
        .disable_smbus_timeout = true,
        .watchdog_enable = false,
        .drive_ext_clk = false,
        .use_ext_clk = false,
    };

    EMC2305_SetGlobalConfig(&chip, &cfg);

    EMC2305_Fan_Config1 cfg1 = {
        .enable_closed_loop = true,
        .range = EMC2305_RNG_2000,
        .edges = EMC2305_EDG_5,
        .update_time = EMC2305_UDT_100,
    };

    EMC2305_Fan_Config2 cfg2 = {
        .enable_ramp_rate_ctl = true,
        .enable_glitch_filter = true,
        .derivative_options = EMC2305_DPT_BOTH,
        .error_window = EMC2305_ERG_200RPM,
    };

    EMC2305_SetFanConfig(&chip, EMC2305_FAN1, &cfg1, &cfg2);
    EMC2305_SetFanConfig(&chip, EMC2305_FAN2, &cfg1, &cfg2);

    fan_init(EMC2305_FAN1);
    fan_init(EMC2305_FAN2);
}