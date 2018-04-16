#ifndef __ECE_LIB_DEV_H_
#define __ECE_LIB_DEV_H_

#include "edge_pub.h"
#include "pdt.h"

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

extern void get_dev_oid(enum edge_access_type type, unsigned char *addr, unsigned long addr_len, char *oid);
extern int dev_init(void);

#endif
