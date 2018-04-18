#ifndef __ECE_LIB_G1_H_
#define __ECE_LIB_G1_H_

#include "edge_pub.h"

enum g1_process_status
{
    G1_STAT_ERR_PKT,
    G1_STAT_NO_DEVICE,
    G1_STAT_NO_PKT_FMT,
    G1_STAT_RAW_PKT,
    G1_STAT_BUTT,
};

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

struct g1_pkt_in_file
{
    unsigned int module;
	unsigned int access_type;
	unsigned int len;
	unsigned char *pkt;
} __attribute__((packed));

#define G1_PKT_RECORD_FILE "/tmp/pkt_db"
#define G1_PKT_FILE_SIZE 2*1024*1024

#endif
