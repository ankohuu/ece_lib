#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "base_def.h"
#include "hash_map.h"
#include "list.h"
#include "pdt.h"

static hash_map g_pdt_map;

#if 0
void device_module_show_pkt_fmt(hash_map *map, void *key, void *data)
{
    struct g1_fmt *fmt = (struct g1_fmt *)data;

    lib_printf("show packet format key:%x size:%lu",
        fmt->key, fmt->size);
    return;
}

void device_module_show(hash_map *map, void *key, void *data)
{
    struct device_module *module = (struct device_module *)data;
    struct list_node *pos, *next;
    struct device *dev;

    lib_printf("show device module topic:%x scenerio:%lu endian:%lu",
        module->topic, module->scenerio_id, module->endian);
	hash_map_work(module->gfs.fmt_map, device_module_show_pkt_fmt);
    for (next = (pos = (&module->head)->n.next, pos->next); pos != &(&module->head)->n; pos = next, next = pos->next) {
        dev = container_of(pos, struct device, list);
        lib_printf("\tdev oid:%s", dev->oid);
    }
    return;
}

void device_module_show_all(void)
{
    hash_map_work(&g_device_module_map, device_module_show);
    return;
}
#endif


struct edge_pdt *add_pdt(unsigned int topic, enum edge_pdt_endian endian)
{
    struct edge_pdt *pdt;

    if (true == hash_map_contains_key(&g_pdt_map, &topic)) 
        return NULL;

	/* build self */
    pdt = (struct edge_pdt *)malloc(sizeof(*pdt));
    if (NULL == pdt)
        return NULL;
    lib_printf("add product topic:%x endian:%u", topic, endian);
    pdt->topic = topic;
    pdt->endian = endian;

	/* build for other */
	//g1_fmt_set_init(&module->gfs);
    INIT_LIST_HEAD(&pdt->head);

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
#if 0
    struct device_module *module;
    struct list_node *pos, *next;
    struct device *dev;

    if (NULL == (module = get_device_module(topic)))
        return;

    lib_printf("del device module topic:%x", topic);
    for (next = (pos = (&module->head)->n.next, pos->next); pos != &(&module->head)->n; pos = next, next = pos->next) {
        dev = container_of(pos, struct device, list);
        lib_printf("del dev oid:%s", dev->oid);
        delete_device_internal(dev);
    }
	g1_fmt_set_des(&module->gfs);
    hash_map_remove(&g_device_module_map, (void *)&topic);
    free(module);
#endif

	/* find */
	pdt = hash_map_get(&g_pdt_map, (void *)&topic);
	if (unlikely(NULL == pdt))
		return;

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

int pdt_init(void)
{
	hash_map_init(&g_pdt_map, 16, int_cmp, int_hash_func);
	return 0;
}

