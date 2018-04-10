/**
 * @file
 * Simple hash map implementation.
 */

#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "linked_list.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/**
 * Comparator function to determine whether `*l` and `*r` are equal.
 * @return Negative if `*l` is less than `*r`; zero if `*l` is equal to `*r`; positive if `*l` is greater than `*r`.
 */
typedef int (*hash_map_comparator)(const void *l, const void *r);

/**
 * Hash function
 * @param key key to hash
 * @param capacity maximum size of the map
 * @return an offset within the range `[0, capacity)`
 */
typedef size_t (*hash_map_hash_func)(const void *key, size_t capacity);


/**
 * Hash map object
 */
typedef struct {
	/** Maximum size of hash table */
	size_t capacity;
	/** Size of hash table */
	size_t size;
	/** Hash table */
	linked_list **table;
	/** Key comparator function */
	hash_map_comparator comparator;
	/** Key hash function */
	hash_map_hash_func hash_func;
	/** Keys */
	linked_list *keys;
} hash_map;

/**
 * Just Loop
 */
typedef void (*hash_map_loop)(hash_map *map, void *key, void *data);


/**
 * Key/value pair
 */
typedef struct {
	/** Key */
	void *key;
	/** Value */
	void *value;
} hash_map_pair;

/**
 * Initialize the hash map.
 * @param map hash map structure
 * @param capacity maximum size of the hash map
 * @param comparator key comparator function
 * @param hash_func key hash function
 */
void hash_map_init(hash_map *map, size_t capacity, hash_map_comparator comparator, hash_map_hash_func hash_func);

/**
 * Free the hash map.
 * This function will also free the table of internal linked lists.
 * @param map hash map structure
 */
void hash_map_free(hash_map *map);

/**
 * Get the value for the given key.
 * @param map hash map structure
 * @param key key for value to fetch
 * @return pointer to the value
 */
void *hash_map_get(hash_map *map, void *key);

/**
 * Insert the value into the map.
 * @param map hash map structure
 * @param key key associated with value
 * @param value value associated with key
 */
void hash_map_put(hash_map *map, void *key, void *value);

/**
 * Remove the mapping from the map if this key exists. Calling this on
 * a key for which there is no mapping does nothing (does not error).
 * @param map hash map structure
 * @param key key for mapping to remove
 */
void hash_map_remove(hash_map *map, void *key);

/**
 * Returns number of key-value pairs in the map
 * @param map hash map structure
 * @return size of the hash map
 */
size_t hash_map_size(hash_map *map);

/**
 * Returns a linked list that contains all keys in the map
 * @param map hash map structure
 * @return a linked list containing all keys
 */
linked_list *hash_map_keys(hash_map *map);

/**
 * Removes all key/value pairs from the map
 * @param map hash map structure
 */
void hash_map_clear(hash_map *map);

/**
 * Check if the map contains the given key
 * @param map hash map structure
 * @param key the key to check
 * @return true if map contains key
 */
bool hash_map_contains_key(hash_map *map, void *key);

void hash_map_work(hash_map *map, hash_map_loop loop);

/* user add */
static inline int int_cmp(const void *l, const void *r)
{
    return *((unsigned int *) l) - *((unsigned int *) r);
}

static inline size_t int_hash_func(const void *key, size_t capacity) {
    unsigned int a = *(unsigned int *)key;
    a = (a ^ 61) ^ (a >> 16);
    a = a + (a << 3);
    a = a ^ (a >> 4);
    a = a * 0x27d4eb2d;
    a = a ^ (a >> 15);
    return a%capacity;
}

static inline int str_cmp(const void *l, const void *r)
{
    return strncmp(l, r, strlen(l));
}

static inline size_t str_hash_func(const void *key, size_t capacity)
{  
    unsigned int h = 0;  
    unsigned int x;  
    char *str = (char *)key;

    while(*str)  
    {  
        h = (h << 4) + *str++;  
        x = h & 0xF0000000L;  
        if(x)  
        {  
            h ^= x>>24;  
            h &= ~x;  
        }  
    }  
    return (h & 0x7FFFFFFF)%capacity;  
} 

#endif

