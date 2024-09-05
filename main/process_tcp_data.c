/*
This file holds the code for running the tcp server used to connect to the
Access point esp32 on the drop module

Author:         James Sorber
Contact:        jrsorber@ncsu.edu
Created:        9/01/2024
Modified:       -
Last Built With ESP-IDF v5.2.2
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "errno.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "functions.h"
#include "process_tcp_data.h"

extern SemaphoreHandle_t tcp_rx_array_mutex;

// Static global variables
static char rx_data_storage[RX_DATA_ROWS][RX_DATA_COLUMNS];
static uint8_t rx_data_row_idx = 0;
static uint8_t rx_data_column_idx = 0;


/*
Write_Rx_Storage
This function takes the index of the socket data received from,
a pointer to the start of the rx data, and the length of the data
The function places the data in a row of the rx_data_storage array
It moves to the next row of the array when a newline character is received
Returns void
*/
void Write_Rx_Storage(char* data, uint16_t len){
    xSemaphoreTake(tcp_rx_array_mutex, portMAX_DELAY);
    for(uint16_t i = 0; i < len; i++){
        // Place next character of data into storage array
        rx_data_storage[rx_data_row_idx][rx_data_column_idx] = data[i];

        // If newline received, go to the next line
        if(data[i] == '\n'){
            rx_data_column_idx = 0;
            rx_data_row_idx++;
            if(rx_data_row_idx >= RX_DATA_ROWS) rx_data_row_idx = 0;
            Clear_Array(rx_data_storage[rx_data_row_idx], RX_DATA_COLUMNS);
            continue;
        }

        // If run out of space on current row, reset row index and wrap to next row
        rx_data_column_idx++;
        if(rx_data_column_idx >= RX_DATA_COLUMNS - 1){      // stop 1 character early bc last must be a null
            rx_data_column_idx = 0;
            rx_data_row_idx++;
            if(rx_data_row_idx >= RX_DATA_ROWS) rx_data_row_idx = 0;
            Clear_Array(rx_data_storage[rx_data_row_idx], RX_DATA_COLUMNS);
        }
    }
    xSemaphoreGive(tcp_rx_array_mutex);
    return;
}

/*
process_rx_data_task
This is a rtos thread that looks at the data received from the tcp server and processes it as needed
*/
void Process_TCP_Rx_Data_Task(void *pvParameters){
    const char* PROCESS_DATA_TAG = "Process Data";
    uint8_t rx_row_idx_read = 0;
    char* sub_str_ptr;

    while(1){
        // If there is new data in the rx data storage buffer, print it to the pc
        if(rx_row_idx_read != rx_data_row_idx){
            xSemaphoreTake(tcp_rx_array_mutex, portMAX_DELAY);
            // Add null to end of array for safety
            rx_data_storage[rx_row_idx_read][RX_DATA_COLUMNS - 1] = '\0';

            // Process data
            ESP_LOGI(PROCESS_DATA_TAG, "%s", rx_data_storage[rx_row_idx_read]);

            // If the line is an identification request from the client, send this laelap's HW ID
            sub_str_ptr = strstr(rx_data_storage[rx_row_idx_read], "$id-req");
            if(sub_str_ptr != NULL){
                TCP_Write(PROCESS_DATA_TAG, LAELAPS_HWID, strlen(LAELAPS_HWID));
            }

            // Increment read index
            rx_row_idx_read++;
            if(rx_row_idx_read >= RX_DATA_ROWS) rx_row_idx_read = 0;
            xSemaphoreGive(tcp_rx_array_mutex);
        }

        // Run this loop every 200 ms
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}



