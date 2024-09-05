/*
Main file for Laelaps-3 firmware
app_main is the entry point for the program
It will initalize the system, spawn threads, and then exit
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "netdb.h"
#include "esp_netif.h"
#include "errno.h"
#include "sys/socket.h"
#include "main.h"
#include "functions.h"

// Thread protection
SemaphoreHandle_t tcp_rx_array_mutex;


//TaskHandle_t xToggle2_Handle = NULL;
TaskHandle_t xRead_GPS_Handle = NULL;
TaskHandle_t xControl_Loop = NULL;
TaskHandle_t xWifi_Connect = NULL;
TaskHandle_t xTCP_Client = NULL;
TaskHandle_t xProcess_USB = NULL;
TaskHandle_t xProcess_TCP = NULL;


void app_main(void){
  const char *MAIN_TAG = "App_Main";
  // Run NVS setup
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Run other setup
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // Init Mutexes
  tcp_rx_array_mutex = xSemaphoreCreateMutex();

  // INIT ALL
  Init_Ports();
  Init_UART2();
  Init_UART0();
  Init_Servos();

  // Start Tasks
  xTaskCreate(Wifi_Connect_Task, "Wifi_Connect", 4096, NULL, 1, &xWifi_Connect);  // Using about 2.5kB stack
  xTaskCreate(Read_GPS, "Read_GPS", 2048, NULL, 2, &xRead_GPS_Handle);            // Using about 0.8kB stack
  xTaskCreate(Process_USB_Rx_Data_Task, "Rx_USB", 3072, NULL, 3, &xProcess_USB);  // ??
  xTaskCreate(Process_TCP_Rx_Data_Task, "Rx_TCP", 3072, NULL, 4, &xProcess_TCP);  // ??
  xTaskCreate(Control_Loop, "Control Loop", 4096, NULL, 5, &xControl_Loop);       // Using about 1.8kB stack
  xTaskCreate(TCP_Client_Task, "tcp_client", 4096, NULL, 6, &xTCP_Client);        // Using about 1.9kB stack



  // Uncomment to print log of stack high water marks
  // while(1){
  //     vTaskDelay(pdMS_TO_TICKS(5000));
  //     ESP_LOGI(MAIN_TAG, "Wifi Connect HWM: %d", uxTaskGetStackHighWaterMark(xWifi_Connect));
  //     ESP_LOGI(MAIN_TAG, "Read GPS HWM: %d", uxTaskGetStackHighWaterMark(xRead_GPS_Handle));
  //     ESP_LOGI(MAIN_TAG, "Read USB HWM: %d", uxTaskGetStackHighWaterMark(xProcess_USB));
  //     ESP_LOGI(MAIN_TAG, "Read TCP HWM: %d", uxTaskGetStackHighWaterMark(xProcess_TCP));
  //     ESP_LOGI(MAIN_TAG, "Control Loop HWM: %d", uxTaskGetStackHighWaterMark(xControl_Loop));
  //     ESP_LOGI(MAIN_TAG, "TCP Client HWM: %d", uxTaskGetStackHighWaterMark(xTCP_Client));
  // } 
  // Done with app_main. Main task will self delete
  return;
}