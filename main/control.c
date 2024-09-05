/*
This file holds the source code controling laelaps
These functions / tasks look at the position, etc and
set servos to try to reach the target

Author:         James Sorber
Contact:        jrsorber@ncsu.edu
Created:        8/30/2024
Modified:       -
Last Built With ESP-IDF v5.2.2
*/

// Include Header Libraries
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "control.h"
#include "functions.h"
#include "init.h"

// Temp control function
// Will add functionality later
void Control_Loop(void *args){
    //const char* CTRL_TAG = "Control_Loop";
    int16_t servo1_pos = 0;
    int16_t servo2_pos = 0;
    int16_t servo_min_pos = -90;
    int16_t servo_max_pos = 90;
    int16_t servo1_step = -10;
    int16_t servo2_step = 10;

    while(1){
        //ESP_LOGI(CTRL_TAG, "%d deg", servo_pos);
        Set_Servo(0, servo1_pos);
        Set_Servo(1, servo2_pos);

        servo1_pos += servo1_step;
        servo2_pos += servo2_step;
        if((servo1_pos <= servo_min_pos) || (servo1_pos >= servo_max_pos)) servo1_step *= -1;
        if((servo2_pos <= servo_min_pos) || (servo2_pos >= servo_max_pos)) servo2_step *= -1;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}