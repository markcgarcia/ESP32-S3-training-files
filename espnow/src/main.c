// This is the firmware for the transmitter board.

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_wifi.h>
#include <esp_mac.h>
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"

// send callback function
static void send_cb(const wifi_tx_info_t *tx_info,          // new: transmission info
                    const esp_now_send_status_t status)     // Status (success/fail)
{
  if (tx_info->des_addr == NULL) {
    printf("Error: null destination MAC address");
    return;
  }
  printf("Sent data to:");
  printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
         tx_info->des_addr[0], tx_info->des_addr[1], tx_info->des_addr[2], 
         tx_info->des_addr[3], tx_info->des_addr[4], tx_info->des_addr[5]);
  if (status == ESP_NOW_SEND_FAIL) {
    printf("Status: FAILED\n");
  }
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

  
  // Register ESPNOW sending or receiving callback function
  /* espnow_send_cb is a function called when a packet has been
     sent. This function also gets a status parameter which can
     tell you whether another device acknowledged the packet. */
  ESP_ERROR_CHECK(esp_now_register_send_cb(send_cb));

  // Add ESPNOW peer information
  esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
  // Clear out memory at this part of the heap
  memset(peer, 0, sizeof(esp_now_peer_info_t));
  // address of peer Rx (B)
  char source[] = {0xa4, 0xcb, 0x8f, 0xd9, 0x2b, 0x0c};
  memcpy((char*)peer->peer_addr, source, sizeof(source));
  peer->channel = 0;
  ESP_ERROR_CHECK(esp_now_add_peer(peer));
  free(peer);

  // Send and receive ESPNOW data
  while (1) {
    uint8_t arr[] = {1, 2, 3, 4, 5};              // dummy data
    ESP_ERROR_CHECK(esp_now_send(NULL, arr, 5));
    vTaskDelay(1000 / portTICK_PERIOD_MS);        // send every 1s
  } 
}
