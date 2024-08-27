#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "main.h"

// Global Variables - BAD
static uint8_t led1_state = 0;
static uint8_t led2_state = 0;

TaskHandle_t LED1 = NULL;
TaskHandle_t LED2 = NULL;

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
        vTaskDelay(LED1_RATE / portTICK_PERIOD_MS);
    }
}

void Toggle_2(void *args){
    while(1){
        // gpio_set_level(LED2_PIN, led2_state);
        // led2_state = !led2_state;
        printf("T2 exe\n");
        vTaskDelay(LED2_RATE / portTICK_PERIOD_MS);
    }
}


void app_main(void){
    // INIT ALL
    Init_Ports();

    // Start Tasks
    xTaskCreate(Toggle_1, "Toggle_1", 512, NULL, 2, &LED1);
    //xTaskCreate(Toggle_2, "Toggle_2", 512, NULL, 1, &LED2);

    // Done with app_main. Main task will self delete after return from app_main
    return;
}
