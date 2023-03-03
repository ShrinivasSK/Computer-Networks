/*
NETWORKS LAB ASSIGNMENT 3 : Building FTP Server-Client
FTP Client Code
Shrinivas Khiste 19CS30043
Rupinder Goyal 19CS10050
*/

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include<dirent.h>

#define BUF_LEN 300
#define LARGE_BUF_LEN 1000
#define NUM_USERS 2

int min(int a, int b){
    if (a < b) return a;
    return b;
}

char* recvFromClient(int sockfd, int* status, char* cmdName) {
    char* recVal = (char*)malloc(LARGE_BUF_LEN * sizeof(char));

    char buf[BUF_LEN + 1];

    for (int i = 0;i <= BUF_LEN;i++) {
        buf[i] = '\0';
    }

    int id = 0, done = 0;

    int ret_val;

    while ((ret_val = recv(sockfd, buf, BUF_LEN, 0)) > 0) {
        for (int i = 0;i < ret_val;i++) {
            recVal[id++] = buf[i];
            if (buf[i] == '\0') {
                done = 1;
                break;
            }
        }
        if (done) {
            break;
        }
        for (int i = 0;i <= BUF_LEN;i++) {
            buf[i] = '\0';
        }
    }
    if (ret_val < 0) {
        *status = -1;
    }
    else if (ret_val == 0){
        *status = 0;
    }
    else {
        *status = 1;
    }
    //recVal[id + 1] = '\0';

    id = 0;
    for (int i = 0;i < strlen(recVal);i++) {
        if (recVal[i] != ' ')
            cmdName[id++] = recVal[i];
        else
            break;
    }
    cmdName[id] = '\0';
    return recVal;
}

char** readLoginCreds() {
    char** loginCred = (char**)malloc((NUM_USERS * 2) * sizeof(char*));

    for (int i = 0;i < NUM_USERS * 2;i++) {
        loginCred[i] = (char*)malloc(BUF_LEN * sizeof(char));
    }

    int fd = open("user.txt", O_RDONLY);

    if (fd < 0) {
        printf("Could not find User Credentials file \"user.txt\".Exitting\n");
        exit(0);
    }

    char buf[BUF_LEN];
    char line[LARGE_BUF_LEN];

    int id = 0, userId = 1;

    for (int i = 0;i <= BUF_LEN;i++) {
        buf[i] = '\0';
    }

    while (read(fd, buf, BUF_LEN) > 0) {
        // printf("%s\n", buf);
        for (int i = 0;i < strlen(buf);i++) {
            if (buf[i] != '\n') {
                line[id++] = buf[i];
            }
            else {
                // printf("yo %s\n",loginCred[userId]);
                strcpy(loginCred[userId - 1], strtok(line, " "));
                strcpy(loginCred[userId], strtok(NULL, " "));
                userId += 2;
                id = 0;
                if (userId > NUM_USERS * 2) {
                    break;
                }
            }
        }
        if (userId > NUM_USERS * 2) {
            break;
        }
        for (int i = 0;i <= BUF_LEN;i++) {
            buf[i] = '\0';
        }
    }

    // for (int i = 0;i < NUM_USERS * 2;i++) {
    //     printf("%s\n", loginCred[i]);
    // }

    return loginCred;
}

int main() {
    int	sockfd, newsockfd; /* Socket descriptors */
    int	clilen;
    struct sockaddr_in	cli_addr, serv_addr;

    // Create a socket. Check if there is any error too. 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error Occurred. Could not create socket.\n");
        exit(0);
    }
    printf("Created a socket to connect  to client.\n");

    /* Assign the properties of the server. */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    /* Associate the server with its port using the bind() system call. */
    if (bind(sockfd, (struct sockaddr*)&serv_addr,
        sizeof(serv_addr)) < 0) {
        perror("Unable to bind local address\n");
        exit(0);
    }

    printf("Listening for client request....\n");
    listen(sockfd, 5);

    int login = -1, userId = -1;

    char** loginCreds = readLoginCreds();

    // if(chdir("")){
    //     printf("yo1\n");
    // }else{
    //     printf("no\n");
    // }

    while (1) {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr,(socklen_t*)&clilen);


        if (newsockfd < 0) {
            perror("Accept error\n");
            exit(0);
        }

        printf("Successfully connected to the client.\n\n");
        int pid = fork();
        if (pid == 0){
            close(sockfd);
            login = -1;
            userId = -1;
            
            while (1) {
                int isValid = 0;
                char cmdName[BUF_LEN];
                char status[BUF_LEN];
                status[0] = '2';
                status[1] = '0';
                status[2] = '0';
                status[3] = '\0';
                char* cmd = recvFromClient(newsockfd, &isValid, cmdName);
                if (isValid < 0) {
                    printf("ERROR in receiving data. Exitting.\n");
                    break;
                }
                if (isValid == 0){
                    printf("Connection Closed. Bbye\n");
                    close(newsockfd);
                    return 0;
                }

                //printf("Command Received: %s\n", cmd);

                if (login == -1) {
                    if (strcmp(cmdName, "user") == 0) {
                        char* username = (char*)malloc(BUF_LEN * sizeof(char));

                        username = strtok(cmd, " ");
                        username = strtok(NULL, " ");

                        if (username != NULL) {
                            for (int i = 0;i < NUM_USERS * 2;i += 2) {
                                // printf("%s %s\n",username,loginCreds[i]);
                                if (strcmp(username, loginCreds[i]) == 0) {
                                    userId = i / 2;
                                }
                            }
                            if (userId == -1) {
                                //printf("Wrong username. Sending status 500. \n");
                                status[0] = '5';
                            }
                            else {
                                //printf("Correct username. Sending status 200.\n");
                                status[0] = '2';
                            }
                        }
                        else {
                            //printf("Invalid Command. Sending status 600.\n");
                            status[0] = '6';
                        }
                    }
                    else if ((userId != -1) && (strcmp(cmdName, "pass") == 0)) {
                        char* password = (char*)malloc(BUF_LEN * sizeof(char));

                        password = strtok(cmd, " ");
                        password = strtok(NULL, " ");

                        if (password != NULL) {
                            //printf("%s %s\n", password, loginCreds[userId * 2 + 1]);
                            if (strcmp(loginCreds[userId * 2 + 1], password) == 0) {
                                //printf("Correct Password. Sending status 200. Logged in Successfully.\n");
                                login = 1;
                                status[0] = '2';
                            }
                            else {
                                //printf("Wrong Password. Sending status 500.\n");
                                userId = -1;
                                status[0] = '5';
                            }
                        }
                        else {
                            //printf("Invalid Command. Sending status 600.\n");
                            userId = -1;
                            status[0] = '6';
                        }
                    }
                    else {
                        //printf("Invalid Command. Sending status 600.\n");
                        status[0] = '6';
                    }
                    send(newsockfd, status, strlen(status) + 1, 0);

                }
                else if (strcmp(cmdName, "cd") == 0) {
                    char* dirName = (char*)malloc(BUF_LEN * sizeof(char));

                    dirName = strtok(cmd, " ");
                    dirName = strtok(NULL, " ");

                    int len = strlen(dirName);
                    //dirName[len] = '\n';
                    dirName[len] = '\0';

                    // printf("Dir Name: %s\n", dirName);

                    if (chdir(dirName) == 0) {
                        status[0] = '2';
                    }
                    else {
                        status[0] = '5';
                    }
                    send(newsockfd, status, strlen(status) + 1, 0);
                    //printf("%s\n", getcwd(dirName,BUF_LEN));

                }
                else if (strcmp(cmdName, "dir") == 0) {
                    DIR* d;
                    struct dirent* dir;
                    char *direc = (char *)malloc(sizeof(char) * BUF_LEN);
                    getcwd(direc,BUF_LEN);
                    d = opendir(direc);
                    char buf[BUF_LEN];
                    for (int i = 0;i < BUF_LEN;i++) {
                        buf[i] = '\0';
                    }
                    if (d) {
                        while ((dir = readdir(d)) != NULL) {
                            strcpy(buf, dir->d_name);
                            int len = strlen(dir->d_name);
                            buf[len] = '\0';
                            // printf("%d %ld\n", len,strlen(buf));
                            send(newsockfd, buf, strlen(buf)+1, 0);
                            for (int i = 0;i < BUF_LEN;i++) {
                                buf[i] = '\0';
                            }
                        }
                        send(newsockfd, "\0", 1, 0);
                        closedir(d);
                    }
                    else {
                        send(newsockfd, "\0", 1, 0);
                        //printf("Could not list files of directory.\n");
                    }
                }
                else if (strcmp(cmdName, "get") == 0){
                    char *localFile = (char *)malloc(sizeof(char) * BUF_LEN);
                    char *cmdcopy = (char *)malloc(sizeof(char) * LARGE_BUF_LEN);
                    strcpy(cmdcopy, cmd);
                    localFile = strtok(cmdcopy," ");
                    localFile = strtok(NULL," ");
                    if ( (localFile[0] == '.') && ((localFile[1] == '/' ) || (localFile[1] == '.'))){
                        send(newsockfd, "500\0", 4, 0);
                        continue;
                    }
                    int fd = open(localFile, O_RDONLY);
                    //printf("fd = %d\n", fd);
                    if (fd < 0){
                        send(newsockfd, "500\0", 4, 0);
                    }
                    else{
                        send(newsockfd, "200\0", 4, 0);
                        char *data = (char *)malloc(sizeof(char) * LARGE_BUF_LEN);
                        char *temp = (char *)malloc(sizeof(char) * LARGE_BUF_LEN);
                        int first = 1, prevlen = 0;
                        char flag = 'M';
                        while(1){
                            int r = read(fd, temp, LARGE_BUF_LEN);
                            //printf("%s\n", temp);
                            if (r < 0){
                                printf("Error in reading data from file\n");
                                return -1;
                            }
                            if (r == 0) flag = 'L';
                            else flag = 'M';
                            if (first == 0){
                                //printf("YESsss\n");
                                uint16_t len = htons(prevlen);
                                //printf("YESsss\n");
                                send(newsockfd,&flag, 1, 0);
                                //printf("YESsss\n");
                                send(newsockfd, (void *)&len, 2, 0);
                                //printf("YESsss\n");
                                send(newsockfd, data, prevlen, 0);
                                //printf("YESsss\n");
                            }
                            first = 0;
                            for (int i = 0; i < LARGE_BUF_LEN; i++){
                                data[i] = temp[i];
                                temp[i] = '\0';
                            }
                            //printf("YES\n");
                            prevlen = r;
                            if (r == 0) break;
                        }
                    }
                }
                else if (strcmp(cmdName, "put") == 0){
                    char *localFile = (char *)malloc(sizeof(char) * BUF_LEN);
                    char *cmdcopy = (char *)malloc(sizeof(char) * LARGE_BUF_LEN);
                    strcpy(cmdcopy, cmd);
                    localFile = strtok(cmdcopy," ");
                    localFile = strtok(NULL," ");
                    localFile = strtok(NULL," ");
                    if ( (localFile[0] == '.') && ((localFile[1] == '/') || (localFile[1] == '.'))){
                        send(newsockfd, "500\0", 4, 0);
                        continue;
                    }
                    int fd = open(localFile, O_WRONLY | O_CREAT , 0666);
                    if (fd < 0){
                        send(newsockfd, "500\0", 4, 0);
                    }
                    else{
                        send(newsockfd, "200\0", 4, 0);
                        char data[LARGE_BUF_LEN] = {0};
                        char flag;
                        uint16_t num;
                        int last = 0;
                        while(1){
                            char *numChar = (char *)&num;
                            int r = recv(newsockfd, &flag, 1, 0);
                            if (r < 0){
                                printf("Error in receiving header\n");
                                return -1;
                            }
                            if (flag == 'L'){
                                last = 1;
                            }
                            r = recv(newsockfd, numChar, 1, 0);
                            if (r < 0){
                                printf("Error in receiving header\n");
                                return -1;
                            }
                            r = recv(newsockfd, numChar + 1, 1, 0);
                            if (r < 0){
                                printf("Error in receiving header\n");
                                return -1;
                            }
                            int len = ntohs(num);
                            //printf("%d\n", len);
                            while (len > 0){
                                r = recv(newsockfd, data,min(LARGE_BUF_LEN,len), 0);
                                if (r < 0) {
                                    printf("Error in receiving Data\n");
                                    return -1;
                                }
                                write(fd, data,r);
                                len-=r;
                                for (int i = 0; i < LARGE_BUF_LEN; i++) data[i] = '\0';
                            }
                            if (last) break;
                        }
                    }
                    close(fd);
                }
                else {
                    send(newsockfd, "500\0", 4, 0);
                }

                //printf("\n");
            }
            close(newsockfd);
            return 0;
        }
        close(newsockfd);

    }
    return 0;
}
