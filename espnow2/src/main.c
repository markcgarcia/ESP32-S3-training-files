// This is the firmware for the receiver board.

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_wifi.h>
#include <esp_mac.h>
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"

// Receive callback function
static void recv_cb(const esp_now_recv_info_t *recv_info,   // Receive struct (has send/dest MAC addr.)
                    const uint8_t *data,                    // Pointer to sent data
                    const int len)                          // Length of sent data
{
  if (recv_info == NULL || data == NULL || len <= 0) {
    printf("Error: invalid parameters");
    return;
  }
  // Print confirmation
  printf("Received data from:\n");
  printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
         recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2], 
         recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5]);
  printf("%d, %d, %d, %d, %d\n", data[0], data[1], data[2], data[3], data[4]);
}

void app_main() {

  // Initialize WiFi
  ESP_ERROR_CHECK(nvs_flash_init());
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));

  // Initialize ESPNOW
  ESP_ERROR_CHECK(esp_now_init());

  // Read MAC address and echo it to the serial monitor
  printf("[DEFAULT] ESP32 Board MAC Address: ");
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
         baseMac[0], baseMac[1], baseMac[2], baseMac[3], 
         baseMac[4], baseMac[5]);

  // Read WiFi channel
  uint8_t primary = 0;
  wifi_second_chan_t secondary = 0;
  esp_wifi_get_channel(&primary, &secondary);
  printf("WIFI: Connected AP Channel:\n Primary: %d\n Secondary: %d\n", primary, secondary);

  /* espnow_recv_cb is a function called when this device receives a
     ESP_NOW packet */ 
  ESP_ERROR_CHECK(esp_now_register_recv_cb(recv_cb));

  // Receive ESPNOW data
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
