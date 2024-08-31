/*
This file holds the source code for initializing periphials
and other devices, freeRTOS

Author:         James Sorber
Contact:        jrsorber@ncsu.edu
Created:        8/28/2024
Modified:       -
Last Built With ESP-IDF v5.2.2
*/

// Include Header Libraries
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/mcpwm_prelude.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "init.h"
#include "gps.h"
#include "functions.h"


// Init_Ports
void Init_Ports(void){
    gpio_reset_pin(LED1_PIN);
    gpio_set_direction(LED1_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED2_PIN);
    gpio_set_direction(LED2_PIN, GPIO_MODE_OUTPUT);
}

// Init UART2
// Used to read GPS
void Init_UART2(void){
    uart_config_t uart2_config_params = {
        .baud_rate  = 9600,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    // Set parameters
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart2_config_params));
    // Set UART2 Rx to GPIO 16 and TX to 17. These are the default pins for UART2
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // Install resources and drivers
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, UART2_RX_BUF_LEN, UART2_TX_BUF_LEN, 0, NULL, 0));
}