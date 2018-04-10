#ifndef __ECE_MGT_H_
#define __ECE_MGT_H_

enum edge_mgt_status
{
    EDGE_STATUS_OFFLINE,
    EDGE_STATUS_ONLINE,
};

enum edge_mgt_function
{
    EDGE_FUNC_OFF,
    EDGE_FUNC_ON,
};

struct edge_mgt_stat
{
    unsigned char mac[6];
    unsigned int link_type;
} __attribute__((packed));

struct edge_mgt_control
{
    unsigned int status;
    unsigned int function;
} __attribute__((packed));

enum edge_mgt_protocol_type
{
    EDGE_PRO_HELLO = 0,
    EDGE_PRO_ACK,
    EDGE_PRO_BUTT,
};

struct edge_mgt_tlv 
{
    unsigned int type;
    unsigned int len;
    unsigned char val[0];
} __attribute__((packed));

extern int mgt_init(void);

#endif
