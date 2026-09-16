/*
 * http_result_task.c
 *
 *  Created on: Sep 15, 2026
 *      Author: Olegd
 */

#include "inttypes.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../http/http_client_task.h"

#define HTTP_RESULT_TASK_STACK_SIZE			4098U
#define HTTP_RESULT_TASK_PRIORITY			4U

static TaskHandle_t http_result_worker_task_handle = NULL;

static const char *TAG = "HTTP RESULT TASK";

static void http_result_worker_task(void *arg)
{
	static http_client_result_t result = {0};
	  
	while(1)
	{
		esp_err_t err = http_client_task_receive_result(&result, portMAX_DELAY);
		if(err != ESP_OK)
		{
			ESP_LOGE(TAG, "Failed to receive resalt");
			continue;
		}
		ESP_LOGI(TAG, "Request id = %" PRIu32, result.request_id);
		
		// result.err це результат transport/http operation 	
		if(result.err != ESP_OK)
		{
			ESP_LOGE(TAG, "HTTP transport failed: %s", esp_err_to_name(result.err));
			continue;
		}	  
		 
		ESP_LOGI(TAG, "HTTP status =%d", result.response.status_code);
		ESP_LOGI(TAG, "Content-type =%s", result.response.content_type);
		  
		// Транспорт може бути ESP_OK а HTTP status млже бути 200, 404, 500 
		if(result.response.status_code >= 200 && result.response.status_code <= 300)
		{
			ESP_LOGI(TAG, "Request sucsesfuly");
			ESP_LOGI(TAG, "Response body =%s", result.response.body);
		}
		else
		{
			ESP_LOGE(TAG, "Server returned HTTP error status = %d", result.response.status_code);
		}
	}
}

esp_err_t http_result_task_start(void)
{
	if(http_result_worker_task_handle != NULL)
	{
		return ESP_ERR_INVALID_STATE;
	}
	
	BaseType_t status = xTaskCreate(http_result_worker_task, "http_result_worker_task", HTTP_RESULT_TASK_STACK_SIZE, NULL, HTTP_RESULT_TASK_PRIORITY, &http_result_worker_task_handle);
	if(status != pdPASS)
	{
		http_result_worker_task_handle = NULL;
		ESP_LOGE(TAG, "Failed to create http_result_worker_task");
		return ESP_ERR_NO_MEM;
	}
	ESP_LOGI(TAG, "HTTP result task started");
	
	return ESP_OK;
}
