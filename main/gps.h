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