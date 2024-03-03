/*
 *  Copyright © 2024 hadron13
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 *  and associated documentation files (the “Software”), to deal in the Software without
 *  restriction, including without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all copies or
 *  substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 *  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef ds_H
#define ds_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

typedef struct {
    size_t size, capacity;
} list_header_t;

#define list_header(l)  ((list_header_t *)(l) - 1)

#define list_size(l)    ((l) ? list_header(l)->size : 0)
#define list_capacity(l)((l) ? list_header(l)->capacity : 0)
#define list_full(l)    (list_size(l) == list_capacity(l))
#define list_init(t)    (list__init(sizeof(t)))
#define list_free(l)    (free(list_header(l)))

#define list_push(l, e) (  (l) = (!l)? list_init(*l) : \
                         list_full(l)? list__resize((l), sizeof(*l),list_capacity(l)*2):\
                                        (l),\
                         (l)[list_header(l)->size++] = (e))

#define list_pop(l)     (list_header(l)->size--, (l) = (list_size(l) <= list_capacity(l) / 2)?\
                         list__resize((l), sizeof(*l), list_capacity(l)/2) : (l))

#define list_swap_delete(l, i)((l)[i] = (l)[list_size(l) - 1], list_pop(l))

#define list_join(l1, l2)(l1 = list__join(l1, l2, sizeof(*l1)), assert(sizeof(*(l1))==sizeof(*(l2)))

#define list_resize(l, s)(l = list__resize(l, sizeof(*(l)), s))
                        


static void *list__init(size_t element_size){
    list_header_t *header = malloc(element_size + sizeof(list_header_t));
    if(header == NULL)
        return NULL;
    header->size = 0;
    header->capacity = 1;
    return (void*)(header + 1);
}

static void *list__resize(void *list, size_t element_size, size_t new_ammount) {
    list_header(list)->capacity = new_ammount;
    return (list_header_t*)realloc(list_header(list), (element_size * new_ammount) + sizeof(list_header_t)) + 1;
}

static void *list__join(void *list1, void *list2, size_t element_size){
    size_t total_size = (list_size(list1) + list_size(list2));

    list1 = list__resize(list1, element_size, total_size);
    memcpy( (unsigned char*)list1 + list_size(list1), list2, list_size(list2) * element_size);

    list_header(list1)->size += list_size(list2);

    return list1;
}

/*
 *  ||      ||     /\     //===\\  ||      ||  ========    /\     ||==\\   ||     ======
 *  ||      ||    //\\    ||       ||      ||     ||      //\\    ||   ||  ||     ||
 *  ||======||   //  \\   \\===\\  ||======||     ||     //  \\   ||==\\   ||     ||===
 *  ||      ||  //====\\        || ||      ||     ||    //====\\  ||   ||  ||     ||
 *  ||      || //      \\ \\===//  ||      ||     ||   //      \\ ||==//   ||==== ======
 *
 *
 *  Usage:
 *  hash32_t my_table = hash32_create(8); // creates table with 2^8 buckets ready to use
 *
 *  hash32_insert(&my_table, 1984, 42);   // inserts value 42 with key 1984, resizes table if needed
 *
 *  hash32_resize(&my_table, 7);          // resizes and rehashes table to new exponent
 *
 *  uint32_t value = hash32_lookup(&my_table, 1984); // lookups value with key 1984
 *
 *  hash32_delete(&my_table, 1984);       // deletes and replaces key with a gravestone
 *
 *  hash32_destroy(&my_table);            // free the table's contents
 *
 *  
 *  for other hashtable types, replace hash32 with hash64 or str_hash
 *  
 *  special note for str_hash: strings are not copied, making sure they live is out of the table's scope. 
 *
 *  note for hash32 and hash64: empty keys are marked with UINT32_MAX and UINT64_MAX respectively, 
 *  and gravestone as UINT32_MAX -1 and UINT64_MAX -1 respectively, therefore do not use these values as keys
 */


// TODO: gravestone counting & rebuilding

typedef struct{
    size_t length;
    size_t exponent;
    char **keys;
    uint64_t *values;
}str_hash_t;

typedef struct{
    size_t length;
    size_t exponent;
    uint32_t *keys;
    uint32_t *values;
}hash32_t;

typedef struct{
    size_t length;
    size_t exponent;
    uint64_t *keys;
    uint64_t *values;
}hash64_t;


static int32_t  msi_lookup(uint64_t hash, int exp, int32_t idx);
static uint64_t fnv1a_hash(const unsigned char *data, size_t length);

static str_hash_t str_hash_create(size_t initial_exponent);
static void       str_hash_resize(str_hash_t *hash_table, size_t new_exponent);
static void       str_hash_insert(str_hash_t *restrict hash_table, const char *restrict key, uint64_t value);
static uint64_t   str_hash_lookup(str_hash_t *restrict hash_table, const char *restrict key);
static uint64_t   str_hash_delete(str_hash_t *restrict hash_table, const char *restrict key);
static void       str_hash_destroy(str_hash_t *hash_table);

static hash32_t   hash32_create(size_t initial_exponent);
static void       hash32_resize(hash32_t *hash_table, size_t new_exponent);
static void       hash32_insert(hash32_t *hash_table, uint32_t key, uint32_t value);
static uint32_t   hash32_lookup(hash32_t *hash_table, uint32_t key);
static uint32_t   hash32_delete(hash32_t *hash_table, uint32_t key);
static void       hash32_destroy(hash32_t *hash_table);

static hash64_t   hash64_create(size_t initial_exponent);
static void       hash64_resize(hash64_t *hash_table, size_t new_exponent);
static void       hash64_insert(hash64_t *hash_table, uint64_t key, uint64_t value);
static uint64_t   hash64_lookup(hash64_t *hash_table, uint64_t key);
static uint64_t   hash64_delete(hash64_t *hash_table, uint64_t key);
static void       hash64_destroy(hash64_t *hash_table);


static int32_t msi_lookup(uint64_t hash, int exp, int32_t idx){
    uint32_t mask = ((uint32_t)1 << exp) - 1;
    uint32_t step = (hash >> (64 - exp)) | 1;
    return (idx + step) & mask;
}

static uint64_t fnv1a_hash(const unsigned char *data, size_t length){
    uint64_t hash = 0xcbf29ce484222325;
    
    for(size_t i = 0; i < length; i++){
        hash ^= data[i];
        hash *= 0x100000001b3;
    }
    return hash;
}

uint32_t lowbias32(uint32_t x){
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

const char *STR_HASH_GRAVESTONE = "\xff anti-cien tech";

static str_hash_t str_hash_create(size_t initial_exponent){
    size_t initial_size = 1 << initial_exponent;
    str_hash_t hash_table = {
        .length   = 0, 
        .exponent = initial_exponent,
        .keys   = calloc(initial_size, sizeof (char*)),
        .values = malloc(initial_size * sizeof (uint64_t*))
    };

    if(hash_table.keys == NULL || hash_table.values == NULL)
        return (str_hash_t){.exponent = 0, .length = 0, .keys = NULL, .values = NULL};
    
    return hash_table;
}

static void str_hash_resize(str_hash_t *hash_table, size_t new_exponent){
    
    size_t old_size = 1 << hash_table->exponent;

    str_hash_t new_hash = str_hash_create(new_exponent);

    for(size_t i = 0; i < old_size; i++){
        if(hash_table->keys[i] == NULL || hash_table->keys[i] == STR_HASH_GRAVESTONE)
            continue;
        
        str_hash_insert(&new_hash, hash_table->keys[i], hash_table->values[i]);
    }

    *hash_table = new_hash;
}

static void str_hash_insert(str_hash_t *restrict hash_table, const char * restrict key, uint64_t value){    

    if(hash_table->length + 1  == (1 << hash_table->exponent) - ((1 << hash_table->exponent) >> 1) ){
        str_hash_resize(hash_table, hash_table->exponent + 1);
    }

    uint64_t hash = fnv1a_hash((unsigned char*)key, strlen(key));
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);

        if(hash_table->keys[index] != NULL && hash_table->keys[index] != STR_HASH_GRAVESTONE && strcmp(hash_table->keys[index], key) != 0){
            continue;
        }

        hash_table->keys[index] = (char*)key;
        hash_table->values[index] = value;
        hash_table->length++;
        return;
    }
}

static uint64_t str_hash_lookup(str_hash_t * restrict hash_table, const char *restrict key){
    uint64_t hash = fnv1a_hash((unsigned char*)key, strlen(key));
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index] == NULL)
            return UINT64_MAX;

        if(strcmp(hash_table->keys[index], key) != 0){
            continue;
        }
        
        return hash_table->values[index];
    }
}
static uint64_t str_hash_delete(str_hash_t * restrict hash_table, const char *restrict key){
    uint64_t hash = fnv1a_hash((unsigned char*)key, strlen(key));
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index] == NULL)
            return 0;

        if(strcmp(hash_table->keys[index], key) != 0){
            continue;
        }
        hash_table->keys[index] = (char*)STR_HASH_GRAVESTONE;
        return hash_table->values[index];
    }
}

static void str_hash_destroy(str_hash_t *hash_table){
    hash_table->length = 0;
    hash_table->exponent = 0;
    free(hash_table->keys);
    free(hash_table->values);
}

#define HASH32_GRAVESTONE (UINT32_MAX-1)

static hash32_t hash32_create(size_t initial_exponent){
    size_t initial_size = 1 << initial_exponent;
    hash32_t hash_table = {
        .length   = 0, 
        .exponent = initial_exponent,
        .keys   = malloc(initial_size * sizeof (*hash_table.keys)),
        .values = malloc(initial_size * sizeof (*hash_table.values))
    };

    memset(hash_table.keys, 0xff, initial_size * sizeof(*hash_table.keys));

    if(hash_table.keys == NULL || hash_table.values == NULL)
        return (hash32_t){0, 0, NULL, NULL};
    
    return hash_table;
}

size_t total_collisions = 0;

static void hash32_resize(hash32_t *hash_table, size_t new_exponent){
    // total_collisions = 0;
    size_t old_size = 1 << hash_table->exponent;

    hash32_t new_hash = hash32_create(new_exponent);

    for(size_t i = 0; i < old_size; i++){
        if(hash_table->keys[i] >= HASH32_GRAVESTONE)
            continue;

        hash32_insert(&new_hash, hash_table->keys[i], hash_table->values[i]);    
    }

    *hash_table = new_hash;
}

static void hash32_insert(hash32_t *hash_table, uint32_t key, uint32_t value){
    if(hash_table->length + 1 == (size_t)((1 << hash_table->exponent) * 0.6f)){
        hash32_resize(hash_table, hash_table->exponent + 1);
    }
    
    // uint64_t hash = lowbias32(key);
    
    //Sppoooky, but somehow works
    uint64_t hash = key;
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);

        if(hash_table->keys[index] < HASH32_GRAVESTONE && hash_table->keys[index] != key){
            total_collisions++;
            continue;
        }

        hash_table->keys[index] = key;
        hash_table->values[index] = value;
        hash_table->length++;
        return;
    }
}

static uint32_t hash32_lookup(hash32_t *hash_table, uint32_t key){
    // uint64_t hash = lowbias32(key);
    uint64_t hash = key;
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index] == UINT32_MAX)
            return 0;
        if(hash_table->keys[index] != key || hash_table->keys[index] == HASH32_GRAVESTONE)
            continue;
        
        return hash_table->values[index];
    }
}

static uint32_t hash32_delete(hash32_t *hash_table, uint32_t key){
    // uint64_t hash = lowbias32(key);
    uint64_t hash = key;
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index] == UINT32_MAX)
            return UINT32_MAX;
        if(hash_table->keys[index] != key || hash_table->keys[index] == HASH32_GRAVESTONE)
            continue;
        
        hash_table->keys[index] = HASH32_GRAVESTONE;

        return hash_table->values[index];
    }
}

static void hash32_destroy(hash32_t *hash_table){
    hash_table->length = 0;
    hash_table->exponent = 0;
    free(hash_table->keys);
    free(hash_table->values);
}


#define HASH64_GRAVESTONE (UINT64_MAX-1)

static hash64_t hash64_create(size_t initial_exponent){
    size_t initial_size = 1 << initial_exponent;
    hash64_t hash_table = {
        .length   = 0, 
        .exponent = initial_exponent,
        .keys   = malloc(initial_size * sizeof (*hash_table.keys)),
        .values = malloc(initial_size * sizeof (*hash_table.values))
    };

    memset(hash_table.keys, 0xff, initial_size * sizeof(*hash_table.keys));

    if(hash_table.keys == NULL || hash_table.values == NULL)
        return (hash64_t){0, 0, NULL, NULL};
    
    return hash_table;
}

static void hash64_resize(hash64_t *hash_table, size_t new_exponent){

    size_t old_size = 1 << hash_table->exponent;

    hash64_t new_hash = hash64_create(new_exponent);

    for(size_t i = 0; i < old_size; i++){
        if(hash_table->keys[i] >= HASH64_GRAVESTONE)
            continue;

        hash64_insert(&new_hash, hash_table->keys[i], hash_table->values[i]);    
    }

    *hash_table = new_hash;
}

static void hash64_insert(hash64_t *hash_table, uint64_t key, uint64_t value){
        if(hash_table->length + 1 == 1 << hash_table->exponent){
        hash64_resize(hash_table, hash_table->exponent + 1);
    }

    uint64_t hash = fnv1a_hash((unsigned char*)&key, sizeof(key));
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);

        if(hash_table->keys[index] < HASH64_GRAVESTONE && hash_table->keys[index] != key){
            continue;
        }

        hash_table->keys[index] = key;
        hash_table->values[index] = value;
        hash_table->length++;
        return;
    }
}

static uint64_t hash64_lookup(hash64_t *hash_table, uint64_t key){
    uint64_t hash = fnv1a_hash((unsigned char*)&key, sizeof(key));
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index] == UINT64_MAX)
            return UINT64_MAX;
        if(hash_table->keys[index] != key || hash_table->keys[index] == HASH64_GRAVESTONE)
            continue;
        
        return hash_table->values[index];
    }
}

static uint64_t hash64_delete(hash64_t *hash_table, uint64_t key){
    uint64_t hash = fnv1a_hash((unsigned char*)&key, sizeof(key));
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index] == UINT64_MAX)
            return 0;
        if(hash_table->keys[index] != key || hash_table->keys[index] == HASH64_GRAVESTONE)
            continue;
        hash_table->keys[index] = HASH64_GRAVESTONE;
        
        return hash_table->values[index];
    }
}

static void hash64_destroy(hash64_t *hash_table){
    hash_table->length = 0;
    hash_table->exponent = 0;
    free(hash_table->keys);
    free(hash_table->values);
}



static char *string_duplicate(const char *src);
static char *string_duplicate_len(const char *src, size_t len);

static char *string_duplicate(const char *src){
    char *new_buffer = malloc(strlen(src) + 1);
    if(!new_buffer)
        return NULL;
    strcpy(new_buffer, src);
    return new_buffer;
}

static char *string_duplicate_len(const char *src, size_t len){

    size_t string_size = strlen(src);
    size_t new_length = len < string_size ? len : string_size;

    char*result=malloc(new_length+1);
    if(result){
        memcpy(result, src, new_length);
        result[new_length] = 0;
    }
    return result;

}

#ifndef PATH_SEPARATOR
    #define PATH_SEPARATOR '/'
#endif 

static char *string_path(const char *filename){
    if(!filename)
        return NULL;
    char *last_slash = strrchr(filename, PATH_SEPARATOR);
    if(!last_slash)
        return NULL;
    
    size_t path_size = (size_t)(last_slash - filename) + 1;
    
    return string_duplicate_len(filename, path_size);
}

static const char *string_extension(const char* filename){
    if(!filename)
        return NULL;
    char *last_point= strrchr(filename, '.');
    if(!last_point)
        return filename;

    return last_point;
}


static char *string_join(size_t number, ...){
    
    if(number <= 0)
        return NULL;

    va_list args; 
    va_start(args, number); 

    size_t total_size = 0;

    for (size_t i = 0; i < number; i++) {
        char *string = va_arg(args, char*);
        if(!string)
            continue;
        
        total_size += strlen(string);
    }

    va_end(args);
 
    char *final_string = malloc(total_size + 1);
    if(!final_string)
        return NULL;
    
    final_string[0] = '\0'; 

    va_start(args, number);
   
    for (size_t i = 0; i < number; i++) {
        strcat(final_string, va_arg(args, char*)); 
    }

    va_end(args);

    return final_string;
}

#endif 


