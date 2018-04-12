#include <stdio.h>

#include "base_def.h"
#include "hash_map.h"
#include "g1_fmt.h"
#include "pdt.h"

struct g1_token * add_g1_token_internal(struct g1_fmt *fmt, unsigned int token_topic, enum edge_attr_type type, 
					unsigned long offset, unsigned long len)
{
	struct g1_token *token;
	struct g1_token **ptr;
    struct g1_token *cur, **res;
    unsigned long offset1, len1;

	token = (struct g1_token *)malloc(sizeof *token);
    if (NULL == token)
        return NULL;
	token->topic = token_topic;
    token->offset = offset;
    token->len = len;
   
    ptr = &fmt->token;
    offset1 = len1 = 0;
    res = NULL;

    while (NULL != (cur = *ptr)) {
        if (token_topic == cur->topic) {
			free(token);
            return NULL;
        }
        if (offset >= offset1 + len1 && offset + len <= cur->offset) {
            res = ptr; 
            break;
        }
        offset1 = cur->offset;
        len1 = cur->len;
        ptr = &((*ptr)->next);
    }

    if (NULL == res && offset >= offset1 + len1 && offset + len <= 10000 /* fmt->size */)  {
        res = ptr;
    }

	if (NULL == res)
	{
		free(token);
		return NULL;
	}

	token->next = *res;
	*res = token;
	lib_printf("add token topic[0x%8.8x] offset:%lu len:%lu", token_topic, offset, len);
	return token;
}

void del_g1_token_internal(struct g1_fmt *fmt, unsigned int topic)
{
	struct g1_token *token, **ptr;
	
	ptr = &fmt->token;
	while (NULL != (token = *ptr))  {
        if (topic == token->topic) {
            *ptr = token->next;
            free(token); 
			lib_printf("del token topic[0x%8.8x]", topic);
            break;
        }
        ptr = &((*ptr)->next);
    }
	return;
}

void show_g1_fmt_token(struct g1_fmt *fmt)
{
	struct g1_token *token = fmt->token;

	while (token) {
		lib_printf("\t\ttoken device topic[0x%8.8x] offset:%lu len:%lu",
				token->topic, token->offset, token->len);	
		token = token->next;
	}
	
	return;
}

struct g1_token * add_g1_token(unsigned int topic, unsigned int key, 
			unsigned int token_topic, enum edge_attr_type type, unsigned long offset, unsigned long len)
{
	struct g1_token *token;
	struct g1_fmt *fmt;
	struct edge_attr *attr;

	if (0 == topic || 0 == token_topic || 0 == len) {
        return NULL;
    }

	fmt = get_g1_fmt(topic, key);
	if (unlikely(NULL == fmt))
		return NULL;

	token = add_g1_token_internal(fmt, token_topic, type, offset, len);
	if (unlikely(NULL == token))
		return NULL;

	attr = add_pdt_attr(fmt->pdt, token_topic, type, len);
	if (NULL == attr) {
		lib_printf("add_pdt_attr error");
		del_g1_token_internal(fmt, token_topic);
	}

	return token;
}

void del_g1_token(unsigned int topic, unsigned int key, unsigned int token_topic)
{
	struct g1_fmt *fmt;
	
	if (0 == topic || 0 == token_topic) {
        return;
    }

	fmt = get_g1_fmt(topic, key);
	if (unlikely(NULL == fmt))
		return;

	del_pdt_attr(fmt->pdt, token_topic);
	del_g1_token_internal(fmt, token_topic);
	return;
}

struct g1_fmt * add_g1_fmt(unsigned int topic, unsigned int key)
{
    struct g1_fmt *fmt;
	struct edge_pdt *pdt = get_pdt(topic);

	if (unlikely(NULL == pdt))
		return NULL;

    fmt = (struct g1_fmt *)malloc(sizeof *fmt);
    if (unlikely(NULL == fmt))
        return NULL;
	memset(fmt, 0x00, sizeof(*fmt));
    fmt->key = key;
    fmt->pdt = pdt;
  	lib_printf("product 0x%8.8x add g1 packet format key:0x%8.8x", topic, key);
	hash_map_put(&pdt->fmt_map, (void *)&fmt->key, (void *)fmt);
	return fmt; 
}

void del_g1_fmt_internal(struct g1_fmt *fmt)
{
	struct g1_token *next, *token;

	if (NULL == fmt)
        return;

	token = fmt->token;
	while (token) {
		next = token->next;
		del_pdt_attr(fmt->pdt, token->topic);
		free(token);
		token = next;
	}

	hash_map_remove(&fmt->pdt->fmt_map, (void *)&fmt->key);
    free(fmt);
    return;
}

void del_g1_fmt(unsigned int topic, unsigned int key) 
{
	struct g1_fmt *fmt;
	struct edge_pdt *pdt = get_pdt(topic);

	if (unlikely(NULL == pdt))
		return;
	
	fmt = hash_map_get(&pdt->fmt_map, (void *)&key);
	if (unlikely(NULL == fmt))
		return;

	lib_printf("product 0x%8.8x delete g1 packet format key:0x%8.8x", topic, key);
    return del_g1_fmt_internal(fmt);
}


void hash_del_fmt(hash_map *map, void *key, void *data)
{
    struct g1_fmt * fmt = (struct g1_fmt *)data;
    return del_g1_fmt_internal(fmt);
}

struct g1_fmt * get_g1_fmt_internal(struct edge_pdt *pdt, unsigned int key)
{
	if (unlikely(NULL == pdt))
		return NULL;
	return hash_map_get(&pdt->fmt_map, (void *)&key);
}

struct g1_fmt * get_g1_fmt(unsigned int topic, unsigned int key)
{
	struct edge_pdt *pdt = get_pdt(topic);
	if (unlikely(NULL == pdt))
		return NULL;
	return hash_map_get(&pdt->fmt_map, (void *)&key);
}

void show_g1_fmt(struct g1_fmt *fmt)
{
    if (NULL == fmt)
        return;
    lib_printf("\tpacket fmt topic:0x%8.8x", fmt->key);
	show_g1_fmt_token(fmt);
    return;
}

void hash_show_fmt(hash_map *map, void *key, void *data)
{
    struct g1_fmt *fmt = (struct g1_fmt *)data;
    return show_g1_fmt(fmt);
}

void show_g1_fmt_map(hash_map *map)
{
	hash_map_work(map, hash_show_fmt);
	return;
}

