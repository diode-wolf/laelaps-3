/*
This file holds the macro definitions for gps.h
Macros used for debugging, readability, etc

Author:         James Sorber
Contact:        jrsorber@ncsu.edu
Created:        8/28/2024
Modified:       -
Last Built With ESP-IDF v5.2.2
*/

#ifndef GPS_H
#define GPS_H

// Uncomment to print debug information
#define GPS_DEBUG

// Print debut info
#ifdef GPS_DEBUG
#define GPS_PRINT_DEBUG ESP_LOGI(GPS_TAG, "UTC Time: %d:%d:%d", current_gps_data.utc_hour, current_gps_data.utc_minute, current_gps_data.utc_second); ESP_LOGI(GPS_TAG, "Lattitude: %.5f", current_gps_data.lat); ESP_LOGI(GPS_TAG, "Longitude: %.5f", current_gps_data.lon); ESP_LOGI(GPS_TAG, "Altitude: %.1f", current_gps_data.altitude); ESP_LOGI(GPS_TAG, "# Sats: %d", current_gps_data.sats); ESP_LOGI(GPS_TAG, "HDOP: %.2f", current_gps_data.hdop); ESP_LOGI(GPS_TAG, "Stack High Water: %d", uxTaskGetStackHighWaterMark(NULL));
#elif
#define GPS_PRINT_DEBUG
#endif

// Macros
#define NMEA_FIELD_MAX_LEN  12
#define NMEA_FIELDS         9
#define UART2_RX_BUF_LEN    1024
#define UART2_TX_BUF_LEN    0
#define ASCII_OFFSET        0x30

#define TRUE                1
#define FALSE               0


// Custom data types
// Struct to hold gps data
typedef struct GPS_Data{
    float lat;
    float lon;
    float altitude;
    float hdop;
    uint8_t utc_hour;
    uint8_t utc_minute;
    uint8_t utc_second;
    uint8_t sats;
} gps_data_t;

// Struct to hold char strings from nmea messages
typedef struct NMEA_Fields{
    char lat_str[NMEA_FIELD_MAX_LEN];
    char lon_str[NMEA_FIELD_MAX_LEN];
    char acc_str[NMEA_FIELD_MAX_LEN];
    char alt_str[NMEA_FIELD_MAX_LEN];
    char sat_str[NMEA_FIELD_MAX_LEN];
    char utc_str[NMEA_FIELD_MAX_LEN];
} nmea_fields_t;
#endif