#ifndef __ECE_LIB_G1_H_
#define __ECE_LIB_G1_H_

#include "edge_pub.h"

struct g1_header
{
    unsigned char msg_type;
    unsigned char addr_ctrl;
    unsigned char addr[0];
} __attribute__ ((packed));

struct g1_msg
{
    unsigned int topic;
    unsigned char did;
    unsigned char len;
    unsigned char data[0];
} __attribute__ ((packed));

struct pkt_info
{
    time_t ts;
	unsigned int module;
	enum edge_access_type type;
	unsigned char *pkt;
	unsigned long len;

    unsigned long msg_type;
	unsigned long addr_ctrl;
    
    unsigned char *h1;
    unsigned long h1_len;
    unsigned char *h2;
    unsigned long h2_len;
    unsigned char *h3;
    unsigned long h3_len;

	unsigned int key;
	unsigned long did;
    unsigned long payload_len;
    unsigned char *payload;

};

#endif
