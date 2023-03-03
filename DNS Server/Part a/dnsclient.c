/*
NETWORKS LAB ASSIGNMENT 2a : DNS Server-Client
DNS UDP Client Code
Shrinivas Khiste
19CS30043
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <unistd.h>

#define MAX_LEN 100
#define RECV_LEN 1000

int main(){
    char dns_name[MAX_LEN];
    printf("Enter a DNS name: ");
    scanf("%s",dns_name);

    int	sockfd; /* Socket descriptors */
	struct sockaddr_in	serv_addr; /*Server address*/

    memset(&serv_addr, 0, sizeof(serv_addr)); 

    // Create a socket. Check if there is any error too. 
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}
    printf("Created Socket to connect to Server.\n");

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec=0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);
    printf("Sending DNS name to server.\n");
    sendto(sockfd, dns_name, strlen(dns_name), 0,(const struct sockaddr *) &serv_addr, sizeof(serv_addr));

    char buf[RECV_LEN+1];
    for(int i=0;i<=RECV_LEN;i++){
        buf[i]='\0';
    }

    socklen_t len = sizeof(serv_addr);
    int recv_bytes;
    int flag=0;
    printf("Waiting for data from server..\n");
    while((recv_bytes=recvfrom(sockfd, buf, RECV_LEN, 0,(struct sockaddr *)&serv_addr,&len))>0){
        if(strcmp(buf,"$")==0) break;

        if(strcmp(buf,"0.0.0.0")==0){
            printf("IP address not found\n");
            break;
        }
        if(flag==0){
            printf("IP Address(es) are as follows:\n");
            flag=1;
        }
        printf("%s\n",buf);
        for(int i=0;i<=RECV_LEN;i++){
            buf[i]='\0';
        }
    }

    if(recv_bytes<0){
        printf("\nConnection Timeout. Exitting.\n");
        exit(0);
    }else{
        printf("\nSuccessfully received data. Closing socket.\n");
    }

    close(sockfd);

    return 0;
}
