
// Client side C/C++ program to demonstrate Socket programming

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

int main()
{
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char input[100];
	char buffer[100] = {0};
	char ipAdd[20] = {0};
	char noIp[7] = "0.0.0.0";
	printf("Enter the DNS name: ");
    scanf("%s", input);
    //printf("%s\n",input);
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	struct timeval tv;
    fd_set waitfd;
    FD_ZERO(&waitfd);
    FD_SET(sock, &waitfd);
    tv.tv_sec = 2;
    tv.tv_usec = 0;

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	int errr = send(sock, input, (int)strlen(input) + 1, 0);
	if(select(sock + 1, &waitfd, NULL, NULL, &tv) == -1) perror("select");
	if (FD_ISSET(sock, &waitfd)){
        int firstRecieved = 0;
        int firstStringIndex = 0;
        int n;
        while(n = recv(sock, (char *)buffer, 100, 0) > 0){
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
                        close(sock);
                        exit(0);
                    }
                    printf("%s", ipAdd);
                    while(i < 100 && buffer[i] != '\0'){
                        if (buffer[i] == '$'){
                            close(sock);
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
                        close(sock);
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
           
    close(sock); 





		
	close(sock);
	return 0;
}
