#ifndef __ECE_LIB_DEV_H_
#define __ECE_LIB_DEV_H_

#include "edge_pub.h"
#include "pdt.h"

#define MAX_DEVICE_ATTRIBUTE         16
#define MAX_DEVICE_ATTRIBUTE_LEN     32
#define MAX_DEVICE_ATTRIBUTE_STR_LEN 32

struct dev
{
	char oid[64];
    unsigned long module;

	hash_map attr_map;

	time_t ts;
    time_t update_ts;
    time_t report_ts;

	struct list_node list;
    struct list_node age_list;
    struct edge_pdt *pdt;
};

struct attr_val
{
    unsigned long len;
    unsigned char val[MAX_DEVICE_ATTRIBUTE_LEN];
    char str[MAX_DEVICE_ATTRIBUTE_STR_LEN];
    unsigned int topic;
	struct dev *dev;
};

extern struct dev *add_dev(unsigned short module, unsigned char* addr, unsigned int addr_len, enum edge_access_type type, unsigned int topic);
extern void del_dev(enum edge_access_type type, unsigned char *addr, unsigned long addr_len);
extern struct attr_val * upt_dev_attr(struct dev *dev, unsigned int topic, unsigned char *val, unsigned long len);
extern struct dev *get_dev(enum edge_access_type type, unsigned char *addr, unsigned long addr_len);
extern void show_dev(struct dev *dev);
extern void show_all_dev(void);
extern void get_dev_oid(enum edge_access_type type, unsigned char *addr, unsigned long addr_len, char *oid);
extern int dev_init(void);
extern void do_device_ageing(unsigned long sec);
extern void fresh_dev(struct dev *dev);
extern void report_dev_attr(struct dev *dev, struct attr_val *attr);
extern void do_device_update(unsigned long sec);

#endif
