#ifndef __ECE_EDGE_PUB_H_
#define __ECE_EDGE_PUB_H_

enum edge_access_type
{
    ACCESS_WLOC,
    ACCESS_LORA,
    ACCESS_BUTT,
};

extern int edge_init(void);
/* data path entry */
extern long edge_rcv_pkt(enum edge_access_type type, unsigned long module, unsigned char *pkt, unsigned long size);

#endif
