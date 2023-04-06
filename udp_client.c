#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFLEN 512
#define PORT 8888

void die(char *s) {
    perror(s);
    exit(1);
}

int main(int argc, char** argv) {
    struct sockaddr_in si_other;
    int s, slen = sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];

    // Create a UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }

    // Zero out the structure
    memset((char *) &si_other, 0, sizeof(si_other));

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(atoi(argv[1]));

    // Set the destination IP address
    if (inet_aton("127.0.0.1", &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    // Send messages to the server
    while (1) {
        printf("Enter message : ");
        fgets(message, BUFLEN, stdin);

        // Send the message
        if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == -1) {
            die("sendto()");
        }

        // Receive a reply and print it
        memset(buf, '\0', BUFLEN);
        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1) {
            die("recvfrom()");
        }

        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Data: %s\n", buf);
    }

    close(s);
    return 0;
}
