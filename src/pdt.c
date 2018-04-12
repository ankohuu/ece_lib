#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "base_def.h"
#include "hash_map.h"
#include "list.h"
#include "pdt.h"
#include "attr.h"
#include "g1_fmt.h"

static hash_map g_pdt_map;

struct edge_attr * pdt_add_attr(struct edge_pdt *pdt, unsigned int topic, enum edge_attr_type type, unsigned long len)
{
    struct pdt_attr *attr;
    struct edge_attr *eattr;

    if (NULL == pdt || 0 == topic || EDGE_ATTR_BUTT <= type || 0 == len)
        return NULL;

    attr = hash_map_get(&pdt->attr_map, (void *)&topic);
    if (NULL == attr) {
        attr = (struct pdt_attr *)malloc(sizeof(*attr));
        if (unlikely(NULL == attr))
            return NULL;
        memset(attr, 0x0, sizeof(*attr));
        attr->topic = topic;
        hash_map_put(&pdt->attr_map, (void *)&attr->topic, (void *)attr);
    }
    attr->ref++;
    eattr = add_attr(topic, type, len);
    if (unlikely(NULL == eattr)) {
        attr->ref--;
        if (0 == attr->ref) {
	        hash_map_remove(&pdt->attr_map, (void *)&topic);
            free(attr);
        }
    }
    return eattr;
}

void pdt_del_attr(struct edge_pdt *pdt, unsigned int topic)
{
    struct pdt_attr *attr;

    if (NULL == pdt || 0 == topic)
        return;

    /* find */
    attr = hash_map_get(&pdt->attr_map, (void *)&topic);
    if (unlikely(NULL == attr))
        return;

    attr->ref--;
    if (0 == attr->ref) {
        /* delete devices attribute value */
        hash_map_remove(&pdt->attr_map, (void *)&topic);
        free(attr);
    }

    return del_attr(topic);
}

struct edge_pdt *add_pdt(unsigned int topic, enum edge_pdt_endian endian)
{
    struct edge_pdt *pdt;

    if (true == hash_map_contains_key(&g_pdt_map, &topic)) 
        return NULL;

	/* build self */
    pdt = (struct edge_pdt *)malloc(sizeof(*pdt));
    if (NULL == pdt)
        return NULL;
    lib_printf("add product topic:0x%x endian:%u", topic, endian);
    pdt->topic = topic;
    pdt->endian = endian;

	/* build for other */
	//g1_fmt_set_init(&module->gfs);
    INIT_LIST_HEAD(&pdt->head);
    hash_map_init(&pdt->attr_map, 4, int_cmp, int_hash_func);
	hash_map_init(&pdt->fmt_map, 4, int_cmp, int_hash_func);

	/* hello world */
    hash_map_put(&g_pdt_map, (void *)&pdt->topic, (void *)pdt);

    return pdt;
}

struct edge_pdt *get_pdt(unsigned int topic)
{
    return hash_map_get(&g_pdt_map, (void *)&topic);
}

void del_pdt(unsigned int topic)
{
	struct edge_pdt *pdt;
	struct list_node *pos, *next;

	/* find */
	pdt = hash_map_get(&g_pdt_map, (void *)&topic);
	if (unlikely(NULL == pdt))
		return;

    lib_printf("del product topic:0x%x", topic);
	/* down escape */
		//device
	for (next = (pos = (&pdt->head)->n.next, pos->next); pos != &(&pdt->head)->n; pos = next, next = pos->next) {
        //dev = container_of(pos, struct device, list);
        //lib_printf("del dev oid:%s", dev->oid);
        //delete_device_internal(dev);
    }
		//analysis rules

	/* up escape */
	hash_map_remove(&g_pdt_map, (void *)&topic);

	/* free */
    free(pdt);

    return;
}

void show_pdt(struct edge_pdt *pdt)
{
    if (NULL == pdt)
        return;
    lib_printf("product topic:0x%x endian %s", pdt->topic, 
        (EDGE_BIG_ENDIAN == pdt->endian)?"BIG":"SMALL");
	show_g1_fmt_map(&pdt->fmt_map);
#if 0
	for (next = (pos = (&module->head)->n.next, pos->next); pos != &(&module->head)->n; pos = next, next = pos->next) {
        dev = container_of(pos, struct device, list);
        lib_printf("\tdev oid:%s", dev->oid);
    }
#endif
    return;
}

void hash_show_pdt(hash_map *map, void *key, void *data)
{
    struct edge_pdt *pdt= (struct edge_pdt *)data;
    return show_pdt(pdt);
}

void show_all_pdt(void)
{
    hash_map_work(&g_pdt_map, hash_show_pdt);
    return;
}

int pdt_init(void)
{
	hash_map_init(&g_pdt_map, 16, int_cmp, int_hash_func);
	return 0;
}

