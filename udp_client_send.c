#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <file_path> <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open the file to send
    FILE* fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
    serv_addr.sin_port = htons(atoi(argv[3]));

    // Send the file
    char buf[BUF_SIZE];
    size_t nread;
    while ((nread = fread(buf, 1, BUF_SIZE, fp)) > 0) {
        int bytes_sent;
        if ((bytes_sent=sendto(sockfd, buf, nread, 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) < 0) {
            perror("Failed to send data");
            exit(EXIT_FAILURE);
        }else{
            //printf("Sent %d bytes to %d:%d\n", bytes_sent, serv_addr.sin_addr.s_addr, serv_addr.sin_port);

        }
    }

    // Close the file and socket
    if (fclose(fp) != 0) {
        perror("fclose");
        exit(1);
    }
    close(sockfd);

    return 0;
}
