#ifndef __ECE_LIB_ATTR_H_
#define __ECE_LIB_ATTR_H_

enum edge_attr_type
{
    EDGE_ATTR_NUM,
    EDGE_ATTR_STR,
    EDGE_ATTR_BUTT,
};

struct edge_attr
{
	unsigned int topic;
    enum edge_attr_type type;
    unsigned long len;
    unsigned long ref;
};

extern struct edge_attr *add_attr(unsigned int topic, enum edge_attr_type type, unsigned long len);
extern void del_attr(unsigned int topic);
extern int attr_init(void);

#endif
