/*
 * http_get_client.h
 *
 *  Created on: Aug 25, 2026
 *      Author: Olegd
 */

#ifndef MAIN_HTTP_GET_CLIENT_H_
#define MAIN_HTTP_GET_CLIENT_H_

#include "esp_err.h"

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

#define HTTP_GET_RESPONSE_BODY_SIZE 1024U
#define HTTP_GET_CONTENT_TYPE_SIZE  64U

typedef struct{
	int status_code;
	int64_t content_length;
	size_t body_length;
	bool body_truncated;
	
	char content_type[HTTP_GET_CONTENT_TYPE_SIZE];
	char body[HTTP_GET_RESPONSE_BODY_SIZE];
}http_get_response_t;

esp_err_t http_get_client_perform(const char *urr, http_get_response_t *response);

#endif /* MAIN_HTTP_GET_CLIENT_H_ */
