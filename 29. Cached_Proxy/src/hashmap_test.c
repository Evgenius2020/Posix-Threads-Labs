#include "hashmap.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    Hashmap hashmap;
    hashmap_init(&hashmap);

    if (hashmap_get(&hashmap, "a"))
        printf("a_value is set!!!\n");

    char *data = malloc(10);
    strcpy(data, "a_value");
    hashmap_insert(&hashmap, "a", data, strlen(data) + 1);

    Hashmap_List_Element *el = hashmap_get(&hashmap, "a");
    if (el)
        printf("%s: %s\n", el->key, (char*)el->value);
        
    hashmap_dispose(&hashmap);
}