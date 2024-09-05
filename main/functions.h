/*
This file holds the function prototypes for the entire project
functions.h should be included in all .c source files

Author:         James Sorber
Contact:        jrsorber@ncsu.edu
Created:        8/28/2024
Modified:       -
Last Built With ESP-IDF v5.2.2
*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// Forward declaration of custom types
typedef struct GPS_Data gps_data_t;

// INIT.C
void Init_Ports(void);
void Init_UART2(void);

// CONTROL.c
void Control_Loop(void *args);


// GPS.C
//void Toggle_2(void *args);
void Read_GPS(void *args);
int8_t Extract_GPS_Data(char *data, uint16_t start_idx, uint16_t len, gps_data_t *output);
int8_t Get_GGA_Start(char* array, uint16_t len_to_scan, uint16_t* s_idx_ptr, uint16_t* len_target_str);
void Clear_Array(char* array, uint16_t len);
uint8_t Str_2_Int(char* array, uint8_t s_idx, uint8_t len);

// SERVO.C
void Init_Servos(void);
void Set_Servo(uint8_t servo, int16_t position);
uint16_t Map_Servo_Deg_PWM(int16_t degrees);

// WIFI_STA.C
void Init_Wifi_Sta(void);
void Wifi_Connect_Task(void *pvParameters);

// TCP_CLIENT.C
void TCP_Client_Task(void *pvParameters);

#endif
