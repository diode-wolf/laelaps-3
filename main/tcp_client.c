/* BSD non-blocking socket example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sys/socket.h"
#include "netdb.h"
#include "errno.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "functions.h"

#define TCP_CLIENT_CONNECT_ADDRESS  "192.168.4.1"
#define TCP_CLIENT_CONNECT_PORT     "7983"
#define INVALID_SOCK (-1)
#define YIELD_TO_ALL_MS 50
#define TRUE    1
#define FALSE   0

static int sock = INVALID_SOCK;
static uint8_t socket_error_flag = FALSE;


/**
 * @brief Utility to log socket errors
 *
 * @param[in] tag Logging tag
 * @param[in] sock Socket number
 * @param[in] err Socket errno
 * @param[in] message Message to print
 */
static void log_socket_error(const char *tag, const int sock, const int err, const char *message)
{
    ESP_LOGE(tag, "[sock=%d]: %s\n"
                  "error=%d: %s", sock, message, err, strerror(err));
}

/**
 * @brief Tries to receive data from specified sockets in a non-blocking way,
 *        i.e. returns immediately if no data.
 *
 * @param[in] tag Logging tag
 * @param[in] sock Socket for reception
 * @param[out] data Data pointer to write the received data
 * @param[in] max_len Maximum size of the allocated space for receiving data
 * @return
 *          >0 : Size of received data
 *          =0 : No data available
 *          -1 : Error occurred during socket read operation
 *          -2 : Socket is not connected, to distinguish between an actual socket error and active disconnection
 */
static int try_receive(const char *tag, char * data, size_t max_len)
{
    int len = recv(sock, data, max_len, 0);
    if (len < 0) {
        if (errno == EINPROGRESS || errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;   // Not an error
        }
        if (errno == ENOTCONN) {
            ESP_LOGW(tag, "[sock=%d]: Connection closed", sock);
            return -2;  // Socket has been disconnected
        }
        log_socket_error(tag, sock, errno, "Error occurred during receiving");
        return -1;
    }

    return len;
}

/**
 * @brief Sends the specified data to the socket. This function blocks until all bytes got sent.
 *
 * @param[in] tag Logging tag
 * @param[in] sock Socket to write data
 * @param[in] data Data to be written
 * @param[in] len Length of the data
 * @return
 *          >0 : Size the written data
 *          -1 : Error occurred during socket write operation
 */
int TCP_Write(const char *tag, const char * data, const size_t len){
    int to_write = len;
    while (to_write > 0) {
        int written = send(sock, data + (len - to_write), to_write, 0);
        if (written < 0 && errno != EINPROGRESS && errno != EAGAIN && errno != EWOULDBLOCK) {
            log_socket_error(tag, sock, errno, "Error occurred during sending");
            socket_error_flag = TRUE;
        }
        to_write -= written;
    }
    return len;
}


// This is the tcp client thread
// Reads and sends data from laelaps server
void TCP_Client_Task(void *pvParameters){
    static const char *TAG = "TCP-client";
    static char rx_buffer[128];

    struct addrinfo hints = { .ai_socktype = SOCK_STREAM };
    struct addrinfo *address_info;

    fd_set fdset;
    struct timeval connect_timeout;
    connect_timeout.tv_sec = 1;         // Set to 1s timeout
    


    // Convert address from string, create socket, and set as not blocking
    int res = getaddrinfo(TCP_CLIENT_CONNECT_ADDRESS, TCP_CLIENT_CONNECT_PORT, &hints, &address_info);
    if (res != 0 || address_info == NULL) {
        ESP_LOGE(TAG, "couldn't get hostname for `%s` "
                      "getaddrinfo() returns %d, addrinfo=%p", TCP_CLIENT_CONNECT_ADDRESS, res, address_info);
        vTaskDelete(NULL);
    }

    //------------------------------------------------------------------------------------------
    // Try to create and connect socket. On success, enters another infinite while loop checking for data
    // On fail, wait one second and try to connect again
    while(1){
        vTaskDelay(pdMS_TO_TICKS(1000));         // Wait 1s before attempting to reconnect again
        // Set socket error to false, make sure socket is closed befor attempting to connect
        socket_error_flag = FALSE;
        if (sock != INVALID_SOCK) {
            close(sock);
        }

        // Creating client's socket
        sock = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
        if (sock < 0) {
            log_socket_error(TAG, sock, errno, "Unable to create socket");
            vTaskDelete(NULL);
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%s", TCP_CLIENT_CONNECT_ADDRESS, TCP_CLIENT_CONNECT_PORT);

        // Marking the socket as non-blocking
        int flags = fcntl(sock, F_GETFL);
        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
            log_socket_error(TAG, sock, errno, "Unable to set socket non blocking");
        }

        // Attempt to connect while checking for errors
        if (connect(sock, address_info->ai_addr, address_info->ai_addrlen) != 0) {
            // If connection is in progress
            if (errno == EINPROGRESS) {
                ESP_LOGD(TAG, "connection in progress");
                FD_ZERO(&fdset);
                FD_SET(sock, &fdset);

                // Wait 1s for socket to be ready indicated by file descriptor becoming writeable
                res = select(sock+1, NULL, &fdset, NULL, &connect_timeout);
                if (res < 0) {
                    log_socket_error(TAG, sock, errno, "Error during connection: Waiting for socket to be writable");
                    socket_error_flag = TRUE;
                } 
                else if (res == 0) {
                    log_socket_error(TAG, sock, errno, "Connection timeout: Waiting for socket to be writable");
                    socket_error_flag = TRUE;
                } 
                else {
                    int sockerr;
                    socklen_t len = (socklen_t)sizeof(int);

                    // If socket indicated ready for write, check for socket errors
                    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void*)(&sockerr), &len) < 0) {
                        log_socket_error(TAG, sock, errno, "Error when getting socket error using getsockopt()");
                        socket_error_flag = TRUE;
                    }
                    // If error indicted, print here
                    if (sockerr) {
                        log_socket_error(TAG, sock, sockerr, "Connection error");
                        socket_error_flag = TRUE;
                    }
                }
            }
            // If connect failed and not in progress
            else {
                log_socket_error(TAG, sock, errno, "Socket is unable to connect");
                socket_error_flag = TRUE;
            }
        }

        // if socket_error_flag is still FALSE, we are connected to the server
        if(!socket_error_flag){
            // Continually check for incoming data
            int len;
            while(1){
                len = try_receive(TAG, rx_buffer, sizeof(rx_buffer));
                if (len < 0) {
                    // If error occured, break from inner loop so close and reconnect happens again
                    ESP_LOGE(TAG, "Error occurred during try_receive");
                    socket_error_flag = TRUE;
                    break;
                }
                else if(len > 0){
                    ESP_LOGI(TAG, "Received: %.*s", len, rx_buffer);
                    // Do stuff with RX data here
                    Write_Rx_Storage(rx_buffer, len);
                }
                vTaskDelay(pdMS_TO_TICKS(YIELD_TO_ALL_MS));
            } // End rx while
        }  // End if no error flag
    } // End outer while
} // End task


// End of File
