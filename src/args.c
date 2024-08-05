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

#include <utils/args.h>

char **split(const char *str, char delimiter, int *num_tokens) {
    char **tokens = NULL;
    char *token;
    char *str_copy;
    int count = 0;

    str_copy = strdup(str);
    if (str_copy == NULL) {
        perror("strdup failed");
        return NULL;
    }

    token = strtok(str_copy, &delimiter);
    while (token != NULL) {
        count++;
        token = strtok(NULL, &delimiter);
    }

    tokens = malloc(count * sizeof(char *));
    if (tokens == NULL) {
        perror("malloc failed");
        free(str_copy);
        return NULL;
    }

    strcpy(str_copy, str);

    token = strtok(str_copy, &delimiter);
    for (int i = 0; i < count; i++) {
        tokens[i] = strdup(token);
        if (tokens[i] == NULL) {
            perror("strdup failed");
            for (int j = 0; j < i; j++) {
                free(tokens[j]);
            }
            free(tokens);
            free(str_copy);
            return NULL;
        }
        token = strtok(NULL, &delimiter);
    }

    free(str_copy);
    *num_tokens = count;
    return tokens;
}

void free_split(char **tokens, int num_tokens) {
    for (int i = 0; i < num_tokens; i++) {
        free(tokens[i]);
    }
    free(tokens);
}