/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <string.h>
#include "esp_log.h"
#include "esp_check.h"
#include "esp_http_client.h"
#include "errno.h"

#include <stdio.h>//printf
#include <string.h>//字符串处理
#include <sys/socket.h>//套接字
#include <arpa/inet.h>//ip地址处理
#include <fcntl.h>//open系统调用
#include <unistd.h>//write系统调用
#include <netdb.h>//查询DNS
#include <stdlib.h>//exit函数
#include <sys/stat.h>//stat系统调用获取文件大小
#include <sys/time.h>//获取下载时间
#include <stdbool.h>

static const char *TAG = "HTTP_CLIENT";

/**
 * HTTP client class
 */


/**
 * Default settings
 */
#define DEFAULT_HTTP_PORT (80)


static const char *HTTP_METHOD_MAPPING[] = {
    "GET",
    "POST",
    "PUT",
    "PATCH",
    "DELETE",
    "HEAD",
    "NOTIFY",
    "SUBSCRIBE",
    "UNSUBSCRIBE",
    "OPTIONS",
    "COPY",
    "MOVE",
    "LOCK",
    "UNLOCK",
    "PROPFIND",
    "PROPPATCH",
    "MKCOL"
};

static esp_err_t esp_http_client_request_send(esp_http_client_handle_t client);
static esp_err_t esp_http_client_connect(esp_http_client_handle_t client);


esp_http_client_handle_t esp_http_client_init(const esp_http_client_config_t *config)
{

    esp_http_client_handle_t client;
    esp_err_t ret = ESP_OK;
    char *host_name;
    bool _success;

    _success = (
                   (client                         = calloc(1, sizeof(esp_http_client_t)))           &&
                   (client->request                = calloc(1, sizeof(esp_http_data_t)))             &&
                   (client->response               = calloc(1, sizeof(esp_http_data_t)))
               );
    client->response->head_buffer = NULL;                                           
    client->request->head_buffer = NULL;
    if (!_success) {
        ESP_LOGE(TAG, "Error allocate memory");
        goto error;
    }

    client->connection_info.url = config->url;

    if (ret == ESP_OK) {
        return client;
    }

error:
    esp_http_client_cleanup(client);
    return NULL;
}

esp_err_t esp_http_client_close(esp_http_client_handle_t client)
{
    close(client->client_socket);
    return ESP_OK;
}

esp_err_t esp_http_client_cleanup(esp_http_client_handle_t client)
{
    if (client == NULL) {
        return ESP_FAIL;
    }

    free(client->response);
    free(client->request);
    free(client);
    client = NULL;
    return ESP_OK;
}

static  HTTP_RES_HEADER parse_header(const char *response)
{
    /*获取响应头的信息*/
     HTTP_RES_HEADER resp;

    char *pos = strstr(response, "HTTP/");
    if (pos)//获取返回代码
        sscanf(pos, "%*s %d", &resp.status_code);

    pos = strstr(response, "Content-Type:");
    if (pos)//获取返回文档类型
        sscanf(pos, "%*s %s", resp.content_type);

    pos = strstr(response, "Content-Length:");
    if (pos)//获取返回文档长度
        sscanf(pos, "%*s %ld", &resp.content_length);
    
    pos = strstr(response, "Location:");
    if (pos)//获取返回重定向地址
        sscanf(pos, "%*s %s", resp.location);

    return resp;
}

int esp_http_client_read(esp_http_client_handle_t client, char *buffer, int len)
{
    if((!client->response->head_buffer) || (!client->response->headers.content_length)){
        return 0;
    }
    int ridx = read(client->client_socket,buffer,len);

    return ridx;
}

static void get_ip_addr(char *host_name, char *ip_addr)
{
    /*通过域名得到相应的ip地址*/
    struct hostent *host = gethostbyname(host_name);//此函数将会访问DNS服务器
    if (!host)
    {
        ip_addr = NULL;
        return;
    }

    for (int i = 0; host->h_addr_list[i]; i++)
    {
        strcpy(ip_addr, inet_ntoa( * (struct in_addr*) host->h_addr_list[i]));
        break;
    }
}
static void parse_url(const char *url, char *host, int *port, char *file_name)
{
    /*通过url解析出域名, 端口, 以及文件名*/
    int j = 0;
    int start = 0;
    *port = 80;
    char *patterns[] = {"http://", "https://", NULL};

    for (int i = 0; patterns[i]; i++)//分离下载地址中的http协议
        if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
            start = strlen(patterns[i]);

    //解析域名, 这里处理时域名后面的端口号会保留
    for (int i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
        host[j] = url[i];
    host[j] = '\0';

    //解析端口号, 如果没有, 那么设置端口为80
    char *pos = strstr(host, ":");
    if (pos)
        sscanf(pos, ":%d", port);

    //删除域名端口号
    for (int i = 0; i < (int)strlen(host); i++)
    {
        if (host[i] == ':')
        {
            host[i] = '\0';
            break;
        }
    }

    //获取下载文件名
    j = 0;
    for (int i = start; url[i] != '\0'; i++)
    {
        if (url[i] == '/')
        {
            if (i !=  strlen(url) - 1)
                j = 0;
            continue;
        }
        else
            file_name[j++] = url[i];
    }
    file_name[j] = '\0';
}

static esp_err_t esp_http_client_connect(esp_http_client_handle_t client)
{
    esp_err_t err;
    
    if(!client->connection_info.url){
        return err;
    }
    parse_url(client->connection_info.url,client->connection_info.host,&client->connection_info.port,client->file_name);
    get_ip_addr(client->connection_info.host, client->connection_info.ip_addr);//调用函数同访问DNS服务器获取远程主机的IP
    //设置http请求头信息
    client->request->head_buffer = (char*) malloc(2048*sizeof(char));
    sprintf(client->request->head_buffer, \
            "GET %s HTTP/1.1\r\n"\
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"\
            "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537(KHTML, like Gecko) Chrome/47.0.2526Safari/537.36\r\n"\
            "Host: %s\r\n"\
            "Connection: keep-alive\r\n"\
            "\r\n"\
        ,client->connection_info.url, client->connection_info.host);

    puts("3: 创建网络套接字...");
    client->client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client->client_socket < 0)
    {
        printf("套接字创建失败: %d\n", client->client_socket);
        exit(-1);
    }

    //创建IP地址结构体
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(client->connection_info.ip_addr);
    addr.sin_port = htons(client->connection_info.port);

    //连接远程主机
    puts("4: 正在连接远程主机...");
    int res = connect(client->client_socket, (struct sockaddr *) &addr, sizeof(addr));
    if (res == -1)
    {
        printf("连接远程主机失败, error: %d\n", res);
        exit(-1);
    }
    return ESP_OK;
}


static esp_err_t esp_http_client_request_send(esp_http_client_handle_t client)
{
    puts("5: 正在发送http下载请求...");
    write(client->client_socket, client->request->head_buffer, strlen(client->request->head_buffer));//write系统调用, 将请求header发送给服务器

    return ESP_OK;
}

esp_err_t esp_http_client_open(esp_http_client_handle_t client)
{
    esp_err_t err;
    if ((err = esp_http_client_connect(client)) != ESP_OK) {
        return err;
    }
    if ((err = esp_http_client_request_send(client)) != ESP_OK) {
        return err;
    }
    if(!client->response->head_buffer){
        int mem_size = 4096;
        int length = 0;
        int len;
        char *buf = (char *) malloc(mem_size * sizeof(char));
        client->response->head_buffer = (char *) malloc(mem_size * sizeof(char));
        
        //每次单个字符读取响应头信息
        puts("6: 正在解析http响应头...");
        while ((len = read(client->client_socket, buf, 1)) != 0)
        {
            if (length + len > mem_size)
            {
                //动态内存申请, 因为无法确定响应头内容长度
                mem_size *= 2;
                char * temp = (char *) realloc(client->response->head_buffer, sizeof(char) * mem_size);
                if (temp == NULL)
                {
                    printf("动态内存申请失败\n");
                    exit(-1);
                }
                client->response->head_buffer = temp;
            }
        
            buf[len] = '\0';
            strcat(client->response->head_buffer, buf);
        
            //找到响应头的头部信息
            int flag = 0;
            for (int i = strlen(client->response->head_buffer) - 1; client->response->head_buffer[i] == '\n' || client->response->head_buffer[i] == '\r'; i--, flag++);
            if (flag == 4)//连续两个换行和回车表示已经到达响应头的头尾, 即将出现的就是需要下载的内容
                break;
        
            length += len;
        }
        
        client->response->headers = parse_header(client->response->head_buffer);
        printf("\n>>>>http响应头解析成功:<<<<\n");
        printf("\tHTTP响应代码: %d\n", client->response->headers.status_code);
        printf("\tHTTP Location: %s\n", client->response->headers.location);
        if (client->response->headers.status_code != 200)
        {
            printf("响应码不成功, 远程主机返回: %d\n", client->response->headers.status_code);
            return ESP_FAIL;
        }
        if (client->response->headers.content_length <= 0)
        {
            printf("没有响应体, 响应体长度返回: %ld\n", client->response->headers.content_length);
            return ESP_FAIL;
        }
        printf("\tHTTP文档类型: %s\n", client->response->headers.content_type);
        printf("\tHTTP主体长度: %ld字节\n\n", client->response->headers.content_length);
        
    
    
    }
    return ESP_OK;
}

int esp_http_client_write(esp_http_client_handle_t client, const char *buffer, int len)
{
    int widx=0;
    widx = write(client->client_socket,buffer,len);
    return widx;
}


int esp_http_client_get_status_code(esp_http_client_handle_t client)
{
    return client->response->headers.status_code;
}

int64_t esp_http_client_get_content_length(esp_http_client_handle_t client)
{
    return client->response->headers.content_length;
}