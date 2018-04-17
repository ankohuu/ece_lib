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
#include "mgt.h"


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

struct attr_val * add_dev_attr(struct dev *dev, unsigned int topic, unsigned char *val, unsigned long len)
{
    struct attr_val *attr = 0;

    if (NULL == dev || 0 == topic || NULL == val || 0 == len)
        return NULL;
	
    attr = (struct attr_val *)malloc(sizeof *attr);
    if (NULL == attr)
        return NULL;
    memset(attr, 0x00, sizeof(*attr));
    attr->len = len;
    attr->topic = topic;
	if (memcmp(attr->val, val, len)) {
		memcpy(attr->val, val, len);
		dev->update_ts = time(NULL);
		//GET_TOKEN_OPERATIONS(token_type_num).to_string(val, len, attr->str);
	}
	hash_map_put(&dev->attr_map, (void *)&attr->topic, (void *)attr);
	lib_printf("add_dev_attr topic:%8.8x", topic);
    return attr;
}

struct attr_val * upt_dev_attr(struct dev *dev, unsigned int topic, unsigned char *val, unsigned long len)
{
    struct attr_val *attr;

    if (NULL == dev || 0 == topic || NULL == val || 0 == len)
        return NULL;
    
    dev->ts = time(NULL);
    attr = (struct attr_val *)hash_map_get(&dev->attr_map, (void *)&topic);
    if (NULL == attr) {
        attr = add_dev_attr(dev, topic, val, len);
    } else {
        attr->len = len;
        if (memcmp(attr->val, val, len)) {
            memcpy(attr->val, val, len);
            dev->update_ts = dev->ts;
            //GET_TOKEN_OPERATIONS(token_type_num).to_string(val, len, attr->str);
        }
    }

	#if 0
	if (NULL != attr)
    	report_device_to_server(dev, attr);
	#endif

    return attr;    
}

void show_dev_attr(struct attr_val *attr)
{
	unsigned long i;
	
    if (NULL == attr)
        return;
    lib_printf("attr topic:%8.8x", attr->topic);
	lib_printf("val:%s len:%lu", attr->str, attr->len);
	for (i=0; i<attr->len; i++)
		printf("%2.2x ", attr->val[i]);
	printf("\n");
    return;
}

void hash_show_dev_attr(hash_map *map, void *key, void *data)
{
    struct attr_val *attr = (struct attr_val *)data;
    return show_dev_attr(attr);
}

void fresh_dev(struct dev *dev)
{
	if (NULL == dev)
		return;

	dev->ts = time(NULL);
	list_move_tail(&dev->age_list, &g_dev_age_head);
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

void del_dev_attr_hash(hash_map *map, void *key, void *data)
{
	hash_map_remove(map, key);
	lib_printf("del dev attr topic:%8.8x", *(unsigned int *)key);
	free(data);
	return;
}

static void __del_dev(struct dev *dev)
{
	list_del(&dev->list);
	list_del(&dev->age_list);

	hash_map_work(&dev->attr_map, del_dev_attr_hash);
	hash_map_free(&dev->attr_map);

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

void show_dev(struct dev *dev)
{
    if (NULL == dev)
        return;
    lib_printf("dev module:%lu oid %s", dev->module, dev->oid);
	lib_printf("time_stamp:%lu update time_stamp:%lu", dev->ts, dev->update_ts);
	hash_map_work(&dev->attr_map, hash_show_dev_attr);
    return;
}

void hash_show_dev(hash_map *map, void *key, void *data)
{
    struct dev *dev = (struct dev *)data;
    return show_dev(dev);
}

void show_all_dev(void)
{
    hash_map_work(&g_dev_map, hash_show_dev);
    return;
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
	        if (now - dev->ts > get_dev_age_interval()) {
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

