    #include "common.h"
    #include "BPS_Tasks.h"
    #include "CarCAN_can_msgs.h"
    #include "Contactors.h"
    #include "CANbus.h"
    #include "StatusLEDs.h"
    #include <string.h>

    #define MODULES_PER_SEGMENT (NUM_BATTERY_MODULES / NUM_SEGMENTS)

    #define BPS_STATUS_CAN_DELAY_MS 10u
                                        
    // typedef struct {
    //     uint8_t BPS_Fault;
    //     uint8_t BPS_Charge_OK;
    //     uint8_t BPS_Regen_OK;
    //     uint8_t HV_Plus_Contactor_State;
    //     uint8_t HV_Minus_Contactor_State;
    //     uint8_t Array_Contactor_State;
    //     uint8_t Array_Precharge_Contactor_State;
    //     uint32_t Main_Battery_Voltage;
    //     int16_t Main_Battery_Avg_Temperature;
    //     uint8_t BPS_Segment0_Status;
    //     int8_t BPS_Segment1_Status;
    //     uint8_t BPS_Segment2_Status;
    //     uint8_t BPS_Segment3_Status;
    //     uint8_t BPS_Segment4_Status;
    //     uint8_t BPS_Segment5_Status;
    //     uint8_t BPS_Segment6_Status;
    //     uint8_t BPS_Segment7_Status;
    // } bps_status_t;

    // | BPS_Fault | BPS_Status | [0:7] | 8 | 1 | 0 | None | None |  |
    // | BPS_Charge_OK | BPS_Status | [8:8] | 1 | 1 | 0 | None | None |  |
    // | BPS_Regen_OK | BPS_Status | [9:9] | 1 | 1 | 0 | None | None |  |
    // | HV_Plus_Contactor_State | BPS_Status | [10:10] | 1 | 1 | 0 | None | None |  |
    // | HV_Minus_Contactor_State | BPS_Status | [11:11] | 1 | 1 | 0 | None | None |  |
    // | Array_Contactor_State | BPS_Status | [12:12] | 1 | 1 | 0 | None | None |  |
    // | Array_Precharge_Contactor_State | BPS_Status | [13:13] | 1 | 1 | 0 | None | None |  |
    // | Main_Battery_Voltage | BPS_Status | [16:39] | 24 | 0.001 | 0 | 0 | 16777.215 | V |
    // | Main_Battery_Avg_Temperature | BPS_Status | [40:55] | 16 | 0.01 | 0 | -327.68 | 327.67 | C |
    // | BPS_Segment0_Status | BPS_Status | [56:56] | 1 | 1 | 0 | None | None |  |
    // | BPS_Segment1_Status | BPS_Status | [57:57] | 1 | 1 | 0 | None | None |  |
    // | BPS_Segment2_Status | BPS_Status | [58:58] | 1 | 1 | 0 | None | None |  |
    // | BPS_Segment3_Status | BPS_Status | [59:59] | 1 | 1 | 0 | None | None |  |
    // | BPS_Segment4_Status | BPS_Status | [60:60] | 1 | 1 | 0 | None | None |  |
    // | BPS_Segment5_Status | BPS_Status | [61:61] | 1 | 1 | 0 | None | None |  |
    // | BPS_Segment6_Status | BPS_Status | [62:62] | 1 | 1 | 0 | None | None |  |
    // | BPS_Segment7_Status | BPS_Status | [63:63] | 1 | 1 | 0 | None | None |  |

    // typedef enum {
    //     BPS_STATUS_BPS_FAULT_DISCHARGING_OVERCURRENT = 17,
    //     BPS_STATUS_BPS_FAULT_CHARGING_OVERCURRENT = 16,
    //     BPS_STATUS_BPS_FAULT_ESTOP_3 = 15,
    //     BPS_STATUS_BPS_FAULT_ESTOP_2 = 14,
    //     BPS_STATUS_BPS_FAULT_ESTOP_1 = 13,
    //     BPS_STATUS_BPS_FAULT_ARRAY_PCHG_CONTACTOR_SENSE = 12,
    //     BPS_STATUS_BPS_FAULT_ARRAY_CONTACTOR_SENSE = 11,
    //     BPS_STATUS_BPS_FAULT_HV_MINUS_CONTACTOR_SENSE = 10,
    //     BPS_STATUS_BPS_FAULT_HV_PLUS_CONTACTOR_SENSE = 9,
    //     BPS_STATUS_BPS_FAULT_SEGMENT_WATCHDOG = 8,
    //     BPS_STATUS_BPS_FAULT_INTERNAL_WATCHDOG = 7,
    //     BPS_STATUS_BPS_FAULT_ARRAY_PRECHARGE_TIMEOUT = 6,
    //     BPS_STATUS_BPS_FAULT_ELCON = 5,
    //     BPS_STATUS_BPS_FAULT_OVERTEMPERATURE = 4,
    //     BPS_STATUS_BPS_FAULT_REGEN = 3,
    //     BPS_STATUS_BPS_FAULT_UNDERVOLTAGE = 2,
    //     BPS_STATUS_BPS_FAULT_OVERVOLTAGE = 1,
    //     BPS_STATUS_BPS_FAULT_OK = 0,
    //     BPS_STATUS_BPS_FAULT_AMPERES_WATCHDOG = 18,
    // } bps_status_bps_fault_e;

    // returns 1 if good else 0
    static uint8_t get_segment_status(uint8_t segment_num) {

        uint8_t segment_status = 0;

        if (((exposed_temp_sensor_bitmap >> (segment_num * MODULES_PER_SEGMENT)) & 0xF) != 0xF) segment_status = 1;
        if (((exposed_volt_sensor_bitmap >> (segment_num * MODULES_PER_SEGMENT)) & 0xF) != 0xF) segment_status = 1;

        for (uint8_t i = segment_num*MODULES_PER_SEGMENT; i < (segment_num+1)*MODULES_PER_SEGMENT; i++) {

            if (temp_can_data[i].BPS_Temperature_Tap_Fault != BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OK)  segment_status = 1;
            if (volt_can_data[i].BPS_Voltage_Tap_Fault != BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_OK)              segment_status = 1;
        }

        return segment_status;
    }

    static uint32_t get_pack_voltage() {

        uint32_t voltage_sum = 0;

        for (uint8_t module_num = 0; module_num < NUM_BATTERY_MODULES; module_num++) {
            voltage_sum += volt_can_data[module_num].BPS_Voltage_Tap_Data;
        }

        return voltage_sum;
    }


    static uint32_t get_avg_temp() {
        
        int32_t temp_sum = 0;

        for (uint8_t module_num = 0; module_num < NUM_BATTERY_MODULES; module_num++) {
            temp_sum += (temp_can_data[module_num].BPS_Temperature_Tap_Data/10);
        }

        return  (temp_sum / NUM_BATTERY_MODULES);
    }

    static void pack_bps_status_message(const bps_status_t *status, uint8_t *can_data) {
        // Byte 0: BPS_Fault [0:7]
        can_data[0] = status->BPS_Fault;

        // Byte 1: Contactor and OK States [8:13]
        // Bits 14 and 15 are unused, set to 0
        can_data[1] = 0x00;
        can_data[1] |= (status->BPS_Charge_OK & 0x01)                   << 0; // Bit 8
        can_data[1] |= (status->BPS_Regen_OK & 0x01)                    << 1; // Bit 9
        can_data[1] |= (status->HV_Plus_Contactor_State & 0x01)         << 2; // Bit 10
        can_data[1] |= (status->HV_Minus_Contactor_State & 0x01)        << 3; // Bit 11
        can_data[1] |= (status->Array_Contactor_State & 0x01)           << 4; // Bit 12
        can_data[1] |= (status->Array_Precharge_Contactor_State & 0x01) << 5; // Bit 13

        // Bytes 2, 3, 4: Main_Battery_Voltage [16:39] (24 bits)
        // Assuming little-endian packing
        can_data[2] = (uint8_t)(status->Main_Battery_Voltage & 0xFF);         // Bits 16-23
        can_data[3] = (uint8_t)((status->Main_Battery_Voltage >> 8) & 0xFF);  // Bits 24-31
        can_data[4] = (uint8_t)((status->Main_Battery_Voltage >> 16) & 0xFF); // Bits 32-39

        // Bytes 5, 6: Main_Battery_Avg_Temperature [40:55] (16 bits)
        // Cast to uint16_t to ensure safe bitwise shifting for signed integers
        uint16_t raw_temp = (uint16_t)status->Main_Battery_Avg_Temperature;
        can_data[5] = (uint8_t)(raw_temp & 0xFF);         // Bits 40-47
        can_data[6] = (uint8_t)((raw_temp >> 8) & 0xFF);  // Bits 48-55

        can_data[7] = status->BPS_Segment0_Status;
        can_data[7] |= status->BPS_Segment1_Status << 1;
        can_data[7] |= status->BPS_Segment2_Status << 2;
        can_data[7] |= status->BPS_Segment3_Status << 3;
        can_data[7] |= status->BPS_Segment4_Status << 4;
        can_data[7] |= status->BPS_Segment5_Status << 5;
        can_data[7] |= status->BPS_Segment6_Status << 6;
        can_data[7] |= status->BPS_Segment7_Status << 7;
    }


    void Task_Can_Status(void* pvParameters) {
        
        TickType_t xLastWakeTime = xTaskGetTickCount();

        bps_status_t bps_status_message = {0};
        uint8_t bps_status_raw_can[CAN_DLC_BPS_STATUS] = {0};
        uint32_t fault_bitmap = 0;

        while (1) {
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CAN_STATUS_TASK_DELAY_MS));

            fault_bitmap = (uint32_t)xEventGroupGetBits(faultBits);

            if ((fault_bitmap & FAULT_BIT(CELL_OVERVOLTAGE_FAULT)) != 0)                    bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_OVERVOLTAGE;
            else if ((fault_bitmap & FAULT_BIT(CELL_UNDERVOLTAGE_FAULT)) != 0)              bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_UNDERVOLTAGE;
            // PLACEHOLDER
            else if (false)                                                                 bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_REGEN;
            else if ((fault_bitmap & FAULT_BIT(CELL_OVERTEMP_FAULT)) != 0)                  bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_OVERTEMPERATURE;
            else if ((fault_bitmap & FAULT_BIT(ELCON_FAULT)) != 0)                          bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_ELCON;          
            else if ((fault_bitmap & FAULT_BIT(PRECHARGE_TIMEOUT_FAULT)) != 0)              bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_ARRAY_PRECHARGE_TIMEOUT;
            else if ((fault_bitmap & FAULT_BIT(RTOS_WATCHDOG_ERROR)) != 0)                  bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_INTERNAL_WATCHDOG;
            else if ((fault_bitmap & FAULT_BIT(BQ_CHIP_FAULT)) != 0)                        bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_SEGMENT_WATCHDOG;
            else if ((fault_bitmap & FAULT_BIT(CONTACTOR_HV_PLUS_FAULT)) != 0)              bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_HV_PLUS_CONTACTOR_SENSE;
            else if ((fault_bitmap & FAULT_BIT(CONTACTOR_HV_MINUS_FAULT)) != 0)             bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_HV_MINUS_CONTACTOR_SENSE;
            else if ((fault_bitmap & FAULT_BIT(CONTACTOR_ARRAY_FAULT)) != 0)                bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_ARRAY_CONTACTOR_SENSE;
            else if ((fault_bitmap & FAULT_BIT(CONTACTOR_ARRAY_PRE_FAULT)) != 0)            bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_ARRAY_PCHG_CONTACTOR_SENSE;
            else if ((fault_bitmap & FAULT_BIT(BPS_ESTOP1_FAULT)) != 0)                     bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_ESTOP_1;
            else if ((fault_bitmap & FAULT_BIT(BPS_ESTOP2_FAULT)) != 0)                     bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_ESTOP_2;
            else if ((fault_bitmap & FAULT_BIT(BPS_ESTOP3_FAULT)) != 0)                     bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_ESTOP_3;
            else if ((fault_bitmap & FAULT_BIT(PACK_OVERCURRENT_CHARGING_FAULT)) != 0)      bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_CHARGING_OVERCURRENT;
            else if ((fault_bitmap & FAULT_BIT(PACK_OVERCURRENT_DISCHARGING_FAULT)) != 0)   bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_DISCHARGING_OVERCURRENT;
            else if ((fault_bitmap & FAULT_BIT(AMPERES_WATCHDOG_FAULT)) != 0)               bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_AMPERES_WATCHDOG;

            else {             
                if (fault_bitmap == 0) {                                      
                    bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_OK;  
                }
                else {
                    bps_status_message.BPS_Fault = BPS_STATUS_BPS_FAULT_INTERNAL_WATCHDOG;
                }
            }

            bps_status_message.BPS_Charge_OK = (bps_status_message.BPS_Fault == BPS_STATUS_BPS_FAULT_OK) ? 1 : 0;
            bps_status_message.BPS_Regen_OK = (bps_status_message.BPS_Fault == BPS_STATUS_BPS_FAULT_OK) ? 1 : 0;

            bps_status_message.HV_Plus_Contactor_State =            (contactor_get(HV_PLUS_CONTACTOR) == CONTACTOR_CLOSED) ? 1 : 0;
            bps_status_message.HV_Minus_Contactor_State =           (contactor_get(HV_MINUS_CONTACTOR) == CONTACTOR_CLOSED) ? 1 : 0;
            bps_status_message.Array_Contactor_State =              (contactor_get(ARRAY_CONTACTOR) == CONTACTOR_CLOSED) ? 1 : 0;
            bps_status_message.Array_Precharge_Contactor_State =    (contactor_get(ARRAY_PRE_CONTACTOR) == CONTACTOR_CLOSED) ? 1 : 0;

            bps_status_message.Main_Battery_Voltage =               (get_pack_voltage());
            bps_status_message.Main_Battery_Avg_Temperature =       (get_avg_temp());

            bps_status_message.BPS_Segment0_Status = get_segment_status(0);
            bps_status_message.BPS_Segment1_Status = get_segment_status(1);
            bps_status_message.BPS_Segment2_Status = get_segment_status(2);
            bps_status_message.BPS_Segment3_Status = get_segment_status(3);
            bps_status_message.BPS_Segment4_Status = get_segment_status(4);
            bps_status_message.BPS_Segment5_Status = get_segment_status(5);
            bps_status_message.BPS_Segment6_Status = get_segment_status(6);
            bps_status_message.BPS_Segment7_Status = get_segment_status(7);

            pack_bps_status_message(&bps_status_message, bps_status_raw_can);

            if (car_can_send(CAN_ID_BPS_STATUS, bps_status_raw_can, CAN_DLC_BPS_STATUS, BPS_STATUS_CAN_DELAY_MS) != CAN_OK) {
                // Don't handle, we don't want to fault when CarCAN is faulted
            };
        }
    }
