#include "udp_client.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>

void send_file(const char *filename)
{
    int sockfd, fd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    off_t offset = 0;
    struct stat file_stat;
    fstat(fd, &file_stat);
    size_t file_size = file_stat.st_size;

    for (int i = 0; offset < file_size; i++)
    {
        ssize_t sent_bytes = sendfile(sockfd, fd, &offset, file_size - offset);
        if (sent_bytes == -1)
        {
            perror("File send failed");
            exit(EXIT_FAILURE);
        }
        printf("Sent %zd bytes of file\n", sent_bytes);
    }

    close(fd);
    close(sockfd);
}

void receive_file(const char *filename)
{
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd == -1)
    {
        perror("File creation failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t n = 0;
    while ((n = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
    {
        ssize_t written_bytes = write(fd, buffer, n);
        if (written_bytes == -1)
        {
            perror("File write failed");
            exit(EXIT_FAILURE);
        }
        printf("Received %zd bytes of file\n", n);
    }

    close(fd);
    close(sockfd);
}
