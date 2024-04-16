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
#ifndef DS_H
#define DS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

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

#define list_join(l1, l2) (l1 = list__join(l1, l2, sizeof(*l1)), assert(sizeof(*(l1))==sizeof(*(l2))) )

#define list_resize(l, s)(l = list__resize(l, sizeof(*(l)), s))



void *list__init(size_t element_size){
    list_header_t *header = malloc(element_size + sizeof(list_header_t));
    if(header == NULL)
        return NULL;
    header->size = 0;
    header->capacity = 1;
    return (void*)(header + 1);
}

void *list__resize(void *list, size_t element_size, size_t new_ammount) {
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
    // size_t tombstones;
    char **keys;
    uint64_t *values;
}str_hash_t;

typedef struct{
    size_t length;
    size_t exponent;
    // size_t tombstones;
    uint32_t *keys;
    uint32_t *values;
}hash32_t;

typedef struct{
    size_t length;
    size_t exponent;
    // size_t tombstones;
    uint64_t *keys;
    uint64_t *values;
}hash64_t;

#define STR_HASH_MISSING UINT64_MAX
#define HASH32_MISSING UINT32_MAX
#define HASH64_MISSING UINT64_MAX

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

#ifndef PATH_SEPARATOR
    #define PATH_SEPARATOR '/'
    #define PATH_SEPARATOR_STR "/"
#endif 


typedef struct{
    size_t length;
    char  *cstr;
}string_t;

#define str(cstring) ((string_t){.length = strlen(cstring), .cstr = (cstring)})
#define str_null     ((string_t){.length = 0, .cstr = NULL})
#define str_alloc(str_size) ((string_t){.length = (str_size), .cstr = malloc((str_size) + 1)})
#define str_free(string) (free((string).cstr))

#define string_join(...)  (string_join_(str(""),__VA_ARGS__, str_null))
#define string_path(...)  (string_join_(str(PATH_SEPARATOR_STR),__VA_ARGS__, str_null))
#define string_join_sep(separator, ...)  (string_join_((separator),__VA_ARGS__, str_null))

string_t string_join_(string_t separator, ...);
string_t string_dup(string_t string);
string_t string_dup_len(string_t string, size_t length);
bool     string_equal(string_t a, string_t b);
string_t string_get_ext(string_t filename);
string_t string_get_path(string_t filename);

typedef string_t* string_temp_t;

string_t str_temp(string_temp_t *temp, string_t string);
void     str_temp_free(string_temp_t temp);
#define  str_temp_join(temp, ...) (str_temp(temp, string_join_(str(""),__VA_ARGS__, str_null)))



string_t string_join_(string_t separator, ...){
    va_list args;
    va_start(args, separator);
    
    size_t total_size = 0;
    string_t current_string = va_arg(args, string_t);

    for(;current_string.cstr != NULL; current_string = va_arg(args, string_t)){
        total_size += current_string.length + separator.length; 
    }
    total_size -= separator.length;
    va_end(args);

    string_t joined_string = str_alloc(total_size);
    va_start(args, separator);
    
    for(size_t current_character = 0; current_character < total_size;){
        if(current_character != 0){
            memcpy(joined_string.cstr + current_character, separator.cstr, separator.length);
            current_character += separator.length;
        }
        string_t current_string = va_arg(args, string_t);

        memcpy(joined_string.cstr + current_character, current_string.cstr, current_string.length);
        current_character += current_string.length;
    }
    joined_string.cstr[total_size] = '\0';
    va_end(args);
    return joined_string;
}

string_t string_dup(string_t string){
    string_t duplicated = str_alloc(string.length);
    memcpy(duplicated.cstr, string.cstr, string.length + 1);
    return duplicated;
}

string_t string_dup_len(string_t string, size_t length){
    string_t duplicated = str_alloc(string.length);
    memcpy(duplicated.cstr, string.cstr, length + 1);
    return duplicated;
}

bool string_equal(string_t a, string_t b){
    return a.length == b.length && memcmp(a.cstr, b.cstr, a.length) == 0;
}


string_t string_get_ext(string_t filename){
    if(!filename.cstr)
        return str_null;
    char *last_point= strrchr(filename.cstr, '.');
    if(!last_point)
        return filename;
    return string_dup(str(last_point));
}


string_t string_get_path(string_t filename){
    if(!filename.cstr)
        return str_null;
    
    char *last_slash = strrchr(filename.cstr, PATH_SEPARATOR);
    
    if(!last_slash)
        return filename;
    return string_dup((string_t){
        .cstr = filename.cstr,
        .length = last_slash - filename.cstr
    });
}


string_t str_temp(string_temp_t *arena, string_t str){
    list_push(*arena, str);
    return str;
}

void str_temp_free(string_temp_t temp){
    for(size_t i = 0; i < list_size(temp); i++){
        str_free(temp[i]);
    }
    list_free(temp);
}






#endif 
