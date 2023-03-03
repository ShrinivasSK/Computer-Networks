#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "rsocket.h"
   
#define PORT     50100
#define MAXLINE 1024
   
// Driver code
int main() {
    int sockfd;
    char buffer[MAXLINE];
    char *hello = "Hello from u1\0";
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

    cliaddr.sin_family    = AF_INET; // IPv4
    cliaddr.sin_addr.s_addr = INADDR_ANY;
    cliaddr.sin_port = htons(PORT + 1);

       
    // Bind the socket with the server address
    if ( r_bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
       
    int len, n;
   
    len = sizeof(cliaddr);  
    int stringLength;
    printf("Length of the string:");
    scanf("%d", &stringLength);
    getchar();
    char *string = (char *)malloc(sizeof(char) * (stringLength + 1));
    printf("Enter String: ");
    scanf("%[^\n]s",string);
    string[stringLength] = '\0';
    for (int i = 0; i < stringLength; i++){
        if (r_sendto(sockfd, &string[i], 1, 0,  (struct sockaddr *)&cliaddr,len) == -1){
            printf("Sending failed\n");
        }
    }
    while(1){}

    printf("String %s sent\n", string); 
       
    return 0;
}