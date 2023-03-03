// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <string.h>
#include <netdb.h>
#define PORT_TCP 8080
#define PORT_UDP 8181

int max(int x, int y){
	return x > y ? x : y;
}

int main()
{

	char buffer[100] = {0};
	char noIp[8] = "0.0.0.0\n";


	int server_tcp, valread_tcp;
	struct sockaddr_in address_tcp;
	int opt = 1;
	int addrlen_tcp = sizeof(address_tcp);	
	// Creating socket file descriptor
	if ((server_tcp = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_tcp, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address_tcp.sin_family = AF_INET;
	address_tcp.sin_addr.s_addr = INADDR_ANY;
	address_tcp.sin_port = htons( PORT_TCP );
	
	// Forcefully attaching socket to the port 8080
	if (bind(server_tcp, (struct sockaddr *)&address_tcp,sizeof(address_tcp))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_tcp, 5) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}




	int sock_udp, len_udp; 
    struct sockaddr_in servaddr_udp, cliaddr_udp; 
      
    // Create socket file descriptor 
    sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sock_udp < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr_udp, 0, sizeof(servaddr_udp)); 
    memset(&cliaddr_udp, 0, sizeof(cliaddr_udp)); 
    len_udp = sizeof(cliaddr_udp);
      
    servaddr_udp.sin_family    = AF_INET; 
    servaddr_udp.sin_addr.s_addr = INADDR_ANY; 
    servaddr_udp.sin_port = htons(PORT_UDP); 
      
    // Bind the socket with the server address 
    if ( bind(sock_udp, (const struct sockaddr *)&servaddr_udp, sizeof(servaddr_udp)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    printf("Server Running...\n");

    fd_set waitfd;
    FD_ZERO(&waitfd);
    FD_SET(sock_udp, &waitfd);
    FD_SET(server_tcp, &waitfd);

    while(1){
    	FD_ZERO(&waitfd);
    	FD_SET(sock_udp, &waitfd);
    	FD_SET(server_tcp, &waitfd);

    	if(select(max(sock_udp,server_tcp) + 1, &waitfd, NULL, NULL, NULL) == -1) perror("select");

    	if (FD_ISSET(sock_udp,&waitfd)){
			//printf("udp\n");
			char ipAddresses[100];
			for (int i = 0; i < 100; i++){
	            ipAddresses[i] = '\0';
	        }
	        int n = recvfrom(sock_udp, (char *)buffer, 100, 0, ( struct sockaddr *) &cliaddr_udp, &len_udp);
	        //printf("%s\n", buffer);
	        struct hostent *lh = gethostbyname(buffer);
	        struct in_addr ipAddr;
	        if (lh == NULL){
	            sendto(sock_udp, (char *)noIp, sizeof(noIp), 0, (const struct sockaddr *) &cliaddr_udp, len_udp);
	        }
	        else{
	            for (int i = 0; lh->h_addr_list[i] != NULL; i++){
	                //printf("%s\n", buffer);
	                //printf("%d\n", i);
	                ipAddr = *(struct in_addr*) (lh->h_addr_list[i]);
	                strcpy(ipAddresses,inet_ntoa(ipAddr));
	                //printf("%s\n", ipAddresses);
	                //printf("%d\n", (int)strlen(ipAddresses));
	                for (int i = 0; i < 100; i++){
	                    if (ipAddresses[i] == '\0'){
	                        ipAddresses[i] = '\n';
	                        break;
	                    }
	                }
	                //printf("%s\n", ipAddresses);
	                //printf("%d\n", (int)strlen(ipAddresses));
	                sendto(sock_udp, (char *)ipAddresses, strlen(ipAddresses), 0, (const struct sockaddr *) &cliaddr_udp, len_udp);     
	                for (int i = 0; i < 100; i++){
	                    ipAddresses[i] = '\0';
	                }
	            }
	            ipAddresses[0] = '$';
	            sendto(sock_udp, (char *)ipAddresses, strlen(ipAddresses), 0, (const struct sockaddr *) &cliaddr_udp, len_udp);     

	        }
    	}
    	if (FD_ISSET(server_tcp, &waitfd)){
    		//printf("tcc\n");
    		char ipAddresses[100] = {0};
    		int new_socket_tcp = accept(server_tcp, (struct sockaddr *)&address_tcp,(socklen_t*)&addrlen_tcp);
    		int pid = fork();
    		if (pid == 0){
    			close(server_tcp);



    			recv(new_socket_tcp, buffer,100,0);
    			//printf("%s", buffer);

    			struct hostent *lh = gethostbyname(buffer);
    			struct in_addr ipAddr;
    			if (lh == NULL){
		            send(new_socket_tcp, (char *)noIp, sizeof(noIp), 0);
		        }
		        else{
		            for (int i = 0; lh->h_addr_list[i] != NULL; i++){
		                //printf("%s\n", buffer);
		                //printf("%d\n", i);
		                ipAddr = *(struct in_addr*) (lh->h_addr_list[i]);
		                strcpy(ipAddresses,inet_ntoa(ipAddr));
		                //printf("%s\n", ipAddresses);
		                //printf("%d\n", (int)strlen(ipAddresses));
		                for (int i = 0; i < 100; i++){
		                    if (ipAddresses[i] == '\0'){
		                        ipAddresses[i] = '\n';
		                        break;
		                    }
		                }
		                //printf("%s\n", ipAddresses);
		                //printf("%d\n", (int)strlen(ipAddresses));
		                send(new_socket_tcp, (char *)ipAddresses, strlen(ipAddresses), 0);     
		                for (int i = 0; i < 100; i++){
		                    ipAddresses[i] = '\0';
		                }
		            }
		            ipAddresses[0] = '$';
		            send(new_socket_tcp, (char *)ipAddresses, strlen(ipAddresses), 0);     

		        }

    			close(new_socket_tcp);
    			exit(0);
    		}
    		close(new_socket_tcp);


    	}
    }
    close(server_tcp);
    close(sock_udp);





	return 0;
}
