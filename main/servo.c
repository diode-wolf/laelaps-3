/*
This file holds the source code for working with the servo motors

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
#include "driver/mcpwm_prelude.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "servo.h"
#include "functions.h"
#include "init.h"


// Global to this file
// Handles for timer comparators to generate PWM
static mcpwm_cmpr_handle_t servo1_cmp = NULL;
static mcpwm_cmpr_handle_t servo2_cmp = NULL;
static const char* SERVO_TAG = "Servo";


// Init Servos
void Init_Servos(void){
    // Create timer
    mcpwm_timer_handle_t servo_tmr = NULL;
    mcpwm_timer_config_t servo_tmr_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = SERVO_RES_HZ,
        .period_ticks = SERVO_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&servo_tmr_config, &servo_tmr));

    // Create operator
    mcpwm_oper_handle_t servo_oper = NULL;
    mcpwm_operator_config_t servo_oper_config = {
        .group_id = 0,
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&servo_oper_config, &servo_oper));

    // Connect timer to operator
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(servo_oper, servo_tmr));

    // Create two comparators, one for servo 1 value, one for servo 2 value
    mcpwm_comparator_config_t servo1_cmp_config = {
        .flags.update_cmp_on_tez = true,
    };
    mcpwm_comparator_config_t servo2_cmp_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(servo_oper, &servo1_cmp_config, &servo1_cmp));
    ESP_ERROR_CHECK(mcpwm_new_comparator(servo_oper, &servo2_cmp_config, &servo2_cmp));

    // Create PWM generator for each servo
    mcpwm_gen_handle_t servo1_gen = NULL;
    mcpwm_gen_handle_t servo2_gen = NULL;
    mcpwm_generator_config_t servo1_gen_config = {
        .gen_gpio_num = SERVO_1_PIN,
    };
    mcpwm_generator_config_t servo2_gen_config = {
        .gen_gpio_num = SERVO_2_PIN,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(servo_oper, &servo1_gen_config, &servo1_gen));
    ESP_ERROR_CHECK(mcpwm_new_generator(servo_oper, &servo2_gen_config, &servo2_gen));

    // Set the initial servo positions to centered
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(servo1_cmp, Map_Servo_Deg_PWM(0)));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(servo2_cmp, Map_Servo_Deg_PWM(0)));

    // Set PWM generator actions on comparator events
    // Go high on timer empty/top (same event really)
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(servo1_gen, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(servo2_gen, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // Set low when timer reaches compare value
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(servo1_gen, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, servo1_cmp, MCPWM_GEN_ACTION_LOW)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(servo2_gen, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, servo2_cmp, MCPWM_GEN_ACTION_LOW)));

    // Enable and start timers
    ESP_ERROR_CHECK(mcpwm_timer_enable(servo_tmr));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(servo_tmr, MCPWM_TIMER_START_NO_STOP));
}


// Set Servo
// This function sets a servo to a position
// Takes uint8_t servo number, and int16_t servo position
// Does not return anything
void Set_Servo(uint8_t servo, int16_t position){
    uint16_t compare_value;
    // Input validation
    if ((position < SERVO_MIN_DEG) || (position > SERVO_MAX_DEG)){
        ESP_LOGE(SERVO_TAG, "Servo %d invalid angle %d degrees", servo, position);
        return;
    }

    // If data good, set servos
    compare_value = Map_Servo_Deg_PWM(position);
    switch(servo){
    case 0:
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(servo1_cmp, compare_value));
        //ESP_LOGI(SERVO_TAG, "Set Servo 1 %d ms", compare_value);
    break;

    case 1:
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(servo2_cmp, compare_value));
        //ESP_LOGI(SERVO_TAG, "Set Servo 2 %d ms", compare_value);
    break;

    default:
    break;
    }
}


// Map_Servo_Deg_PWM
// This function maps a servo position to a PWM pulsewidth
// Takes angle in degrees, returns pulse high time in us
uint16_t Map_Servo_Deg_PWM(int16_t degrees){
    int32_t micro_seconds;
    // Input validation
    if((degrees < SERVO_MIN_DEG) || (degrees > SERVO_MAX_DEG)){
        return (SERVO_MIN_US + SERVO_MAX_US) / 2;
    }

    // If angle good, map to number of microseconds
    micro_seconds = (degrees - SERVO_MIN_DEG) * (SERVO_MAX_US - SERVO_MIN_US) / (SERVO_MAX_DEG - SERVO_MIN_DEG)  + SERVO_MIN_US;
    return (uint16_t) micro_seconds;
}