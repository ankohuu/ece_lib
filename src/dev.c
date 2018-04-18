#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "md5.h"
#include "cJSON.h"

#include "base_def.h"
#include "hash_map.h"
#include "lib_mqtt.h"
#include "list.h"
#include "expr.h"
#include "pdt.h"
#include "dev.h"
#include "mgt.h"
#include "attr.h"

static char rule_buf[512];
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
	struct edge_attr *ea = NULL;

    if (NULL == dev || 0 == topic || NULL == val || 0 == len)
        return NULL;
	
    attr = (struct attr_val *)malloc(sizeof *attr);
    if (NULL == attr)
        return NULL;
    memset(attr, 0x00, sizeof(*attr));
    attr->len = len;
    attr->topic = topic;
	attr->dev = dev;
	memset(attr->val, 0xFF, MAX_DEVICE_ATTRIBUTE_LEN);
	if (memcmp(attr->val, val, len)) {
		memcpy(attr->val, val, len);
		dev->update_ts = time(NULL);
		ea = get_attr(topic);
		if (NULL != ea)
			GET_ATTR_OPERATIONS(ea->type).to_string(val, len, attr->str);
	}
	hash_map_put(&dev->attr_map, (void *)&attr->topic, (void *)attr);
	lib_printf("add_dev_attr topic:%8.8x", topic);
    return attr;
}

struct attr_val * upt_dev_attr(struct dev *dev, unsigned int topic, unsigned char *val, unsigned long len)
{
    struct attr_val *attr;
	struct edge_attr *ea = NULL;

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
			ea = get_attr(topic);
			if (NULL != ea)
				GET_ATTR_OPERATIONS(ea->type).to_string(val, len, attr->str);
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
	dev->pdt = pdt;
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

void report_dev_attr(struct dev *dev, struct attr_val *attr)
{
    char *out;
    cJSON *root;
    char topic[32];

    root = cJSON_CreateObject();
    if (!root) {
        lib_printf("cJSON create error!");
        return;
    }

    cJSON_AddItemToObject(root, "userID", cJSON_CreateString("lzhouiot"));
    cJSON_AddItemToObject(root, "OIDIndex", cJSON_CreateString(dev->oid));
    memset(topic, 0x00, 32);
    sprintf(topic, "%x", attr->topic);
    cJSON_AddItemToObject(root, "propertyTopic", cJSON_CreateString(topic));
    cJSON_AddItemToObject(root, "value", cJSON_CreateString(attr->str));
	cJSON_AddItemToObject(root, "Flag", cJSON_CreateString("GA"));
    cJSON_AddItemToObject(root, "scenerioID", cJSON_CreateNumber(get_dev_scenerio_id()));
    
    out=cJSON_Print(root);
    send_mqtt_msg(MQTT_OASIS_APP, out, strlen(out));
    cJSON_Delete(root);    
    return;
}

void hash_update_attr(hash_map *map, void *key, void *data)
{
    struct attr_val *attr = (struct attr_val *)data;
	report_dev_attr(attr->dev, attr);
    return;
}

void hash_update_dev(hash_map *map, void *key, void *data)
{
    struct dev *dev = (struct dev *)data;

	if (dev->update_ts > dev->report_ts) {
		hash_map_work(&dev->attr_map, hash_update_attr);
		dev->report_ts = dev->update_ts;
	}
    return;
}

void do_device_update(unsigned long sec)
{
	static time_t last_sec = 0;

	time_t now = time(NULL);

	if (0 == last_sec) {
		last_sec = now;
		return;
	}

	if (now - last_sec > get_pkt_up_interval()) {
		hash_map_work(&g_dev_map, hash_update_dev);
		last_sec = now;
	}

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


void hash_get_attr_val(hash_map *map, void *key, void *data)
{
    struct attr_val *attr = (struct attr_val *)data;
	sprintf(rule_buf, "%sa%8.8x=%s,", rule_buf, attr->topic, attr->str);
	return;
}

void check_rule_engine(struct dev * dev)
{
	struct expr_var_list vars = {0};
	struct expr_func user_funcs[] = {{0}};
	if (dev->update_ts <= dev->report_ts)
	{
		return;	
	}
	
	memset(rule_buf, 0x00, sizeof(rule_buf));
	hash_map_work(&dev->attr_map, hash_get_attr_val);
	strcat(rule_buf, "a00000002 + a00000001 + a00000003");
	printf("rule engine [%s]\n", rule_buf);
	struct expr *e = expr_create(rule_buf, strlen(rule_buf), &vars, user_funcs);
  	if (e == NULL) {
    	printf("Syntax error\n");
    	return;
  	}

  	float result = expr_eval(e);
  	printf("result: %f\n", result);

  	expr_destroy(e, &vars);
	return;
}

int dev_init(void)
{
    hash_map_init(&g_dev_map, 16, str_cmp, str_hash_func);
	INIT_LIST_HEAD(&g_dev_age_head);
	return 0;
}

