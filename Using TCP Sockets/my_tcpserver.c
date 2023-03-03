/*
NETWORKS LAB ASSIGNMENT 1a : USING TCP SOCKETS
TCP Server Code
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
#include<ctype.h>

// Buffer size of the received data
#define BUFF_SIZE 100

// function to check whether a character 
// is alphabumeric or not
int isAlphaNumeric(char ch){
	ch = tolower(ch);
	if(ch>='a' && ch<='z'){
		return 1;
	}else if(ch>='0' && ch<='9'){
		return 1;
	}
	return 0;
}


int main()
{
	int	sockfd, newsockfd ; /* Socket descriptors */
	int	clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[BUFF_SIZE+1];		/* We will use this buffer for communication */

    // Create a socket. Check if there is any error too. 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error Occurred. Could not create socket.\n");
		exit(0);
	}
	printf("Created a socket to connect  to client.\n");

	/* Assign the properties of the server. */
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	/* Associate the server with its port using the bind() system call. */
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	printf("Binded the socket successfully to the local address\n");
    /* Start listening for incoming connections. 5 is the queue size that 
        specifies that upto 5 clients can simultaneously request to the 
        server and their requests will be queued up till the accept call
        of the current request is being processed. 
    */
   	printf("Listening for client request....\n");
	listen(sockfd, 5); 

	while (1) {

		/* Accept function returns when it gets a request from the 
            client. It returns a new socket descriptor that is used 
            for communicating with the current client 
		*/
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen);
		

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}

		printf("Successfully connected to the client.\n");

		size_t recv_bytes;

		// A word is a continuous block of alphanumeric values.
    	// character is all of alphanumeric, spaces, dots and new line
    	// Number of sentences is equal to number of dots. 
		int numWords=0,numChars=0,numSent=0;

		int done = 0; // flag to check whether file has ended.

		// initialise Buffer to null character so that 
    	// no garbage values are counted. 
		for(int i=0;i<BUFF_SIZE+1;i++){
			buf[i]='\0';
		}	

		// flag to check if the previous character is alphanumeric
		// that is a word is being parsed currently. 
		// this is defined outside recvfrom loop as it will also 
		// be maintained between 2 packets as words can be split
		// between packets too. 
		int isWord = 0;

		/**
		 * Start receiving the file from the Client. Recv returns number of
		 * bytes received from the client. If it is negative then it means there 
		 * is an error. It returns 0 if the connection is closed from the client side
		 */
		printf("Receiving Data from the client....\n");
		while((recv_bytes = recv(newsockfd,buf,BUFF_SIZE,0))>0){
			buf[BUFF_SIZE]='\0';

			printf("Received:  %s\n",buf);

			// counting all alphanumeric characters, spaces, dots 
			// and new line as characters. 	
			numChars+=strlen(buf);

			for(int i=0;i<strlen(buf);i++){
				
				if(isAlphaNumeric(buf[i])){
					// if the previous character was not 
					// alphanumeric then this is a new word
					// this also works if the word is broken 
					// in between packets (if it is broken while
					// splitting the file)
					if(isWord==0){
						numWords++;
					}
					isWord=1;
				}else if(buf[i]=='.'){
					// number of full stops is equal to the number of 
                	// sentences. 
					isWord=0;
					// checking whether we have received double
                	// dots which indicate end of file. 
					if(i+1<strlen(buf) && buf[i+1]=='.'){
						done=1;
						break;
					}
					numSent++;
				}else{ // current character is a space. 
					isWord=0;
				}
			}

			// break while loop when complete file is received.
			if(done){
				break;
			}

			// initialise Buffer to null character so that 
        	// no garbage values are counted. 
        	for(int i=0;i<BUFF_SIZE+1;i++){
				buf[i]='\0';
			}

		}

		// check if there was any error in receving the data
		if(recv_bytes<0){
			printf("Error in receiving bytes. Exitting.");
			close(sockfd);
			exit(0);
		}
		printf("Received the file from the client.\n");
		// remove the extra word "D" from D.. packet
		// For logic of sending D.. packet. Look at client code
		numWords--;
		// do not count the last three characters in D..
		numChars-=3;

		printf("Sending the results to the client.\n");
		// send the numbers back to the client
		send(newsockfd,&numChars,sizeof(numChars),0);
		send(newsockfd,&numWords,sizeof(numWords),0);
		send(newsockfd,&numSent,sizeof(numSent),0);		

		// close the connection with the client
		close(newsockfd);
		printf("Closed the connection with the client.\n");
		if(done){
			break;
		}
	}
	printf("Closing the main socket connection.\n");
	// close the socket in the end
	close(sockfd);

	return 0;
}
			

