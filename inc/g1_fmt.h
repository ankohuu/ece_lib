#ifndef __ECE_LIB_G1_FMT_H_
#define __ECE_LIB_G1_FMT_H_

#include "attr.h"

struct edge_pdt;

struct g1_token
{
    unsigned int topic;
    unsigned long offset;
    unsigned long len;
	struct g1_token *next;
};

struct g1_fmt
{
    unsigned int key;
    struct edge_pdt *pdt;
    unsigned long size;
	unsigned char pkt[128];
    unsigned long pkt_len;
	unsigned long offset;
	unsigned long len;
    unsigned long token_num;
    struct g1_token *token;
    /* todo:cmd arg */
};

extern struct g1_token * add_g1_token(unsigned int topic, unsigned int key, unsigned int token_topic, enum edge_attr_type type, unsigned long offset, unsigned long len);
extern void del_g1_token(unsigned int topic, unsigned int key, unsigned int token_topic);
extern struct g1_fmt * add_g1_fmt(unsigned int topic, unsigned int key, unsigned long size, unsigned char *pkt, unsigned long pkt_len, unsigned long offset, unsigned long len);
extern struct g1_fmt * get_g1_fmt(unsigned int topic, unsigned int key);
extern void del_g1_fmt(unsigned int topic, unsigned int key);
extern void hash_del_fmt(hash_map *map, void *key, void *data);
extern void show_g1_fmt_map(hash_map *map);

#endif
