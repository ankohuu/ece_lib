#ifndef __ECE_LIB_ATTR_H_
#define __ECE_LIB_ATTR_H_

enum edge_attr_type
{
    EDGE_ATTR_NUM,
    EDGE_ATTR_STR,
    EDGE_ATTR_BUTT,
};

struct attr_operations
{
	void (*to_value)(unsigned char *data, unsigned long len, void *res);
	void (*to_string)(unsigned char *data, unsigned long len, char *res);
};

struct edge_attr
{
	unsigned int topic;
    enum edge_attr_type type;
    unsigned long len;
    unsigned long ref;
};

extern struct attr_operations g_attr_operations_array[];
#define GET_ATTR_OPERATIONS(type) g_attr_operations_array[type]

extern struct edge_attr *get_attr(unsigned int topic);
extern struct edge_attr *add_attr(unsigned int topic, enum edge_attr_type type, unsigned long len);
extern void del_attr(unsigned int topic);
extern int attr_init(void);

#endif
