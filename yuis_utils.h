#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BYTE_SIZE 256
#define YUIS_MALLOC malloc
#define YUIS_DEFAULT_FREE
char *buffer_push_char(char *buffer, char ch);
char **split_into(const char *str, char delim);
char *read_file(const char *filename);

#ifdef YUIS_UTILS

char *read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening file");
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    size_t fsz = ftell(f);
    rewind(f);
    
    char *buffer = YUIS_MALLOC(fsz + 1);
    if (!buffer) {
        fclose(f);
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    size_t r = fread(buffer, 1, fsz, f);
    fclose(f);
    
    if (r != fsz) {
#ifndef YUIS_DEFAULT_FREE
        free(buffer);
#endif
        fprintf(stderr, "Error reading file\n"); 
        return NULL;
    }
    
    buffer[fsz] = '\0';
    return buffer;
}

char *buffer_push_char(char *buffer, char ch) {
    size_t len = strlen(buffer);
    char *new_buffer = realloc(buffer, len + 2);
    if (!new_buffer) {
        fprintf(stderr, "Memory reallocation failed\n");
        return buffer;
    }
    
    new_buffer[len] = ch;
    new_buffer[len + 1] = '\0';
    return new_buffer;
}

char **split_into(const char *str, char delim) {
    size_t cap = 10, count = 0;
    char **result = YUIS_MALLOC(cap * sizeof(char *));
    if (!result) return NULL;
    
    char *buffer = YUIS_MALLOC(BYTE_SIZE);
    if (!buffer) {
#ifndef YUIS_DEFAULT_FREE
        free(result);
#endif
        return NULL;
    }
    
    size_t idx_str = 0, idx_buf = 0;
    while (1) {
        if (str[idx_str] == delim || str[idx_str] == '\0') {
            buffer[idx_buf] = '\0';
            result[count++] = strdup(buffer);
            if (!result[count - 1]) {
                fprintf(stderr, "Memory allocation failed\n");
#ifndef YUIS_DEFAULT_FREE
                free(buffer);
                for (size_t i = 0; i < count - 1; i++) free(result[i]);
                free(result);
#endif
                return NULL;
            }
            idx_buf = 0;
            if (str[idx_str] == '\0') break;
            
            if (count >= cap) {
                cap *= 2;
                char **temp = realloc(result, cap * sizeof(char *));
                if (!temp) {
                    fprintf(stderr, "Memory reallocation failed\n");
#ifndef YUIS_DEFAULT_FREE
                    for (size_t i = 0; i < count; i++) free(result[i]);
                    free(result);
#endif
                    return NULL;
                }
                result = temp;
            }
        } else {
            buffer[idx_buf++] = str[idx_str];
        }
        idx_str++;
    }
#ifndef YUIS_DEFAULT_FREE
    free(buffer);
#endif
    return result;
}

#endif /* ifdef YUIS_UTILS */
