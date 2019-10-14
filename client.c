/*
** client.c -- a stream socket client demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAXDATASIZE 512  // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    // sockfd: socket file descriptor
    int sockfd;
    char buf[MAXDATASIZE];

    // address info
    struct addrinfo hints, *servinfo, *p;
    // return value
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 3) {
        fprintf(stderr,"usage: client hostname port\n");
        exit(1);
    }

    const char* host = argv[1];
    const char* port = argv[2];

    printf("Connecting to %s:%s\n", host, port);

    memset(&hints, 0, sizeof hints);    // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;        // ipv4 or ipv6
    hints.ai_socktype = SOCK_STREAM;    // tcp

    if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    // address to print
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);

    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    // message to send
    const char* msg = "A53308625\r\n";
    int len = strlen(msg), bytes_sent;

    // the bytes sent out
    bytes_sent = send(sockfd, msg, len, 0);

    if (bytes_sent != len) {
        fprintf(stderr,"send() failed\n");
        exit(1);
    }

    // Receive the response
    ssize_t numBytes;
    // Save results to file or use grep to get the title
    //FILE* fptr = fopen("result.txt", "w");
    do {
        char buffer[MAXDATASIZE]; // I/O buffer
        numBytes = recv(sockfd, buffer, MAXDATASIZE - 1, 0);
        if (numBytes < 0)
            fprintf(stderr,"recv() failed\n");
        else if (numBytes == 0)
            break;
        buffer[numBytes] = '\0';    // Terminate the string!
        fputs(buffer, stdout);      // Print the echo buffer
        //fputs(buffer, fptr);
    } while (numBytes > 0);

    fputc('\n', stdout); // Print a final linefeed
    //fputc('\n', fptr);
    //fclose(fptr);
    fprintf(stderr, "Closed the socket!\n");

    exit(0);
}
