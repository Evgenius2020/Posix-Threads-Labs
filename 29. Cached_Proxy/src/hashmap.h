#include <stdlib.h>

#define HASHMAP_ELEMENT_KEY_LENGTH 256
#define HASHMAP_HASH_MAX 100

typedef struct Hashmap_List_Element
{
    char key[HASHMAP_ELEMENT_KEY_LENGTH];
    void *value;
    size_t value_size;
    struct Hashmap_List_Element *next;
} Hashmap_List_Element;

typedef struct
{
    Hashmap_List_Element *lists[HASHMAP_HASH_MAX];
} Hashmap;

void hashmap_init(Hashmap *hashmap);
void hashmap_insert(Hashmap *hashmap, char *key, void *value, size_t value_size);
Hashmap_List_Element *hashmap_get(Hashmap *hashmap, char *key);
void hashmap_dispose(Hashmap *hashmap);