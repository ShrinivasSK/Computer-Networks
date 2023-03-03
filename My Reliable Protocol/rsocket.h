#ifndef RSOCKET_H
#define RSOCKET_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include<pthread.h>
#define SOCK_MRF 1
#define TABLE_SIZE 100
#define MSG_SIZE 1000
#define __DEBUG_MODE__ 1
#define T 2
#define P 0.1
int min(int a, int b);
int dropMessage( float p);

pthread_mutex_t recvLock, sendLock, msgSendLock;
pthread_attr_t recvattr, sendattr;
typedef struct messageSentStruct{
    //public:
    int acknowledgement;
    time_t time;
    char message[MSG_SIZE];
    int messageSize;
    struct messageSentStruct *next;
    struct sockaddr *dest_addr;
    socklen_t addrlen;
} messageSent;
typedef struct messageReceivedStruct{
    char message[MSG_SIZE];
    int messageSize;
    int indexToBeRead;
    struct messageReceivedStruct *next;
    
} messageReceived;

messageSent unacknowledged_message_table[TABLE_SIZE];
messageReceived *received_message_table;

void initialiseTable();
pthread_t ptids[2];
pthread_attr_t attr;


void *R(void *arg);
void *S(void *arg);
int r_socket(int domain, int type, int protocol);
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags,struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen);
int r_close(int fd);
#endif
