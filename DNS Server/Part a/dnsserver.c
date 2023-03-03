/*
NETWORKS LAB ASSIGNMENT 2a : DNS Server-Client
DNS Server Code
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
#include <ctype.h>
#include <netdb.h>

// Buffer size of the received data
#define BUFF_SIZE 100

int main(){
    int	sockfd ; /* Socket descriptors */
    socklen_t len;
	struct sockaddr_in	cli_addr, serv_addr; /*Client and server addresses*/

	char buf[BUFF_SIZE+1];	/* We will use this buffer for communication */

    // Create a socket. Check if there is any error too. 
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Error Occurred. Could not create socket\n");
		exit(0);
	}
    printf("Created a socket to connect  to client.\n");

    memset(&serv_addr, 0, sizeof(serv_addr)); 
    memset(&cli_addr, 0, sizeof(cli_addr)); 

	/* Assign the properties of the server. */
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	/* Associate the server with its port using the bind() system call. */
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Error occurredn. Unable to bind local address\n");
		exit(0);
	}
    printf("Binded the socket successfully to the local address\n");

    while(1){
        len = sizeof(cli_addr);
        recvfrom(sockfd,buf,BUFF_SIZE,0,( struct sockaddr *) &cli_addr, &len);

        printf("Received DNS Name: %s\n",buf);
        struct hostent *res =gethostbyname(buf);

        char sendData[BUFF_SIZE+1];

        for(int i=0;i<=BUFF_SIZE;i++){
            sendData[i]='\0';
        }

        if(res==NULL){
            printf("IP Address not found\n");
            strcpy(sendData,"0.0.0.0");
            sendData[8]='\0';
            sendto(sockfd,sendData,strlen(sendData),0,(struct sockaddr *) &cli_addr, len);
            for(int i=0;i<=BUFF_SIZE;i++){
                sendData[i]='\0';
            }
        }else{
            struct in_addr ip_addr;

            for(int i=0;res->h_addr_list[i]!=NULL;i++){
                ip_addr = *(struct in_addr *)(res->h_addr_list[i]);
                strcpy(sendData,inet_ntoa(ip_addr));
                printf("Sending IP address: %s\n",sendData);
                sendto(sockfd,sendData,strlen(sendData),0,(struct sockaddr *) &cli_addr, len);
                for(int i=0;i<=BUFF_SIZE;i++){
                    sendData[i]='\0';
                }
            }
        }
        printf("Sending end signal.\n");
        sendData[0]='$';
        sendData[1]='\0';
        sendto(sockfd,sendData,strlen(sendData),0,(struct sockaddr *) &cli_addr, len);
    }
    
    close(sockfd);
    return 0;
}