#include "hashmap.h"
#include "lib/mutex.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

void hashmap_init(Hashmap *hashmap)
{
    for (int i = 0; i < HASHMAP_HASH_MAX; i++)
        hashmap->lists[i] = NULL;
}

unsigned hash_func(char *key)
{
    unsigned sum = 0;
    for (unsigned i = 0; i < strlen(key); i++)
        sum += key[i];
    return sum % HASHMAP_HASH_MAX;
}

void hashmap_insert(Hashmap *hashmap, char *key, void *value, size_t value_size)
{
    Hashmap_List_Element *element = malloc(sizeof(Hashmap_List_Element));
    element->key = malloc(strlen(key) + 2);
    strcpy(element->key, key);
    unsigned hash = hash_func(key);

    element->next = hashmap->lists[hash];
    hashmap->lists[hash] = element;

    element->value = malloc(value_size);
    memcpy(element->value, value, value_size);
    element->value_size = value_size;
}

Hashmap_List_Element *hashmap_get(Hashmap *hashmap, char *key)
{
    unsigned hash = hash_func(key);
    Hashmap_List_Element *res = NULL;
    Hashmap_List_Element *element = hashmap->lists[hash];
    for (; element; element = element->next)
        if (!strcmp(element->key, key))
        {
            res = element;
            break;
        }

    return res;
}

void hashmap_dispose(Hashmap *hashmap)
{
    for (int i = 0; i < HASHMAP_HASH_MAX; i++)
        if (hashmap->lists[i])
            while (hashmap->lists[i])
            {
                Hashmap_List_Element *next = hashmap->lists[i]->next;
                free(hashmap->lists[i]->key);
                free(hashmap->lists[i]->value);
                free(hashmap->lists[i]);
                hashmap->lists[i] = next;
            }
}