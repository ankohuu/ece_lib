#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "base_def.h"
#include "hash_map.h"
#include "attr.h"

static hash_map g_attr_map;
static char *g_attr_str[EDGE_ATTR_BUTT] = {"Number", "String"};

void num_to_value(unsigned char *data, unsigned long len, void *pnum)
{
    unsigned long num = 0;
    printf("num to value functions call\r\n");
    if (NULL == data || 0 ==len || NULL == pnum)
        return;
    
    if (1 == len)
        num = *data;
    else if (2 == len)
        num = ntohs(*(unsigned short *)data);
    else if (4 == len)
        num = ntohl(*(unsigned int *)data);
    *(unsigned long *)pnum = num;
    return;
}

void num_to_string(unsigned char *data, unsigned long len, char *res)
{
    unsigned long num;
    printf("num to string functions call\r\n");
    if (NULL == data || 0 ==len || NULL == res)
        return;
    
    num_to_value(data, len, &num);
    sprintf(res, "%lu", num);
    printf("res:%s\r\n", res);
    return;
}

struct attr_operations g_attr_operations_array[EDGE_ATTR_BUTT]
         = {
            {
                num_to_value,
                num_to_string
            },
            {NULL}
           };

struct edge_attr *add_attr(unsigned int topic, enum edge_attr_type type, unsigned long len)
{
    struct edge_attr *attr;

    if (0 == topic || EDGE_ATTR_BUTT <= type || 0 == len)
        return NULL;

	attr = hash_map_get(&g_attr_map, (void *)&topic);
    if (NULL == attr) {
        attr = (struct edge_attr *)malloc(sizeof(*attr));
        if (unlikely(NULL == attr))
		    return NULL;
        memset(attr, 0x0, sizeof(*attr));
        lib_printf("add attribute topic:0x%8.8x type:%u len:%lu", topic, type, len);
        attr->topic = topic;
        attr->type = type;
        attr->len = len;
        hash_map_put(&g_attr_map, (void *)&attr->topic, (void *)attr);
    } else {
        lib_printf("attribute topic:0x%8.8x add reference", topic);
    }
    attr->ref++;
    return attr;
}

struct edge_attr *get_attr(unsigned int topic)
{
    return hash_map_get(&g_attr_map, (void *)&topic);
}

void del_attr_ref(unsigned int topic, unsigned long ref)
{
	struct edge_attr *attr;

	/* find */
	attr = hash_map_get(&g_attr_map, (void *)&topic);
	if (unlikely(NULL == attr))
		return;

	if (attr->ref >= ref) {
		attr->ref -= ref;
	} else {
		lib_printf("del ref error topic:0x%8.8x ref:%lu", topic, ref);
		attr->ref = 0;
	}

    if (0 == attr->ref) {
        lib_printf("del attribute topic:0x%8.8x", topic);
	    hash_map_remove(&g_attr_map, (void *)&topic);
        free(attr);
    }
    else
        lib_printf("attribute topic:0x%8.8x sub reference %lu", topic, ref);
        
    return;
}

void del_attr(unsigned int topic)
{
	return del_attr_ref(topic, 1);
}

void show_attr(struct edge_attr *attr)
{
    if (NULL == attr)
        return;
    lib_printf("attribute topic:0x%8.8x type:%s len:%lu reference:%lu", 
            attr->topic, g_attr_str[attr->type], attr->len, attr->ref);
    return;
}

void hash_show_attr(hash_map *map, void *key, void *data)
{
    struct edge_attr *attr= (struct edge_attr *)data;
    return show_attr(attr);
}

void show_all_attr(void)
{
    hash_map_work(&g_attr_map, hash_show_attr);
    return;
}

int attr_init(void)
{
	hash_map_init(&g_attr_map, 64, int_cmp, int_hash_func);
	return 0;
}

