/*
This file holds the source code for working with the GPS
module via UART interface

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
#include "esp_log.h"
#include "sdkconfig.h"
#include "gps.h"
#include "functions.h"
#include "init.h"

// EXAMPLE FUNCTIONS
void Toggle_2(void *args){
    // Variables local to this task
    uint8_t led2_state = 0;
    const char *TOG2_TAG = "Toggle_2";

    while(1){
        gpio_set_level(LED2_PIN, led2_state);
        led2_state = !led2_state;
        vTaskDelay(pdMS_TO_TICKS(LED2_RATE));
    }
}

void Read_GPS(void *args){
    // Variables local to this task
    const char *GPS_TAG = "Read_GPS";
    uint8_t *gps_rx_data = (uint8_t *) malloc(UART2_RX_BUF_LEN);
    gps_data_t current_gps_data;
    uint8_t len_data_read = 0;
    uint16_t start_data_idx;
    uint16_t data_length;
    int8_t found_data;

    while(1){
        // Read from UART2
        len_data_read = uart_read_bytes(UART_NUM_2, gps_rx_data, UART2_RX_BUF_LEN, pdMS_TO_TICKS(100));
        if(len_data_read){
            found_data = Get_GGA_Start((char*)gps_rx_data, len_data_read, &start_data_idx, &data_length);
            if(found_data){
                Extract_GPS_Data((char*) gps_rx_data, start_data_idx, data_length, &current_gps_data);
            }
            // gps_rx_ptr[len_data_read] = '\0';     // Add null to terminate string
            // ESP_LOGI(GPS_TAG, "%s", (char *) gps_rx_ptr);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* 
Extract GPS Data
This function takes a character array containing NMEA GPS strings, the index to start scanning at,
the length to scan for, and a pointer to a struct to recieve the output. The function extracts the
gps data from the string, converts it to floats/ints, and stores in the output struct,
The function assumes that the gps data is stored in csv strings as per  NMEA standards
*/
int8_t Extract_GPS_Data(char *data, uint16_t start_idx, uint16_t len, gps_data_t *output){
    const char *Ext_GPS_TAG = "Extract_GPS";
    uint16_t i;
    uint8_t num_commas = 0;
    char gps_strings[NMEA_FIELDS][NMEA_FIELD_MAX_LEN];
    Clear_Array(gps_strings, NMEA_FIELD_MAX_LEN * NMEA_FIELDS);

    // Seperate fields into sub arrays
    for(i = 0; i < len; i++){
        if(data[i + start_idx] == ','){
            num_commas++;
            continue;
        }

        gps_strings[num_commas][i] = data[i + start_idx];
    }

    // Temp debug
    ESP_LOGI(Ext_GPS_TAG, "%s\n", gps_strings[0]);
    ESP_LOGI(Ext_GPS_TAG, "%s\n", gps_strings[1]);
    ESP_LOGI(Ext_GPS_TAG, "%s\n", gps_strings[2]);
    ESP_LOGI(Ext_GPS_TAG, "%s\n", gps_strings[3]);
    ESP_LOGI(Ext_GPS_TAG, "%s\n", gps_strings[4]);
    ESP_LOGI(Ext_GPS_TAG, "%s\n", gps_strings[5]);

    return 1;
}

/*
Get_GGA_Start
This function scans an array for the presence of the string "$GPGGA ... \n"
Upon finding the string, the function outputs the index of the first comma, and the
length until a new line character. The function takes a pointer to an array, the 
length of the array and pointers to the two values to hold the output indexes
Returns 1 on success, 0 on fail to find string
*/
int8_t Get_GGA_Start(char* array, uint16_t len_to_scan, uint16_t* s_idx_ptr, uint16_t* len_target_str){
    uint16_t i;
    uint8_t found_start = FALSE;
    uint8_t cmp_str_idx = 0;
    char cmp_str[7] = "$GPGGA,"
    const char *GGA_Start_TAG = "GGA_Start";

    for(i = 0; i < len_to_scan, i++){
        // If match, move to checking for the next character
        if(array[i] == cmp_str[cmp_str_idx]){
            cmp_str_idx++;
        }
        else{
            cmp_str_idx = 0;
        }

        // If whole string matched, set flag, save current index
        if(cmp_str_idx >= 7){
            cmp_str_idx = 0;
            found_start = TRUE;
            *s_idx_ptr = i;

            // DEBUG
            ESP_LOGI(GGA_Start_TAG, "Found Start, %u", *s_idx_ptr);
        }

        // Once found start, save index of next new line character
        if(found_start && (array[i] == '\n')){
            *len_target_str = i - *s_idx_ptr;

            // DEBUG
            ESP_LOGI(GGA_Start_TAG, "Found End, %u", *len_target_str);
            return TRUE;
        }
    }

    // If exited for loop without finding string, return false
    return FALSE;
}


/*
Clear_Array
This function sets the contents of a character array to all zeros
Takes pointer to array, and array length. Does not return anything
*/
void Clear_Array(char* array, uint16_t len){
    uint16_t i;
    for(i = 0; i < len; i++){
        array[i] = '\0';
    }
    return;
}