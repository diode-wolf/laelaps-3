#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "main.h"
#include "functions.h"

// Global Variables - BAD
static uint8_t led1_state = 0;
static uint8_t led2_state = 0;

TaskHandle_t xToggle1_Handle = NULL;
TaskHandle_t xToggle2_Handle = NULL;

void Init_Ports(void){
    gpio_reset_pin(LED1_PIN);
    gpio_set_direction(LED1_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED2_PIN);
    gpio_set_direction(LED2_PIN, GPIO_MODE_OUTPUT);
}

void Toggle_1(void *args){
    while(1){
        gpio_set_level(LED1_PIN, led1_state);
        led1_state = !led1_state;
        vTaskDelay(pdMS_TO_TICKS(LED1_RATE));
    }
}

void Toggle_2(void *args){
    while(1){
        gpio_set_level(LED2_PIN, led2_state);
        led2_state = !led2_state;
        vTaskDelay(pdMS_TO_TICKS(LED2_RATE));
    }
}


void app_main(void){
    // INIT ALL
    Init_Ports();

    // Start Tasks
    xTaskCreate(Toggle_1, "Toggle_1", 4096, NULL, 2, &xToggle1_Handle);
    xTaskCreate(Toggle_2, "Toggle_2", 4096, NULL, 1, &xToggle2_Handle);

    // Done with app_main. Main task will self delete
    return;
}
