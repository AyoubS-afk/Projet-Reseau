#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    int port_number;
    char *ip_address;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <ip_address> <port_number>\n", argv[0]);
        exit(1);
    }

    ip_address = argv[1];
    port_number = atoi(argv[2]);

    // create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    server_addr.sin_addr.s_addr = inet_addr(ip_address);

    // bind socket to local address
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port_number);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr));
    while (1)
    {
        // receive message from client
        int addr_len = sizeof(client_addr);
        int num_bytes = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);

        // print received message
        printf("Received message: %.*s\n", num_bytes, buffer);

        // send response to client
        sendto(sockfd, buffer, num_bytes, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    }

    // close socket
    close(sockfd);

    return 0;
}