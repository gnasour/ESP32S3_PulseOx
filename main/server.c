#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define PORT 10000

static const char *TAG_SERVER = "Server: ";

int client_sock;

void do_retransmit(const int client_sock, const uint32_t *tx_buffer, const uint32_t tx_buffer_len)
{
    // int len;
    // char rx_buffer[128];

    // do
    // {
    //     len = recv(client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    //     if (len < 0)
    //     {
    //         ESP_LOGE(TAG_SERVER, "Error occurred during receiving: errno %d", errno);
    //     }
    //     else if (len == 0)
    //     {
    //         ESP_LOGW(TAG_SERVER, "Connection closed");
    //     }
    //     else
    //     {
    //         rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
    //         ESP_LOGI(TAG_SERVER, "Received %d bytes: %s", len, rx_buffer);

    //         // send() can return less bytes than supplied length.
    //         // Walk-around for robust implementation.
    //         int to_write = len;
    //         while (to_write > 0)
    //         {
    //             int written = send(client_sock, rx_buffer + (len - to_write), to_write, 0);
    //             if (written < 0)
    //             {
    //                 ESP_LOGE(TAG_SERVER, "Error occurred during sending: errno %d", errno);
    //                 // Failed to retransmit, giving up
    //                 return;
    //             }
    //             to_write -= written;
    //         }
    //     }
    // } while (len > 0);
    //printf("%ld,%ld,%ld\n", tx_buffer[0], tx_buffer[1], tx_buffer[2]);
    //printf("%ld,%ld\n", tx_buffer[1], tx_buffer[2]);
    send(client_sock, tx_buffer, tx_buffer_len, 0);
}

static void tcp_server_task(int pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET)
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0)
    {
        ESP_LOGE(TAG_SERVER, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG_SERVER, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err)
    {
        ESP_LOGE(TAG_SERVER, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG_SERVER, "IPPROTO: %d", addr_family);
        return;
    }
    ESP_LOGI(TAG_SERVER, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err)
    {
        ESP_LOGE(TAG_SERVER, "Error occurred during listen: errno %d", errno);
        return;
    }

    ESP_LOGI(TAG_SERVER, "Socket listening");

    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);
    client_sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
    if (client_sock < 0)
    {
        ESP_LOGE(TAG_SERVER, "Unable to accept connection: errno %d", errno);
        return;
    }

    // Convert ip address to string

    if (source_addr.sin_family == PF_INET)
    {
        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
    }
    ESP_LOGI(TAG_SERVER, "Socket accepted ip address: %s", addr_str);
}

void server_init(void)
{
    tcp_server_task(AF_INET);
}