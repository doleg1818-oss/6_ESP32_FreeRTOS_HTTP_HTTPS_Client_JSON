/*
 * http_get_client.c
 *
 *  Created on: Aug 25, 2026
 *      Author: Olegd
 */


#include "http_client.h"

#include <inttypes.h>
#include <sys/errno.h>

#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include <string.h>
#include <string.h>


static const char *TAG = "HTTP GET";

static esp_err_t http_event_handler(esp_http_client_event_t *event)
{
	if(event == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	http_response_t *response = (http_response_t*)event->user_data;
	
	ESP_LOGI(TAG, "http_event_handler response address =%p <<<", event->user_data); // Print address of structure

	switch(event->event_id)
	{
		case HTTP_EVENT_ERROR:
		{
			ESP_LOGI(TAG, "HTTP_EVENT_ERROR");				// any error
			break;
		}
		case HTTP_EVENT_ON_CONNECTED:   					// Connected to server 
		{
			ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
			break;
		}
		case HTTP_EVENT_HEADERS_SENT:
		{
			ESP_LOGI(TAG, "HTTP_EVENT_HEADERS_SENT");
			break;
		}
		case HTTP_EVENT_ON_HEADER :
		{
			ESP_LOGI(TAG, "Header: %s: %s", event->header_key, event->header_value);
			
			if((event->header_key != NULL) && (event->header_value != NULL))
			{
				if(strcasecmp(event->header_key, "Content-Type") == 0)
				{
					strlcpy(response->content_type, event->header_value, sizeof(response->content_type));
				}
			}
			
			break;
		}
		case HTTP_EVENT_ON_DATA:
		{
			if(response == NULL)
			{
				return ESP_ERR_INVALID_RESPONSE;
			}
			
			size_t free_space = sizeof(response->body) - 1U - response->body_length; // скільки вільного місця в буфері
			size_t copy_length = (size_t)event->data_len;
			if(copy_length > free_space)
			{
				copy_length = free_space;
				response->body_truncated = true;
			}
			if(copy_length > 0)
			{
				// Записати в загальни буфер дані прийняті від сервера
				memcpy(&response->body[response->body_length], event->data, copy_length);
				response->body_length += copy_length;				
				response->body[response->body_length] = '\0';
			}
			
			ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA Received=%d, total=%u", event->data_len, (unsigned)response->body_length);
			break;
		}
		
		default:
		{
			break;
		}
	}
	return ESP_OK;
}

esp_err_t http_client_get(const char *url, http_response_t *response)
{
	if((url == NULL) || (response == NULL))
	{
		return ESP_ERR_INVALID_ARG;
	}
	
	esp_http_client_config_t config = {
		.url = url,
		.method = HTTP_METHOD_GET,
		.event_handler = http_event_handler,
		.user_data = response,
		.timeout_ms = 5000
	};
	
	memset(response, 0, sizeof(*response));
	response->content_length = -1;
		
	ESP_LOGI(TAG, "http_get_client_perform response address =%p <<<", (void *)response); // Print address of structure
	
	esp_http_client_handle_t client = esp_http_client_init(&config);
	if(client == NULL)
	{
		ESP_LOGE(TAG, "Failed to initialized HTTP client");
		return ESP_ERR_NO_MEM;
	}
	
	ESP_LOGI(TAG, "Sending GET request to url: %s", url);
	
	esp_err_t err = esp_http_client_perform(client);
	if(err == ESP_OK)
	{
		response->status_code = esp_http_client_get_status_code(client);
		response->content_length = esp_http_client_get_content_length(client); // Скільки байт сервер відправив до клієнта (Корисне навантаження)
		
		ESP_LOGI(TAG, "HTTP Status: %d, content length = %" PRId64, response->status_code, response->content_length);
	}
	else
	{
		ESP_LOGE(TAG, "HTTP GET failed: %s", esp_err_to_name(err));
	}
	esp_http_client_cleanup(client);
	
	return err;
}


esp_err_t http_client_post_json(const char *url, const char *json, http_response_t *responce)
{
	if((url == NULL) || (json == NULL) || (responce == NULL))
	{
		return ESP_ERR_INVALID_ARG;
	}
	
	memset(responce, 0, sizeof(*responce));
	responce->content_length = -1;
	
	esp_http_client_config_t config = {
		.url = url,
		.method = HTTP_METHOD_POST,
		.event_handler = http_event_handler,
		.user_data = responce,
		.timeout_ms = 5000
	};
	
	esp_http_client_handle_t client = esp_http_client_init(&config);

	if(client == NULL)
	{
		ESP_LOGE(TAG, "Failed ti init HTTP client");
		return ESP_ERR_NO_MEM;
	}	
	
	// Повідомити серверу про формат request body
	esp_err_t err = esp_http_client_set_header(client, "Content-Type", "application/json");
	if(err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to send Content-type");
		esp_http_client_cleanup(client);
		return err;
	}
	
	// Встановити JSON як request body
	err = esp_http_client_set_post_field(client, json, strlen(json));
	if(err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to set POST body");
		return err;
	}
	
	ESP_LOGI(TAG, "POST URL: %s, ",url);
	ESP_LOGI(TAG, "POST JSON: %s, ",json);
	
	err = esp_http_client_perform(client);
	if(err == ESP_OK)
	{
		responce->status_code = esp_http_client_get_status_code(client);
		responce->content_length = esp_http_client_get_content_length(client);
		ESP_LOGI(TAG, "HTTP status: %d, current_length: %" PRId64, responce->status_code, responce->content_length);
	}
	else
	{
		ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(err));
	}
	
	esp_http_client_cleanup(client);
	
	return err;
}






































