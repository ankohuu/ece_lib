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

enum edge_up_mode
{
    EDGE_UP_MODE_ATTR,
    EDGE_UP_MODE_ATTR_PERIODIC,
    EDGE_UP_MODE_PKT,
};

struct edge_mgt_stat
{
    unsigned char mac[6];
    unsigned int link_type;
    unsigned int timeout;
} __attribute__((packed));

struct edge_mgt_control
{
    unsigned int status;
    unsigned int function;
    unsigned int scenerio_id;
    unsigned int hello_interval;
    unsigned int timeout_num;
	unsigned int dev_age_interval;
	unsigned int up_mode;
	unsigned int up_interval;
	unsigned int pkt_record;
} __attribute__((packed));

struct edge_pkt
{
	unsigned int status;
    unsigned int module;
    unsigned char pkt[256];
    unsigned int len;
} __attribute__((packed));

enum edge_mgt_protocol_type
{
    EDGE_PRO_HELLO = 0,
    EDGE_PRO_ACK,
    EDGE_PRO_PDT_ADD,
    EDGE_PRO_PDT_DEL,
    EDGE_PRO_G1_FMT_ADD,
    EDGE_PRO_G1_FMT_DEL,
    EDGE_PRO_G1_TOKEN_ADD,
    EDGE_PRO_G1_TOKEN_DEL,
    EDGE_PRO_DEV_ADD,
    EDGE_PRO_DEV_DEL,
    EDGE_PRO_PKT,
    EDGE_PRO_BUTT,
};

struct edge_mgt_tlv 
{
    unsigned int type;
    unsigned int len;
    unsigned char val[0];
} __attribute__((packed));

struct edge_mgt_pdt_add {
    unsigned int topic;
    unsigned int endian;
} __attribute__((packed));

struct edge_mgt_pdt_del {
    unsigned int topic;
} __attribute__((packed));

struct edge_mgt_g1_fmt_add {
    unsigned int topic;
    unsigned int key;
} __attribute__((packed));

struct edge_mgt_g1_fmt_del {
    unsigned int topic;
	unsigned int key;
} __attribute__((packed));

struct edge_mgt_g1_token_add {
    unsigned int topic;
    unsigned int key;
	unsigned int token_topic;
	unsigned int type;
	unsigned int offset;
	unsigned int len;
} __attribute__((packed));

struct edge_mgt_g1_token_del {
    unsigned int topic;
    unsigned int key;
	unsigned int token_topic;
} __attribute__((packed));

struct edge_mgt_dev_add {
    unsigned char addr[64];
    unsigned int len;
    unsigned int link_type;
    unsigned int module;
	unsigned int topic;
} __attribute__((packed));

struct edge_mgt_dev_del {
    unsigned char addr[64];
    unsigned int len;
    unsigned int link_type;
} __attribute__((packed));

extern struct edge_mgt_control g_edge_mgt_ctl;

static inline unsigned int get_edge_function(void)
{
    return g_edge_mgt_ctl.function;
}

static inline unsigned int get_dev_age_interval(void)
{
    return g_edge_mgt_ctl.dev_age_interval;
}

static inline unsigned int get_dev_scenerio_id(void)
{
    return g_edge_mgt_ctl.scenerio_id;
}

static inline unsigned int get_pkt_up_mode(void)
{
	return g_edge_mgt_ctl.up_mode;
}

static inline unsigned int get_pkt_up_interval(void)
{
	return g_edge_mgt_ctl.up_interval;
}

static inline unsigned int get_pkt_record(void)
{
	return g_edge_mgt_ctl.pkt_record;
}

extern unsigned long mgt_send_pkt(unsigned int status, unsigned int module, unsigned char *pkt, unsigned int len);
extern int mgt_init(void);

#endif
