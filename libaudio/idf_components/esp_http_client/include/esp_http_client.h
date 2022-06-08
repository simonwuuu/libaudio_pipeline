/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ESP_HTTP_CLIENT_H
#define _ESP_HTTP_CLIENT_H

#include "sdkconfig.h"
#include "esp_err.h"
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_HTTP_BUF_SIZE (512)

/**
 * private HTTP Data structure
 */
typedef struct//部分头信息
{
    int status_code;//HTTP/1.1 '200' OK
    char content_type[128];//Content-Type: application/gzip
    long content_length;//Content-Length: 11683079
    char location[1024];
} HTTP_RES_HEADER;

typedef struct {
    HTTP_RES_HEADER headers;       /*!< http header */
    char   *head_buffer;        /*!< data buffer*/
} esp_http_data_t;

typedef struct {
    const char                         *url;
    char                         host[100];
    char                         ip_addr[20];
    int                          port;
} connection_info_t;

struct esp_http_client {
    esp_http_data_t                 *request;
    esp_http_data_t                 *response;
    connection_info_t           connection_info;
    char file_name[256];
    int client_socket;
};

/**
 * @brief HTTP configuration
 */
typedef struct {
    const char                  *url;                /*!< HTTP URL, the information on the URL is most important, it overrides the other fields below, if any */
    const char                  *host;               /*!< Domain or IP as string */
    int                         port;                /*!< Port to connect, default depend on esp_http_client_transport_t (80 or 443) */
} esp_http_client_config_t;

typedef struct esp_http_client esp_http_client_t;
typedef struct esp_http_client *esp_http_client_handle_t;

esp_http_client_handle_t esp_http_client_init(const esp_http_client_config_t *config);

esp_err_t esp_http_client_close(esp_http_client_handle_t client);

/**
 * @brief      This function will be open the connection, write all header strings and return
 *
 * @param[in]  client     The esp_http_client handle
 * @param[in]  write_len  HTTP Content length need to write to the server
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t esp_http_client_open(esp_http_client_handle_t client);

/**
 * @brief     This function will write data to the HTTP connection previously opened by esp_http_client_open()
 *
 * @param[in]  client  The esp_http_client handle
 * @param      buffer  The buffer
 * @param[in]  len     This value must not be larger than the write_len parameter provided to esp_http_client_open()
 *
 * @return
 *     - (-1) if any errors
 *     - Length of data written
 */
int esp_http_client_write(esp_http_client_handle_t client, const char *buffer, int len);



/**
 * @brief      Read data from http stream
 *
 * @param[in]  client  The esp_http_client handle
 * @param      buffer  The buffer
 * @param[in]  len     The length
 *
 * @return
 *     - (-1) if any errors
 *     - Length of data was read
 */
int esp_http_client_read(esp_http_client_handle_t client, char *buffer, int len);


/**
 * @brief      Get http response status code, the valid value if this function invoke after `esp_http_client_perform`
 *
 * @param[in]  client  The esp_http_client handle
 *
 * @return     Status code
 */
int esp_http_client_get_status_code(esp_http_client_handle_t client);

/**
 * @brief      Get http response content length (from header Content-Length)
 *             the valid value if this function invoke after `esp_http_client_perform`
 *
 * @param[in]  client  The esp_http_client handle
 *
 * @return
 *     - (-1) Chunked transfer
 *     - Content-Length value as bytes
 */
int64_t esp_http_client_get_content_length(esp_http_client_handle_t client);

/**
 * @brief      Close http connection, still kept all http request resources
 *
 * @param[in]  client  The esp_http_client handle
 *
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t esp_http_client_close(esp_http_client_handle_t client);

/**
 * @brief      This function must be the last function to call for an session.
 *             It is the opposite of the esp_http_client_init function and must be called with the same handle as input that a esp_http_client_init call returned.
 *             This might close all connections this handle has used and possibly has kept open until now.
 *             Don't call this function if you intend to transfer more files, re-using handles is a key to good performance with esp_http_client.
 *
 * @param[in]  client  The esp_http_client handle
 *
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t esp_http_client_cleanup(esp_http_client_handle_t client);



esp_err_t esp_http_client_get_url(esp_http_client_handle_t client, char *url, const int len);

#ifdef __cplusplus
}
#endif


#endif
