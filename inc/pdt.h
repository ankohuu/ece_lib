#ifndef __ECE_LIB_PDT_H_
#define __ECE_LIB_PDT_H_

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
};

extern int pdt_init(void);

#endif
