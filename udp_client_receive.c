#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <output_file_path> <listen_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Bind to the specified port
    struct sockaddr_in serv_addr, cli_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[2]));
    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // Open the output file
    FILE* fp = fopen(argv[1], "wb");
    if (fp == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Receive the file
    char buf[BUF_SIZE];
    ssize_t nread;
    socklen_t cli_len = sizeof(cli_addr);
    while ((nread = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*) &cli_addr, &cli_len)) > 0) {
        //printf("Received %ld bytes from %s:%d\n", nread, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        //printf("%s",&buf);
        if (fwrite(buf, sizeof(char), nread, fp) < nread) {
            perror("Failed to write data");
            exit(EXIT_FAILURE);
        }else{
            exit(EXIT_SUCCESS);
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