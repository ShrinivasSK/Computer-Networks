/*
NETWORKS LAB ASSIGNMENT 1a : USING TCP SOCKETS
TCP Client Code
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

// size of the chunk being sent from the client
#define CHUNK_SIZE 20

int main(int argc, char *argv[])
{
	// check whether the command line arguments are valid
    if(argc!=2){
        printf("Please specify file name in command line arguments.\n");
        exit(0);
    }

	// open the file as read_only
    int fd = open(argv[1], O_RDONLY);

	// check if the file has been opened correctly
    if(fd<0){
        printf("File not found. Exitting.\n");
        exit(0);
    }

	int	sockfd;  /* Socket descriptors */
	struct sockaddr_in	serv_addr; /*Server address*/


	// Create a socket. Check if there is any error too.
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	printf("Created Socket to connect to Server.\n");

	/*  Since the client can run on a different, we need to specify the 
        IP address of the server. 

	    In this program, we assume that the server is running on the
	    same machine as the client. 127.0.0.1 is a special address
	    for "localhost" (this machine) 
    */
	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	/* With the information specified in serv_addr, the connect()
	   system call establishes a connection with the server process.
	*/
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	printf("Succesfully connected to the Server!\n");
	char buf[CHUNK_SIZE+1]; /* We will use this buffer for communication */

	// initialise Buffer to null character so that 
    // no garbage values are counted. 
	for(int i=0;i<CHUNK_SIZE+1;i++){
		buf[i]='\0';
	}

	size_t file_bytes;

	printf("Reading File and sending to Server.....\n");
	
	/**
     * Read the file in packets of CHUNK_SIZE and send them to 
     * the server. Read returns the number of bytes read.
     * If it returns value=0 then it means the file has ended
     * If it returns value<0 then there is an error. 
     */
	while((file_bytes=read(fd,buf,CHUNK_SIZE))>0){

        send(sockfd, buf, strlen(buf), 0);

		printf("Sent:  %s\n",buf);

        for(int i=0;i<CHUNK_SIZE+1;i++){
			buf[i]='\0';
		}
    }
	// check if there was any error in reading the file
    if(file_bytes<0){
        printf("Error in reading file. Exitting.");
        close(sockfd);
        close(fd);
        exit(0);
    }

	/**
     * Sending the file completion signal. 
     * Sending D.. instead of .. as if the file ends with . Then 
     * this last full stop will not be counted as that with this 
     * packets . will become 2 dots. 
     * Sending D.. will get rid of this problem. 
     * Here we are not assuming that the file ends with a dot always.
     */
	printf("Read the file completely.Sending completion signal.\n");
	strcpy(buf,"D..");
	buf[3]='\0';
	send(sockfd, buf, strlen(buf), 0);

	int numChar=0,numWords,numSent;
	printf("Waiting for result form server.\n");
	// Receive the number of characters, words and sentences from
    // the server and print them
    recv(sockfd, &numChar, sizeof(numChar), 0);
	printf("Number of Characters: %d\n",numChar);

	recv(sockfd, &numWords, sizeof(numWords), 0);
	printf("Number of Words: %d\n",numWords);

	recv(sockfd, &numSent, sizeof(numSent), 0);
	printf("Number of Sentences: %d\n",numSent);

	// close the socket from client side
	printf("Closing connection from client side.\n");
	close(sockfd);

	// close the file
    close(fd);
	return 0;

}

