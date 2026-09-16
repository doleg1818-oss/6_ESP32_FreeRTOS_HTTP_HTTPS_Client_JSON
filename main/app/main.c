

#include "inttypes.h"
#include "stdlib.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "string.h"

#include "../secrets.h"
#include "../wifi/wifi_manager.h"
#include "../wifi/wifi_manager_test.h"
#include "../http/http_client.h"
#include "../http/http_client_task.h"
#include "../data/server_data_parser.h"
#include "../data/device_data_serializer.h"

#include "telemetry_task.h"
#include "http_result_task.h"



static const char *TAG = "WIFI STA";

static bool wait_for_online(uint32_t timeout_ms) {
  TickType_t start_tick = xTaskGetTickCount();
  TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

  while ((xTaskGetTickCount() - start_tick) < timeout_ticks) {
    if (wifi_manager_is_online()) {
      return true;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  return false;
}

void app_main(void) {
  ESP_LOGI(TAG, "Aplication started");

  // Init NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
  }

  // Initialize WiFi
  wifi_manager_config_t config = {.ssid = WIFI_SSID,
                                  .password = WIFI_PASSWORD,

                                  .reconnect_base_delay_ms = 2000,
                                  .reconnect_max_delay_ms = 30000,
                                  .reconnect_jitter_ms = 500,

                                  .max_reconnect_attempts = 15,
                                  .same_ap_reconnect_limit = 2,

                                  .auto_connect = true};

  err = wifi_manager_init(&config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "FAILED INIT WiFi !!! ");
    return;
  }
  ESP_LOGI(TAG, "WiFi STA initialized complited");

  //////////////////////////////////////////////////////
  	if (wait_for_online(30000) == false)
  	{
    	ESP_LOGE(TAG, "WiFi did not become ONLINE !");
    	return;
  	}
  	ESP_LOGI(TAG, "WiFi Online. Starting HTTP POST");

  	wifi_manager_test_print_status();
  
  
  	// Ініувалізує чергу request і result і таску відправки реквесту
  	err = http_client_task_init();
  	if(err != ESP_OK)
  	{
	  	ESP_LOGE(TAG, "Failed to initialize HTTP task");
	  	return;
  	}
  
  	// Ініціалізує і запускає таску яка чекає/приймає результат реквесту від сервера
	err = http_result_task_start();
	if(err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to initialize http_result_task_start task");
	  	return;
	}
  
  	// Запускає переодичне відсилання реквесту від клієнта до сервера
	err = telemetry_task_start();
	if(err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to initialize telemetry_task_start task");
	  	return;
	}
  
	ESP_LOGI(TAG, "Aplication initializet complate"); 

  
	 
  
 


  // Wifi tests
  // wifi_manager_start_test_matrix();
  // wifi_manager_start_status_reader();
}
