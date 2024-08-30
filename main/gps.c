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
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "gps.h"
#include "functions.h"
#include "init.h"

// GLOBAL VARIABLE
// ONLY PERFORM CRITICAL TASKS WHILE PROTECTED!!!
// USE FOLLOWING SPINLOCK TO ENSURE EXCLUSION
portMUX_TYPE gps_data_spinlock = portMUX_INITIALIZER_UNLOCKED;
gps_data_t current_gps_data;



// EXAMPLE FUNCTION
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
                GPS_PRINT_DEBUG
            }
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
    uint8_t gps_str_ptr = 0;
    char gps_strings[NMEA_FIELDS][NMEA_FIELD_MAX_LEN];

    float lattitude = 0.0;
    float longitude = 0.0;
    float altitude;
    float hdop;
    uint8_t num_sats;
    uint8_t utc_min;
    uint8_t utc_hr;
    uint8_t utc_sec;

    // Initalize arrays to hold gps data
    for(i = 0; i < NMEA_FIELDS; i++){
        Clear_Array(gps_strings[i], NMEA_FIELD_MAX_LEN);
    }

    // Seperate fields into sub arrays
    for(i = 0; i < len; i++){
        if(data[i + start_idx] == ','){
            num_commas++;
            gps_str_ptr = 0;
            
            // If have found all fields, break from loop. Done scanning
            if(num_commas >= NMEA_FIELDS){
                break;
            }
            continue;
        }

        if(num_commas < NMEA_FIELDS){
            gps_strings[num_commas][gps_str_ptr] = data[i + start_idx];
            gps_str_ptr++;  
        }
    }

    // Convert from strings to ints or floats
    utc_hr = Str_2_Int(gps_strings[0], 0, 2);
    utc_min = Str_2_Int(gps_strings[0], 2, 2);
    utc_sec = Str_2_Int(gps_strings[0], 4, 2);

    lattitude += Str_2_Int(gps_strings[1], 0, 2);
    lattitude += (atof(&gps_strings[1][2])) / 60;
    if(gps_strings[2][0] == 'S') lattitude *= -1;

    longitude += Str_2_Int(gps_strings[3], 0, 3);
    longitude += (atof(&gps_strings[3][3])) / 60;
    if(gps_strings[4][0] == 'W') longitude *= -1;

    num_sats = Str_2_Int(gps_strings[6], 0, 2);
    hdop = atof(gps_strings[7]);
    altitude = atof(gps_strings[8]);

    // Write data to global variable for use by other tasks
    // Freezes all other tasks. Use spinlock sparingly
    portENTER_CRITICAL(&gps_data_spinlock);
    current_gps_data.lat = lattitude;
    current_gps_data.lon = longitude;
    current_gps_data.altitude = altitude;
    current_gps_data.hdop = hdop;
    current_gps_data.utc_hour = utc_hr;
    current_gps_data.utc_minute = utc_min;
    current_gps_data.utc_second = utc_sec;
    current_gps_data.sats = num_sats;
    portEXIT_CRITICAL(&gps_data_spinlock);

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
    char cmp_str[7] = "$GPGGA,";
    const char *GGA_Start_TAG = "GGA_Start";

    for(i = 0; i < len_to_scan; i++){
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
            *s_idx_ptr = i + 1;         // +1 gives 1st character of data rather than end of "$GPGGA," string

            // DEBUG
            //ESP_LOGI(GGA_Start_TAG, "Found Start, %u", *s_idx_ptr);
        }

        // Once found start, save index of next new line character
        if(found_start && (array[i] == '\n')){
            *len_target_str = i - *s_idx_ptr;

            // DEBUG
            //ESP_LOGI(GGA_Start_TAG, "Found End, %u", *len_target_str);
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

/*
Str_2_Int
This function takes a pointer to a character array, an index to a starting point in that array,
and a length to read. It then reads the specified number of characters and converts to an unsigned
integer. If the specified characters are not numeric, function returns 0. Number must be less than 256
*/
uint8_t Str_2_Int(char* array, uint8_t s_idx, uint8_t len){
    uint8_t i;
    uint8_t output = 0;
    for(i = 0; i < len; i++){
        // Validate data
        if((array[s_idx + i] < '0') || (array[s_idx + i] > '9')){
            output = 0;
            return output;
        }

        // If data good, convert to int
        output *= 10;
        output += array[s_idx + i] - ASCII_OFFSET;
    }
    return output;
}