        vTaskDelay(pdMS_TO_TICKS(VOLT_MONITOR_TASK_DELAY_MS) / 32);
        
        volt_can_pack(can_message, &message_struct);

        if (car_can_send(can_id, can_message, CAN_DLC_BPS_VT0_VOLTAGE_ARR, 10) != CAN_OK) {
            set_faultBit(BATTERY_OVERVOLTAGE_FAULT);
        }
        
        (message_struct.BPS_Tap_idx == 31) ? (message_struct.BPS_Tap_idx = 0) : message_struct.BPS_Tap_idx++;
        
        can_id = message_struct.BPS_Tap_idx / 4 + 2;
