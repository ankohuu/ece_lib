#include <stdio.h>

#include "base_def.h"
#include "hash_map.h"
#include "g1_fmt.h"
#include "pdt.h"

/* called by ctrl plane, use topic */
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
  	lib_printf("product 0x%x add g1 packet format key:0x%x", topic, key);
	hash_map_put(&pdt->fmt_map, (void *)&fmt->key, (void *)fmt);
	return fmt; 
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

	lib_printf("product 0x%x delete g1 packet format key:0x%x", topic, key);
	hash_map_remove(&pdt->fmt_map, (void *)&key);
	free(fmt);
	return;
}

void del_g1_fmt_internal(struct g1_fmt *fmt)
{
    if (NULL == fmt)
        return;

    hash_map_remove(&fmt->pdt->fmt_map, (void *)&fmt->key);
    free(fmt);
    return;
}

void hash_del_fmt(hash_map *map, void *key, void *data)
{
    struct g1_fmt * fmt = (struct g1_fmt *)data;
    return del_g1_fmt_internal(fmt);
}

/* called by data plane, use edge_pdt struct */
struct g1_fmt * get_g1_fmt(struct edge_pdt *pdt, unsigned int key)
{
	if (unlikely(NULL == pdt))
		return NULL;
	return hash_map_get(&pdt->fmt_map, (void *)&key);
}

