#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    char message[]="message by client";
    int sock = 0;
    struct sockaddr_in addr;
    char buffer[1024] = {0};
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("socket error");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("connect error");
        exit(EXIT_FAILURE);
    }
    else printf("Connection succeed\n");

    send(sock, message, strlen(message), 0);
    printf("Message sent: %s\n", message);
    close(sock);
    return 0;
}
