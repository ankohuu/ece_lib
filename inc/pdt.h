#ifndef __ECE_LIB_PDT_H_
#define __ECE_LIB_PDT_H_

#include "hash_map.h"
#include "list.h"
#include "attr.h"

enum edge_pdt_endian
{
    EDGE_BIG_ENDIAN = 1,
    EDGE_SMALL_ENDIAN,
};

struct edge_pdt
{
	unsigned int topic;
    unsigned long endian;
    struct list_head head;
    hash_map attr_map;
	hash_map fmt_map;
};

struct pdt_attr
{
    unsigned long ref;
    unsigned int topic;
};

extern struct edge_attr * add_pdt_attr(struct edge_pdt *pdt, unsigned int topic, enum edge_attr_type type, unsigned long len);
extern void del_pdt_attr(struct edge_pdt *pdt, unsigned int topic);
extern struct edge_pdt *add_pdt(unsigned int topic, enum edge_pdt_endian endian);
extern struct edge_pdt *get_pdt(unsigned int topic);
extern void del_pdt(unsigned int topic);
extern int pdt_init(void);

#endif
