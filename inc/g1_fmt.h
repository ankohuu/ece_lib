#ifndef __ECE_LIB_G1_FMT_H_
#define __ECE_LIB_G1_FMT_H_

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
	#if 0
    unsigned char *pkt;
    unsigned long size;
	#endif
    unsigned long token_num;
    struct g1_token *token;
    /* todo:cmd arg */
};

extern struct g1_fmt * add_g1_fmt(unsigned int topic, unsigned int key);
extern struct g1_fmt * get_g1_fmt(struct edge_pdt *pdt, unsigned int key);
extern void del_g1_fmt(unsigned int topic, unsigned int key);
extern void show_g1_fmt_map(hash_map *map);

#endif
