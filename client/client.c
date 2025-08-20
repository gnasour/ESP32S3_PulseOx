#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define PORT 50000
#define RX_BUF_LEN 128

int server_init(int pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_storage dest_addr;
    int client_sock;

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
        printf("Unable to create socket: errno %d\n", errno);
        return -1;
    }
    printf("Socket created\n");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err)
    {
        printf("Socket unable to bind: errno %d\n", errno);
        printf("IPPROTO: %d\n", addr_family);
        perror("BIND()");
        return -1;
    }

    err = listen(listen_sock, 1);
    if (err)
    {
        printf("Error occurred during listen: errno %d\n", errno);
        return -1;
    }
    printf("Socket listening\n");
    
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);
    client_sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
    if (client_sock < 0)
    {
        printf("Unable to accept connection: errno %d\n", errno);
        return -1;
    }

    return client_sock;

}

int main(int argc, char *argv[])
{
    ssize_t numRead;
    struct  addrinfo hints;
    struct  addrinfo *result, *rp;
    int     recv_sock = server_init(AF_INET);

    // Make FIFO to sync with data processing task
    int res = mkfifo("temp", S_IRUSR|S_IWUSR);
    if(res == -1 && errno != EEXIST)
    {
        return -2;
    }

    int num_read;
    char rx_buf[RX_BUF_LEN];
    int temp_file = open("temp", O_TRUNC|O_WRONLY);
    memset(rx_buf, 0, RX_BUF_LEN);
    printf("Reading from socket now\n");
    while(1)
    {
        num_read = recv(recv_sock, rx_buf, RX_BUF_LEN, SOCK_NONBLOCK);
        if(num_read <= 0)
        {
            printf("recv error\n");
            exit(-1);
        }
        write(temp_file, rx_buf, num_read);
    }
}