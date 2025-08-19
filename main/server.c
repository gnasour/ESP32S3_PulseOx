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

#define PORT 50000

static const char *TAG_SERVER = "Server: ";

int recv_sock;

void do_retransmit(const char *tx_data, const int tx_data_len)
{ 
    
    send(recv_sock, tx_data, tx_data_len, 0);
}

static void tcp_client_task(int pvParameters)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    const char *nodename = "192.168.1.42";
    const char *servname = "50000";

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = 0;
    hints.ai_addr = 0;
    hints.ai_next = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    if(getaddrinfo(nodename, servname, &hints, &result) != 0)
    {
        ESP_LOGE(TAG_SERVER, "Error in getting server info");
        while(1);
    }

    for(rp = result; rp != NULL; rp = rp->ai_next)
    {
        recv_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(recv_sock == -1)
        {
            continue;
        }
        // Success
        if(connect(recv_sock, rp->ai_addr, rp->ai_addrlen) != -1)
        {
            break;
        }
        // Client failed to find an applicable server
        close(recv_sock); 

        if(rp == NULL){
            rp = result;
        }
    }
    printf("connected socket: %d\n", recv_sock);
    if(rp == NULL)
    {
        ESP_LOGE(TAG_SERVER, "Error in server resolution");
        while(1);
    }
    
    freeaddrinfo(result);

    ESP_LOGI(TAG_SERVER, "ESP32 Socket connected successfully");

}


void server_init(void)
{
    tcp_client_task(AF_INET);
}