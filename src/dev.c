#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "md5.h"
#include "base_def.h"
#include "hash_map.h"
#include "list.h"
#include "pdt.h"
#include "dev.h"

static hash_map g_dev_map;
static struct list_head g_dev_age_head;

void get_dev_oid(enum edge_access_type type, unsigned char *addr, unsigned long addr_len, char *oid)
{
    char tmp[256];
    unsigned long i;
    
    if (NULL == oid)
        return;

    snprintf(tmp, 256, "%4.4x", type);
    for (i=0; i<addr_len; i++)
        sprintf(tmp, "%s%2.2x", tmp, addr[i]);

    md5_string(tmp, strlen(tmp), oid);
    oid[32] = 0;
    return;
}

static struct dev * __add_dev(struct edge_pdt *pdt, char* oid, unsigned long module)
{
	struct dev *dev;
	
	if (true == hash_map_contains_key(&g_dev_map, oid)) 
        return NULL;

	dev = (struct dev *)malloc(sizeof *dev);
	if (NULL == dev)
        return NULL;

	memset(dev, 0, sizeof(*dev));
	memcpy(dev->oid, oid, 32);
	dev->module = module;
	dev->ts = time(NULL);
	hash_map_init(&dev->attr_map, 16, int_cmp, int_hash_func);
	INIT_LIST_NODE(&dev->list);
	INIT_LIST_NODE(&dev->age_list);

	hash_map_put(&g_dev_map, (void *)dev->oid, (void *)dev);
	list_add_tail(&dev->list, &pdt->head);
	list_add_tail(&dev->age_list, &g_dev_age_head);
	lib_printf("add dev oid:%s", dev->oid);
	return dev;
}

struct dev *add_dev(unsigned short module, unsigned char* addr, unsigned int addr_len, 
				enum edge_access_type type, unsigned int topic)
{
    struct edge_pdt *pdt;
	char oid[64];

    if (NULL == addr || 0 == addr_len || 0 == topic)
        return NULL;
    pdt = get_pdt(topic);
    if (unlikely(NULL == pdt))
        return NULL;

	memset(oid, 0, 64);
	get_dev_oid(type, addr, addr_len, oid);

	return __add_dev(pdt, oid, module);
}
			
struct dev *get_dev(enum edge_access_type type, unsigned char *addr, unsigned long addr_len)
{
	char oid[64];

	get_dev_oid(type, addr, addr_len, oid);
	return hash_map_get(&g_dev_map, oid);
}

static void __del_dev(struct dev *dev)
{
	list_del(&dev->list);
	list_del(&dev->age_list);

	//hash_map_work(&dev->attr_map, device_hash_del_attr);
	//hash_map_free(&dev->attr_map);

	hash_map_remove(&g_dev_map, (void *)dev->oid);
    free(dev);
    return;
}

void del_dev(enum edge_access_type type, unsigned char *addr, unsigned long addr_len)
{
    struct dev *dev;
	
    if (unlikely(NULL == addr || 0 == addr_len))
        return;

	dev = get_dev(type, addr, addr_len);
    if (unlikely(NULL == dev))
        return;
	return __del_dev(dev);
}

void do_device_ageing(unsigned long sec)
{
	static time_t last_sec = 0;
	struct list_node *pos, *next;
    struct dev *dev;
	time_t now = time(NULL);

	if (0 == last_sec) {
		last_sec = now;
		return;
	}

	if (now - last_sec > 60) {
		for (next = (pos = (&g_dev_age_head)->n.next, pos->next); pos != &(&g_dev_age_head)->n; pos = next, next = pos->next) {
	        dev = container_of(pos, struct dev, age_list);
	        if (now - dev->ts > 1200) {
				lib_printf("\tageing dev oid:%s", dev->oid);
				__del_dev(dev);	
			} else
				break;
	    }
		last_sec = now;
	}

	return;
}

int dev_init(void)
{
    hash_map_init(&g_dev_map, 16, str_cmp, str_hash_func);
	INIT_LIST_HEAD(&g_dev_age_head);
	return 0;
}

