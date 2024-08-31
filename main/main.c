/*
Main file for Laelaps-3 firmware
app_main is the entry point for the program
It will initalize the system, spawn threads, and then exit
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "main.h"
#include "functions.h"


//TaskHandle_t xToggle2_Handle = NULL;
TaskHandle_t xRead_GPS_Handle = NULL;
TaskHandle_t xControl_Loop = NULL;


void app_main(void){
    // INIT ALL
    Init_Ports();
    Init_UART2();
    Init_Servos();

    // Start Tasks
    //xTaskCreate(Toggle_2, "Toggle_2", 4096, NULL, 1, &xToggle2_Handle);
    xTaskCreate(Read_GPS, "Read_GPS", 4096, NULL, 2, &xRead_GPS_Handle); // Using about 2k stack space
    xTaskCreate(Control_Loop, "Control Loop", 4096, NULL, 3, &xControl_Loop);

    // Done with app_main. Main task will self delete
    return;
}
