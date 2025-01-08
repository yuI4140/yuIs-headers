#pragma once
#define BYTE_SIZE 256
char *buffer_push_char(char *buffer, char ch);
char **split_into(const char *str, char delim);
char *read_file(const char *filename);
#ifdef YUIS_HASHMAP
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define HASHMAP_ALLOC malloc
#define HASHMAP_REALLOC realloc
#define HASHMAP_FREE free
#define HASHMAP_GET(type, hashmap, key) ((type)(uintptr_t)hashmap_get(hashmap, key))
typedef enum SlotState {
    SLOT_EMPTY,
    SLOT_OCCUPIED,
    SLOT_DELETED
} SlotState;
typedef struct Hash {
    char *key;
    void *value;
    SlotState state;
} Hash;
typedef struct HashMap {
    uint64_t count;
    uint64_t cap;
    Hash *slots; 
} HashMap;

HashMap hashmap_init(uint64_t cap);
int hashmap_insert(HashMap *hashmap, const char *key, void *value);
uint64_t hash_function(const char *key, uint64_t cap);
void hashmap_free(HashMap *hashmap);
void *hashmap_get(HashMap *hashmap, const char *key);
int hashmap_remove(HashMap *hashmap, const char *key);
#endif // YUIS_HASHMAP
#ifdef YUIS_UTILS
#include <string.h>
#include <stdlib.h>
char *read_file(const char *filename){
    FILE *f=fopen(filename,"rb");
    fseek(f,0,SEEK_END);
    size_t fsz=ftell(f);
    fseek(f,0,SEEK_SET);
    char *buffer=malloc(fsz + 1);
    int r=fread(buffer,1,fsz,f);
    if (r==0) {
       fprintf(stderr, "Error reading file\n"); 
       return NULL;
    }
    return buffer;
}
char *buffer_push_char(char *buffer, char ch) {
    size_t idx = strlen(buffer);
    if (ch == '\0') {
        buffer[0] = ch;
    } else {
        buffer[idx] = ch;
        buffer[idx + 1] = '\0';
    }
    return buffer;
}

// Function to split a string into an array of strings based on a delimiter
char **split_into(const char *str, char delim) {
    char *buffer = malloc(BYTE_SIZE);
    char **result = malloc(BYTE_SIZE * 4096 * sizeof(char *));
    size_t idx_str = 0, idx_res = 0;
    while (1) {
        if (str[idx_str] == '\0') {
            result[idx_res] = strdup(buffer);
            break;
        } else if (str[idx_str] == delim) {
            result[idx_res++] = strdup(buffer);
            buffer_push_char(buffer, '\0');
            idx_str++;
        } else {
            buffer_push_char(buffer, str[idx_str++]);
        }
    }
    return result;
}
#ifdef YUIS_HASHMAP
uint64_t hash_function(const char *key, uint64_t cap) {
    uint64_t hash = 14695981039346656037ULL;
    while (*key) {
        hash ^= (unsigned char)(*key++);
        hash *= 1099511628211ULL;
    }
    return hash % cap;
}


HashMap hashmap_init(uint64_t cap) {
    if (cap == 0) {
        cap = 256;
    }

    HashMap hashmap = (HashMap){0};
    hashmap.cap = cap;
    hashmap.count = 0;
    hashmap.slots = HASHMAP_ALLOC(sizeof(Hash) * cap);

    if (hashmap.slots) {
        for (uint64_t i = 0; i < cap; i++) {
            hashmap.slots[i].state = SLOT_EMPTY;
        }
    }
    return hashmap;
}


void hashmap_free(HashMap *hashmap) {
    for (uint64_t i = 0; i < hashmap->cap; i++) {
        if (hashmap->slots[i].state == SLOT_OCCUPIED) {
            HASHMAP_FREE(hashmap->slots[i].key);
        }
    }
    HASHMAP_FREE(hashmap->slots);
    hashmap->slots = NULL;
    hashmap->cap = 0;
    hashmap->count = 0;
}


void hashmap_resize(HashMap *hashmap) {
    uint64_t old_cap = hashmap->cap;
    Hash *old_slots = hashmap->slots;

    hashmap->cap *= 2;
    hashmap->count = 0;
    hashmap->slots = HASHMAP_ALLOC(sizeof(Hash) * hashmap->cap);

    if (!hashmap->slots) return;

    for (uint64_t i = 0; i < hashmap->cap; i++) {
        hashmap->slots[i].state = SLOT_EMPTY;
    }

    for (uint64_t i = 0; i < old_cap; i++) {
        if (old_slots[i].state == SLOT_OCCUPIED) {
            hashmap_insert(hashmap, old_slots[i].key, old_slots[i].value);
            HASHMAP_FREE(old_slots[i].key);
        }
    }
    HASHMAP_FREE(old_slots);
}

int hashmap_insert(HashMap *hashmap, const char *key, void *value) {
    if (hashmap->count >= hashmap->cap * 0.9) {
        hashmap_resize(hashmap);
    }

    uint64_t index = hash_function(key, hashmap->cap);
    uint64_t start_index = index;

    while (hashmap->slots[index].state == SLOT_OCCUPIED) {
        if (strcmp(hashmap->slots[index].key, key) == 0) {
            hashmap->slots[index].value = value; 
            return 0;
        }
        index = (index + 1) % hashmap->cap;
        if (index == start_index) return -1; 
    }

    hashmap->slots[index].key = strdup(key);
    hashmap->slots[index].value = value;
    hashmap->slots[index].state = SLOT_OCCUPIED;
    hashmap->count++;
    return 0;
}

void *hashmap_get(HashMap *hashmap, const char *key) {
    uint64_t index = hash_function(key, hashmap->cap);
    uint64_t start_index = index;

    while (hashmap->slots[index].state != SLOT_EMPTY) {
        if (hashmap->slots[index].state == SLOT_OCCUPIED &&
            strcmp(hashmap->slots[index].key, key) == 0) {
            return hashmap->slots[index].value; 
        }
        index = (index + 1) % hashmap->cap;
        if (index == start_index) break;
    }

    return NULL; 
}


int hashmap_remove(HashMap *hashmap, const char *key) {
    uint64_t index = hash_function(key, hashmap->cap);
    uint64_t start_index = index;

    while (hashmap->slots[index].state != SLOT_EMPTY) {
        if (hashmap->slots[index].state == SLOT_OCCUPIED &&
            strcmp(hashmap->slots[index].key, key) == 0) {
            HASHMAP_FREE(hashmap->slots[index].key);
            hashmap->slots[index].state = SLOT_DELETED; 
            hashmap->count--;
            return 0;
        }
        index = (index + 1) % hashmap->cap;
        if (index == start_index) break;
    }

    return -1; 
}
#endif // YUIS_HASHMAP
#endif /* ifdef YUIS_UTILS */
