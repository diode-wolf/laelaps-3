/*
This file holds the macro definitions for process_tcp_data.c

Author:         James Sorber
Contact:        jrsorber@ncsu.edu
Created:        9/01/2024
Modified:       -
Last Built With ESP-IDF v5.2.2
*/

#ifndef PROCESS_DATA_H
#define PROCESS_DATA_H

// This is the identity of a particular laelaps capsule
// Should be unique to each capsule. Valid range is 1-5 inclusive
// If support for more payloads is needed, will need to modify server code
#define LAELAPS_HWID        "$id-ack-2\r\n"

#define RX_DATA_ROWS        5
#define RX_DATA_COLUMNS     64
#define GPS_STR_MAX_LEN     12
#define ASCII_OFFSET        0x30


#endif