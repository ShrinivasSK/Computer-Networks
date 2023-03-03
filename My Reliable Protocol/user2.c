#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "rsocket.h"
   
#define PORT  50101
#define MAXLINE 1024
   
// Driver code
int main() {
    int sockfd;
    char buffer[MAXLINE];
    char *hello = "Hello from u2";
    struct sockaddr_in servaddr, cliaddr;
       
    // Creating socket file descriptor
    if ( (sockfd = r_socket(AF_INET, SOCK_MRF, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
       
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
       
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
       
    // Bind the socket with the server address
    if ( r_bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
       
    int len, n;
   
    len = sizeof(cliaddr);  //len is value/resuslt
    while(1){
        n = r_recvfrom(sockfd, (char *)buffer, MAXLINE, 
                    0, ( struct sockaddr *) &cliaddr,
                    &len);
        buffer[n] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }
    //r_sendto(sockfd, hello, strlen(hello), 0,  (struct sockaddr *)&cliaddr,len);
    getchar();
    return 0;
}