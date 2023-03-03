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

#define IP_LEN 20
#define BUF_LEN 300
#define LARGE_BUF_LEN 1000
#define SMALL_BUF_LEN 10

int min(int a, int b){
    if (a < b) return a;
    return b;
}

char *readLine(char *cmdName)
{
    char *cmd = (char *)malloc(LARGE_BUF_LEN * sizeof(char));

    char ch = getchar();

    int pos = 0;

    while (!(ch == EOF || ch == '\n' || pos >= BUF_LEN - 1))
    {
        cmd[pos++] = ch;
        ch = getchar();
    }
    cmd[pos] = '\0';

    int id = 0;

    for (int i = 0; i < strlen(cmd); i++)
    {
        if ((cmd[i] != ' ' && cmd[i] != '\0' )  && id < SMALL_BUF_LEN)
        {
            cmdName[id++] = cmd[i];
        }
        else
        {
            break;
        }
    }

    cmdName[id] = '\0';

    return cmd;
}
int get(char *cmd, int sockfd){
    char *cmd_copy = (char *)malloc(LARGE_BUF_LEN * sizeof(char));
    char *localFile = (char *)malloc(BUF_LEN * sizeof(char));

    strcpy(cmd_copy, cmd);

    localFile = strtok(cmd_copy, " ");
    localFile = strtok(NULL, " ");
    localFile = strtok(NULL, " ");

    int fd = open(localFile, O_CREAT | O_WRONLY ,0666);
    if ( (localFile[0] == '.') && ((localFile[1] == '/') || (localFile[1] == '.'))){
        printf("Relative FileName is not allowed (Relative path can't be used. ).\n");
        return -1;
    }
    if (fd < 0)
    {
        printf("ERROR. Enter correct local file name.\n");
        return -1;
    }
    else
    {
        printf("Sending command: %s\n", cmd);
        send(sockfd, cmd, strlen(cmd) + 1, 0);

        char *status = (char *)malloc(sizeof(char) * 3);
        char *buf = (char *)malloc(sizeof(char) * LARGE_BUF_LEN);
        for (int j = 0; j < LARGE_BUF_LEN; j++){
            buf[j] = '\0';
        }
        int len, id = 0, i;
        int maxLim = 4;
        while (maxLim > 0){
            len = recv(sockfd, buf, maxLim, 0);
            if (len < 0){
                printf("Error in receiving signal Code\n");
                return -1;
            }
            for (int i = 0; i < len; i++){
                status[id++] = buf[i];
                buf[i] = '\0';
            }
            maxLim-=len;
        }
        id = 0;
        if (strcmp(status,"500") == 0)
        {
            printf("Error executing command: Error Code 500\n");
            return 500;
        }
        else
        {
            printf("Remote File present in the server. Getting contents.\n");

            char data[LARGE_BUF_LEN] = {0};
            char flag;
            uint16_t num;
            int last = 0;
            while(1){
                char *numChar = (char *)&num;
                int r = recv(sockfd, &flag, 1, 0);
                if (r < 0){
                    printf("Error in receiving header\n");
                    return -1;
                }
                if (flag == 'L'){
                    last = 1;
                }
                r = recv(sockfd, numChar, 1, 0);
                if (r < 0){
                    printf("Error in receiving header\n");
                    return -1;
                }
                r = recv(sockfd, numChar + 1, 1, 0);
                if (r < 0){
                    printf("Error in receiving header\n");
                    return -1;
                }
                int len = ntohs(num);
                //printf("%d\n", len);
                while (len > 0){
                    r = recv(sockfd, data,min(LARGE_BUF_LEN,len), 0);
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
            printf("Command executed Successfully\n\n");
        }
    }
    close(fd);
    return 200;
}

int put(char *cmd, int sockfd){
    char *cmd_copy = (char *)malloc(LARGE_BUF_LEN * sizeof(char));
    char *status = (char *)malloc(sizeof(char) * 3);
    char *localFile = (char *)malloc(BUF_LEN * sizeof(char));
    char *buf = (char *)malloc(BUF_LEN * sizeof(char));

    strcpy(cmd_copy, cmd);

    localFile = strtok(cmd_copy, " ");
    localFile = strtok(NULL, " ");
    if ( (localFile[0] == '.') && ((localFile[1] == '/') || (localFile[1] == '.'))){
        printf("Relative FileName is not allowed.\n");
        return -1;
    }
    int fd = open(localFile, O_RDONLY);

    if (fd < 0)
    {
        printf("ERROR. Enter correct local file name. %s doesn't exist\n", localFile);
        return -1;
    }
    printf("Sending command: %s\n", cmd);
    send(sockfd, cmd, strlen(cmd) + 1, 0);

    int len, id = 0, i;
    int maxLim = 4;
    while (maxLim > 0){
        len = recv(sockfd, buf, maxLim, 0);
        if (len < 0){
            printf("Error in receiving signal Code\n");
            return -1;
        }
        for (int i = 0; i < len; i++){
            status[id++] = buf[i];
            buf[i] = '\0';
        }
        maxLim-=len;
    }
    if (strcmp(status,"500") == 0)
    {
        printf("Error executing command. Error Code: 500\n");
        return 500;
    }
    //printf("status: %s\n", status);
    char *data = (char *)malloc(sizeof(char) * LARGE_BUF_LEN);
    char *temp = (char *)malloc(sizeof(char) * LARGE_BUF_LEN);
    int first = 1, prevlen = 0;
    char flag = 'M';
    while(1){
        int r = read(fd, temp, LARGE_BUF_LEN);
        if (r < 0){
            printf("Error in reading data from file\n");
            return -1;
        }
        if (r == 0) flag = 'L';
        else flag = 'M';
        if (first == 0){
            //printf("prelen = %d\n%s", prevlen, data);
            uint16_t len = htons(prevlen);
            send(sockfd,&flag, 1, 0);
            send(sockfd, (void *)&len, 2, 0);
            send(sockfd, data, prevlen, 0);
        }
        first = 0;
        for (int i = 0; i < LARGE_BUF_LEN; i++){
            data[i] = temp[i];
            temp[i] = '\0';
        }
        prevlen = r;
        if (r == 0) break;
    }
    printf("Command Executed Successfully\n");
    return 200;
    
}

int main()
{
    int sockfd;                   /* Socket descriptors */
    struct sockaddr_in serv_addr; /*Server address*/
    int connected = 0, isLogin = 0;

    // Create a socket. Check if there is any error too.
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }
// Testing Part
    // serv_addr.sin_family = AF_INET;
    // inet_aton("127.0.0.1", &serv_addr.sin_addr);
    // serv_addr.sin_port = htons(20000);

    // int ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
// Testing part
    while (1)
    {
        printf("myFTP> ");
        char *cmd = (char *)malloc(LARGE_BUF_LEN * sizeof(char));
        char cmdName[SMALL_BUF_LEN];

        cmd = readLine(cmdName);
        //printf("%s %d", cmdName, (int)strlen(cmdName));
        
        if (isLogin == 0) // if the client is not connected to server
        {
            // if the command name is open, seperate ip address and port number and try to connect it to the server
            if (connected == 0){
                if (strcmp(cmdName, "open") == 0)
                {
                    char *ip_address = (char *)malloc(IP_LEN * sizeof(char));
                    char *portStr = (char *)malloc(SMALL_BUF_LEN * sizeof(char));

                    int port;

                    ip_address = strtok(cmd, " ");
                    ip_address = strtok(NULL, " ");
                    if (ip_address == NULL){
                        printf("Invalid Command\n");
                        continue;
                    }
                    portStr = strtok(NULL, " ");
                    if (portStr == NULL){
                        printf("Invalid Command\n");
                        continue;
                    }
                    port = atoi(portStr);
                    
                    // printf("IP Address: %s PORT: %s %d\n",ip_address,portStr,port);

                    if (port < 20000 || port > 65535)
                    {
                        printf("ERROR. Enter valid port between [20000,65535] \n");
                        continue;
                    }

                    serv_addr.sin_family = AF_INET;
                    inet_aton(ip_address, &serv_addr.sin_addr);
                    serv_addr.sin_port = htons(port);

                    int ret =
                        connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

                    if (ret >= 0)
                    {
                        printf("Connected to the server successfully.\n");
                        connected = 1;
                    }
                    else
                    {
                        printf("ERROR. Could not connect to server. Try again.\n");
                    }
                }
                else{
                    printf("Connect to the server first.\n");
                }
            }
            else if (strcmp(cmdName, "user") == 0){
                //printf("Sending command: %s\n", cmd);
                send(sockfd, cmd, strlen(cmd) + 1, 0);

                char *status = (char *)malloc(sizeof(char) * BUF_LEN);
                char *buf = (char *)malloc(sizeof(char) * BUF_LEN);
                for (int i = 0; i < BUF_LEN; i++){
                    status[i] = '\0';
                    buf[i] = '\0';
                }
                int len, id = 0;
                while((len = recv(sockfd, buf, (size_t)BUF_LEN, 0) ) > 0){
                    int end = 0;
                    for (int i = 0; i < len; i++){
                        status[id++] = buf[i];
                        if (buf[i] == '\0') {
                            end = 1;
                            break;

                        }
                    }
                    for (int i = 0; i < BUF_LEN; i++){
                        buf[i] = '\0';
                    }
                    if (end == 1) break;
                }
                //printf("YES\n");
                //printf("%s\n",status);
                if (strcmp(status, "200") == 0)
                {
                    printf("Command Executed Successfully\n\n");
                }
                else if (strcmp(status, "500") == 0)
                {
                    printf("Error executing command. Error Code: 500\n\n");
                }
                else
                {
                    printf("Error executing command. Error Code: 600\n\n");
                }
            }
            else if (strcmp(cmdName, "pass") == 0){
                //printf("Sending command: %s\n", cmd);
                send(sockfd, cmd, strlen(cmd) + 1, 0);

                char *status = (char *)malloc(sizeof(char) * BUF_LEN);
                char *buf = (char *)malloc(sizeof(char) * BUF_LEN);
                int len, id = 0;
                while((len = recv(sockfd, buf, BUF_LEN, 0) ) > 0){
                    int end = 0;
                    for (int i = 0; i < len; i++){
                        status[id++] = buf[i];
                        if (buf[i] == '\0') {
                            end = 1;
                            break;

                        }
                    }
                    if (end == 1) break;
                }

                if (strcmp(status, "200") == 0)
                {
                    printf("Command Executed Successfully.\n\n");
                    isLogin = 1;
                }
                else if (strcmp(status, "500") == 0)
                {
                    printf("Error executing command. Error Code: 500\n\n");
                }
                else
                {
                    printf("Error executing command. Error Code: 600\n\n");
                }
            }
            else if(strcmp(cmdName, "open") == 0){
                printf("Already Connected\n");
            }
            else // if the first command is not "open"
            {
                printf("ERROR. First connect to the server and log in.\n");
            }
        }
        else // if the client is connected to the server
        {
            if (strcmp(cmdName, "quit") == 0){
                close(sockfd);
                return 1;
            }
            if (strcmp(cmdName, "open") == 0){ // trying to use open command
                printf("ERROR. Already Connected. \n");
            }
            else if (strcmp(cmdName, "user") == 0) // if the command name is user , pass the command, wait for the status and print appropriate msg
            {
                printf("User is already logged in \n\n");
            }
            else if (strcmp(cmdName, "pass") == 0)
            {
                printf("User is already logged in \n\n");
            }
            else if (strcmp(cmdName, "cd") == 0)
            {
                //printf("Sending command: %s\n", cmd);
                send(sockfd, cmd, strlen(cmd) + 1, 0);

                char *status = (char *)malloc(sizeof(char) * BUF_LEN);
                char *buf = (char *)malloc(sizeof(char) * BUF_LEN);
                int len, id = 0;
                while((len = recv(sockfd, buf, BUF_LEN, 0) ) > 0){
                    int end = 0;
                    for (int i = 0; i < len; i++){
                        status[id++] = buf[i];
                        if (buf[i] == '\0') {
                            end = 1;
                            break;

                        }
                    }
                    if (end == 1) break;
                }

                if (strcmp(status,"200") == 0)
                {
                    printf("Command successfully.\n\n");
                }
                else
                {
                    printf("Error executing command. Error Code: 500\n\n");
                }
            }
            else if (strcmp(cmdName, "lcd") == 0)
            {
                char *dirName = (char *)malloc(BUF_LEN * sizeof(char));

                dirName = strtok(cmd, " ");
                dirName = strtok(NULL, " ");

                // printf("Dir Name: %s\n",dirName);

                if (chdir(dirName) == 0)
                {
                    printf("Successfully changed directly.\n\n");
                }
                else
                {
                    printf("Some ERROR occurred. Could not change directory. Check path again.\n\n");
                }
            }
            else if (strcmp(cmdName, "dir") == 0)
            {

                send(sockfd, cmd, strlen(cmd) + 1, 0);

                printf("\nFiles in directory are: \n");

                char dirName[BUF_LEN];

                for (int i = 0; i < BUF_LEN; i++)
                {
                    dirName[i] = '\0';
                }

                int prevIsNul = 1, done = 0, recv_len;

                while ((recv_len = recv(sockfd, dirName, sizeof(dirName), 0)) > 0)
                {
                    // printf("%s\n",dirName);
                    for (int i = 0; i < recv_len; i++)
                    {
                        if (dirName[i] == '\0')
                        {
                            if (prevIsNul)
                            {
                                done = 1;
                                break;
                            }
                            printf("\n");
                            prevIsNul = 1;
                        }
                        else
                        {
                            prevIsNul = 0;
                            printf("%c", dirName[i]);
                        }
                    }
                    if (done)
                    {
                        break;
                    }
                    for (int i = 0; i < BUF_LEN; i++)
                    {
                        dirName[i] = '\0';
                    }
                }
                printf("\n");
            }
            else if (strcmp(cmdName, "get") == 0)
            {
                int res = get(cmd, sockfd);
            }
            else if (strcmp(cmdName, "put" ) == 0){
                int res = put(cmd, sockfd);
            }
            else if (strcmp(cmdName,"mget") == 0){
                char *cmd_copy = (char *)malloc(LARGE_BUF_LEN * sizeof(char));
                char *localFile = (char *)malloc(BUF_LEN * sizeof(char));

                strcpy(cmd_copy, cmd);
                localFile = strtok(cmd_copy, " ");
                int count = 0;
                while ( (localFile = strtok(NULL, " ") ) != NULL){
                    count++;
                }
                char **cmds = (char **)malloc(sizeof(char *) * count);
                strcpy(cmd_copy, cmd);
                localFile = strtok(cmd_copy, " ");
                int ind = 0;
                while ( (localFile = strtok(NULL, " ") ) != NULL){
                    cmds[ind] = (char *)malloc(LARGE_BUF_LEN * sizeof(char));
                    cmds[ind][0] = 'g';
                    cmds[ind][1] = 'e';
                    cmds[ind][2] = 't';
                    cmds[ind][3] = ' ';
                    int j = 4;
                    for (int i = 0; i < (int)strlen(localFile); i++, j++){
                        cmds[ind][j]  = localFile[i];
                    }
                    cmds[ind][j++] = ' ';
                    for (int i = 0; i < (int)strlen(localFile); i++, j++){
                        cmds[ind][j]  = localFile[i];
                    }
                    cmds[ind][j] = '\0';
                    ind++;
                }
                for (int i = 0; i < count; i++){
                    int res = get(cmds[i], sockfd);
                    if (res != 200){
                        break;
                    }
                }
            }
            else if (strcmp(cmdName,"mput") == 0){
                char *cmd_copy = (char *)malloc(LARGE_BUF_LEN * sizeof(char));
                char *localFile = (char *)malloc(BUF_LEN * sizeof(char));

                strcpy(cmd_copy, cmd);
                localFile = strtok(cmd_copy, " ");
                int count = 0;
                while ( (localFile = strtok(NULL, " ") ) != NULL){
                    count++;
                }
                char **cmds = (char **)malloc(sizeof(char *) * count);
                strcpy(cmd_copy, cmd);
                localFile = strtok(cmd_copy, " ");
                int ind = 0;
                while ( (localFile = strtok(NULL, " ") ) != NULL){
                    cmds[ind] = (char *)malloc(LARGE_BUF_LEN * sizeof(char));
                    cmds[ind][0] = 'p';
                    cmds[ind][1] = 'u';
                    cmds[ind][2] = 't';
                    cmds[ind][3] = ' ';
                    int j = 4;
                    for (int i = 0; i < (int)strlen(localFile); i++, j++){
                        cmds[ind][j]  = localFile[i];
                    }
                    cmds[ind][j++] = ' ';
                    for (int i = 0; i < (int)strlen(localFile); i++, j++){
                        cmds[ind][j]  = localFile[i];
                    }
                    cmds[ind][j] = '\0';
                    ind++;
                }
                for (int i = 0; i < count; i++){
                    int res = put(cmds[i], sockfd);
                    if (res != 200){
                        break;
                    }
                }
            }
            else
            {
                printf("Enter valid command.\n");
            }
        }
    }

    return 0;
}
// /home/rupinderg00/Documents/OS/ass2
