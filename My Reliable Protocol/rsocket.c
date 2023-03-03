#include "rsocket.h"

int min(int a, int b){
    if (a < b) return a;
    return b;
}

int dropMessage( float p){
    float x = ((float)rand()/(float)(RAND_MAX));
    return (x < p ? 1 : 0);
}

void initialiseTable(){
    for (int i = 0; i < TABLE_SIZE; i++){
        unacknowledged_message_table[i].acknowledgement = -1;
        unacknowledged_message_table[i].time = -1;
        for (int j = 0; j < MSG_SIZE; j++)
            unacknowledged_message_table[i].message[j] = -1;
        unacknowledged_message_table[i].next = NULL;
        unacknowledged_message_table[i].dest_addr = NULL;
        unacknowledged_message_table[i].addrlen = 0;
    }
}

void *R(void *arg){
    int *id = (int *)arg;
    int sockfd = *id;
    struct sockaddr *src_addr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
    socklen_t addrlen = sizeof(struct sockaddr);
    char *type = (char *) malloc(sizeof(char));
    while (1){
        recvfrom(sockfd, type, 1, 0, src_addr, &addrlen);
        if (*type == 'A'){
            int num;
            int r = recvfrom(sockfd, &num, sizeof(int), 0, src_addr, &addrlen);
            if (r < 0){
                printf("Error in receiving header\n");
            }
            if (dropMessage(P) == 1){
                continue;
            }
            pthread_mutex_lock(&sendLock);
            unacknowledged_message_table[num - 1].acknowledgement = -1;
            for (int i = 0; i < MSG_SIZE; i++) 
                unacknowledged_message_table[num - 1].message[i] = '\0';
            pthread_mutex_unlock(&sendLock);
        }
        else if (*type == 'M'){
            uint16_t num;
            recvfrom(sockfd, &num, sizeof(int), 0, src_addr, &addrlen);

            int len = num;
            uint16_t ack;
            recvfrom(sockfd, &ack, sizeof(int), 0, src_addr, &addrlen);

            messageReceived *msg = ( messageReceived *) malloc(sizeof(messageReceived));
            msg -> messageSize = len;
            msg -> indexToBeRead = 0;
            msg ->next = NULL;
            char data[MSG_SIZE];
            int id = 0;
            while (len > 0){
                
                int r = recvfrom(sockfd, data,min(MSG_SIZE,len), 0, src_addr, &addrlen);
                if (r < 0) {
                    printf("Error in receiving Data\n");
                }
                for (int i = 0;i < r; i++) msg->message[id++] = data[i];
                len-=r;
                for (int i = 0; i < MSG_SIZE; i++) data[i] = '\0';
            }
            if (dropMessage(P) == 1){
                continue;
            }
            pthread_mutex_lock(&recvLock);
            if (received_message_table == NULL){
                received_message_table = msg;
            }
            else{
                messageReceived * temp = received_message_table;
                messageReceived *temp2 = temp -> next;
                while (temp2 != NULL){
                    temp = temp -> next;
                    temp2 = temp2 -> next;
                }
                temp -> next = msg;
            }
            pthread_mutex_unlock(&recvLock);

            pthread_mutex_lock(&msgSendLock);
            sendto(sockfd, "A", 1, 0 , src_addr, addrlen);
            sendto(sockfd, &ack, sizeof(int), 0 , src_addr, addrlen);
            pthread_mutex_unlock(&msgSendLock);
        }

    }
}
void *S(void *arg){
    int *id = (int *)arg;
    int sockfd = *id;
    struct timespec tim;
    tim.tv_sec = T;
    while (1){
        pthread_mutex_lock(&sendLock);
        for (int i = 0; i < TABLE_SIZE; i++){
            if (unacknowledged_message_table[i].acknowledgement != -1){
                time_t seconds = time(NULL);
                if (seconds - unacknowledged_message_table[i].time >= 2 * T){
                    unacknowledged_message_table[i].time = seconds;
                    int ack = unacknowledged_message_table[i].acknowledgement;
                    int len = unacknowledged_message_table[i].messageSize;
                    pthread_mutex_lock(&msgSendLock);
                    sendto(sockfd, "M", 1, 0, unacknowledged_message_table[i].dest_addr, unacknowledged_message_table[i].addrlen);
                    sendto(sockfd, (void *)&len, sizeof(int), 0, unacknowledged_message_table[i].dest_addr, unacknowledged_message_table[i].addrlen);
                    sendto(sockfd, (void *)&ack, sizeof(int), 0, unacknowledged_message_table[i].dest_addr, unacknowledged_message_table[i].addrlen);
                    sendto(sockfd, unacknowledged_message_table[i].message, unacknowledged_message_table[i].messageSize, 0, unacknowledged_message_table[i].dest_addr, unacknowledged_message_table[i].addrlen);
                    pthread_mutex_unlock(&msgSendLock);
                }

            }
        }
        pthread_mutex_unlock(&sendLock);
        nanosleep(&tim, NULL);
    }

}


int r_socket(int domain, int type, int protocol){
    received_message_table = NULL;
    initialiseTable();
    srand(time(0));
    pthread_mutex_init(&recvLock, NULL);
    pthread_mutex_init(&sendLock, NULL);
    pthread_mutex_init(&msgSendLock, NULL);
    if (type != SOCK_MRF){
        return -1;
    }
    return socket(domain, SOCK_DGRAM, protocol);
}
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    pthread_attr_init(&attr);
    int *arg = (int *)malloc(sizeof(*arg));
    *arg = sockfd;
    pthread_create(&ptids[0], &attr, R, arg);
    pthread_create(&ptids[1], &attr, S, arg);
    return bind(sockfd,addr,addrlen);
}
ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags,struct sockaddr *dest_addr, socklen_t addrlen){
    char *type = (char *)malloc(sizeof(char));
    *type = 'M';
    if (sendto(sockfd, type, 1,flags, dest_addr, addrlen ) < 0 ) return -1;
    int ack = -1;
    pthread_mutex_lock(&sendLock);
    for (int i = 0; i < TABLE_SIZE; i++){
        if ( unacknowledged_message_table[i].acknowledgement == -1) {
            unacknowledged_message_table[i].acknowledgement = i + 1;
            unacknowledged_message_table[i].messageSize = len;
            unacknowledged_message_table[i].dest_addr = dest_addr;
            unacknowledged_message_table[i].time = time(NULL);
            unacknowledged_message_table[i].addrlen = addrlen;
            strcpy( unacknowledged_message_table[i].message, (const char *)buf );
            ack = i + 1;
            break;
        }
    }
    pthread_mutex_unlock(&sendLock);
    if (sendto(sockfd, &len, sizeof(int),flags, dest_addr, addrlen ) < 0 ) return -1;
    if (sendto(sockfd, &ack, sizeof(int),flags, dest_addr, addrlen ) < 0 ) return -1;
    if (sendto(sockfd, buf, len,flags, dest_addr, addrlen ) < 0 ) return -1;
    return len;
}

ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen){
   struct timespec tim;
   tim.tv_sec = 1;
   size_t siz;
   while(1){
        if (received_message_table == NULL){
           nanosleep(&tim, NULL);
           continue;
        }
        messageReceived *temp = received_message_table;
        pthread_mutex_lock(&recvLock);
        for (int i = temp -> indexToBeRead, j = 0; i < min(temp -> indexToBeRead + len, temp ->messageSize ); i++, j++ ){
            ((char *)buf)[j]  = temp -> message[i];
        }
        siz = min(temp -> indexToBeRead + len, temp ->messageSize ) - temp -> indexToBeRead;
        temp ->indexToBeRead = min(temp -> indexToBeRead + len, temp ->messageSize );
        if (received_message_table ->indexToBeRead >= received_message_table -> messageSize ){
            received_message_table = received_message_table -> next;
            free(temp);
        }
        pthread_mutex_unlock(&recvLock);
        return siz;
   }
}
int r_close(int fd){
    while (received_message_table != NULL){
        messageReceived *temp = received_message_table;
        received_message_table = received_message_table -> next;
        free(temp);
    }
    pthread_join(ptids[0], NULL);
    pthread_join(ptids[1], NULL);
    return close(fd);;
}