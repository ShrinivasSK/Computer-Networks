// A Simple Client Implementation
#include <stdio.h>
#include <fcntl.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#define PORT 8080
#define CHUNK_SIZE 20
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    char buffer[100] = {0};
    char input[100];
    char ipAdd[20] = {0};
    printf("Enter the DNS name: ");
    scanf("%s", input);
    //printf("%s\n",input);
    int n;
    socklen_t len; 
    
    struct timeval tv;
    fd_set waitfd;
    FD_ZERO(&waitfd);
    FD_SET(sockfd, &waitfd);
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    sendto(sockfd, (const char *)input, strlen(input) + 1, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    int leng = sizeof(servaddr);
    char noIp[7] = "0.0.0.0";
    if(select(sockfd + 1, &waitfd, NULL, NULL, &tv) == -1) perror("select");
    //printf("Yes\n");
    if (FD_ISSET(sockfd, &waitfd)){

        int firstRecieved = 0;
        int firstStringIndex = 0;
        int n;
        while(n = recvfrom(sockfd, (char *)buffer, 100, 0, ( struct sockaddr *) &servaddr, &leng) > 0){
            //printf("%s\n" ,buffer);
            if (firstRecieved == 0){
                int i = 0;
                while (i < 100 && buffer[i] != '\0'){
                    ipAdd[firstStringIndex] = buffer[i];
                    if (buffer[i] == '\n'){
                        firstRecieved = 1;
                        i++;
                        break;
                    }
                    firstStringIndex++;
                    i++;
                }
                if (firstRecieved == 1){
                    int ok = 1;
                    for (int ind = 0; ind < 7; ind++){
                        if (ipAdd[ind] != noIp[ind]) ok = 0;
                    }
                    if (ok == 1){
                        printf("No Ip Address Found\n");
                        close(sockfd);
                        exit(0);
                    }
                    printf("%s", ipAdd);
                    while(i < 100 && buffer[i] != '\0'){
                        if (buffer[i] == '$'){
                            close(sockfd);
                            exit(0);
                        }
                        printf("%c", buffer[i]);
                        i++;
                    }
                }
            }
            else{
                int i = 0;
                while(i < 100 && buffer[i] != '\0'){
                    if (buffer[i] == '$'){
                        close(sockfd);
                        exit(0);
                    }
                    printf("%c", buffer[i]);
                    i++;
                }
            }
            for (int i = 0; i < 100; i++) buffer[i] = '\0';
        }
    }
    else{
        printf("Server is too busy.\n");
    }
           
    close(sockfd); 
    return 0; 
} 
