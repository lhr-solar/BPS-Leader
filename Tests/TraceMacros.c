#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "task.h"

 /* First task sets its tag value to 1. */
void vTask1( void *pvParameters )
{
 /* This task is going to be represented by a voltage scale of 1. */
 vTaskSetApplicationTaskTag( NULL, ( void * ) 1 );
  while(1){
    
 }
}
/*************************************************/

/* Second task sets its tag value to 2. */
void vTask2( void *pvParameters )
{
 /* This task is going to be represented by a voltage scale of 2. */
 vTaskSetApplicationTaskTag( NULL, ( void * ) 2 );
 while(1){

 }
}
/*************************************************/

/* Define the traceTASK_SWITCHED_IN() macro to output the voltage associated
   with the task being selected to run on port 0. */
#define traceTASK_SWITCHED_IN() vSetAnalogueOutput( 0, (int)pxCurrentTCB->pxTaskTag )

int main(){
    HAL_Init();
    
}

