#ifndef __ECE_LIB_PDT_H_
#define __ECE_LIB_PDT_H_

#include "hash_map.h"
#include "list.h"

enum edge_pdt_endian
{
    EDGE_BIG_ENDIAN = 1,
    EDGE_SMALL_ENDIAN,
};

struct edge_pdt
{
	unsigned int topic;
    unsigned long endian;
    //struct g1_fmt_set gfs;
    struct list_head head;
    hash_map attr_map;
};

struct pdt_attr
{
    unsigned long ref;
    unsigned int topic;
};

extern struct edge_pdt *add_pdt(unsigned int topic, enum edge_pdt_endian endian);
extern void del_pdt(unsigned int topic);
extern int pdt_init(void);

#endif
