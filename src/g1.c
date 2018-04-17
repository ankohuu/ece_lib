#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "base_def.h"
#include "mgt.h"
#include "dev.h"
#include "g1_fmt.h"
#include "g1.h"

static unsigned char h1_len_tbl[] = {6, 8, 16, 20, 0, 4, 0, 0};
static unsigned char h2_len_tbl[] = {0, 2, 4, 8};
static unsigned char h3_len_tbl[] = {0, 2, 4, 8};

extern pthread_rwlock_t g_edge_rwlock;

static inline unsigned long parse_msg(struct dev *dev, struct pkt_info *info, 
										  unsigned char *pkt, unsigned long size)
{
	struct g1_fmt *fmt;
    struct g1_msg *msg = (struct g1_msg *)pkt;
	struct g1_token *token;
	unsigned long len, i;
    char log[256];

    if (sizeof(struct g1_msg) > size)
		return G1_STAT_ERR_PKT;

	len = size - sizeof(struct g1_msg);
	info->key = ntohl(msg->topic);
	info->did = msg->did;
	info->payload_len = (len >= msg->len)?msg->len:len;
	info->payload = msg->data;

	lib_printf("key:0x%8.8x did:%lu payload len:%lu payload:%p", 
			info->key, info->did, info->payload_len, info->payload);
	memset(log, 0x00, sizeof(256));
	for (i=0; i<info->payload_len; i++)
		sprintf(log, "%s %2.2x", log, msg->data[i]);
	lib_printf("payload:%s", log + 1);

	fmt = get_g1_fmt(dev->pdt->topic, info->key);
	if (NULL != fmt)
		return G1_STAT_NO_PKT_FMT;
	token = fmt->token;
	while (token) {
		lib_printf("token device topic[%x] offset:%lu len:%lu",
				token->topic, token->offset, token->len);	
        (void)upt_dev_attr(dev, token->topic, info->payload + token->offset, token->len);
		token = token->next;
	}
	return 0;
}

static inline unsigned long parse_address_filed(struct pkt_info *info, unsigned char addr_ctrl, 
    						        unsigned char *addr, unsigned long size)
{
    unsigned long type = (addr_ctrl & 0xC0) >> 6;
    unsigned long len;

    if (unlikely(1 < type))
        return 0;

    if (unlikely(1 == type && 1 != (addr_ctrl & 0x3)))
        return 0;
	
    info->h1_len = h1_len_tbl[(addr_ctrl & 0x3) + 4 * type];
    info->h1 = addr;
	
    info->h2_len = h2_len_tbl[(addr_ctrl & 0xC) >> 2];
	if (info->h2_len)
    	info->h2 = info->h1 + info->h1_len;
	
    info->h3_len = h3_len_tbl[(addr_ctrl & 0x30) >> 4];
	if (info->h3_len)
    	info->h3 = info->h2 + info->h2_len;
    len = info->h1_len + info->h2_len + info->h3_len;
    return (size > len)?len:0;
}

static long parse_g1_pkt(struct pkt_info *info)
{
	struct dev *dev;
	unsigned long l, len = 0;
	struct g1_header *header;
	char log[256];
	
	lib_printf("edge begin to parse pkt");

	/* type/ctrl/addr g1 header */
	header = (struct g1_header *)info->pkt;
	info->msg_type = header->msg_type;
	info->addr_ctrl = header->addr_ctrl;
	len += sizeof(struct g1_header);

	/* address */
	l = parse_address_filed(info, header->addr_ctrl, header->addr, info->len - len);
    if (unlikely(0 == l)) {
		lib_printf("wrong addr filed, drop it");
        return G1_STAT_ERR_PKT;
    }
    len += l;

	lib_printf("link type:%s", (ACCESS_WLOC == info->type)?"WLOC":"known");
	memset(log, 0x00, 256);
	for(l=0; l<info->h1_len; l++) {
        sprintf(log, "%s-%2.2x", log, info->h1[l]);
    }
	lib_printf("device l1 address:%s", log + 1);
	if (0 != info->h2_len) {
		memset(log, 0x00, 256);
		for(l=0; l<info->h2_len; l++) {
        	sprintf(log, "%s-%2.2x", log, info->h2[l]);
    	}
		lib_printf("device l2 address:%s", log + 1);	
	}
	if (0 != info->h3_len) {
		memset(log, 0x00, 256);
		for(l=0; l<info->h3_len; l++) {
        	sprintf(log, "%s-%2.2x", log, info->h3[l]);
    	}
		lib_printf("device l3 address:%s", log + 1);	
	}

	dev = get_dev(info->type, info->h1, info->h1_len + info->h2_len + info->h3_len);
	if (NULL == dev)
	{
		return G1_STAT_NO_DEVICE;
	}
	fresh_dev(dev);

	/* message */
	return parse_msg(dev, info, info->pkt + len, info->len - len);
}

static long edge_dp_rcv(enum edge_access_type type, unsigned long module, unsigned char *pkt, unsigned long size)
{
	struct pkt_info info;

	/* record */


	/* parse */
	memset(&info, 0x00, sizeof(info));

	info.ts = time(NULL);
	info.module = module;
	info.type = type;
	info.pkt = pkt;
	info.len = size;

    return parse_g1_pkt(&info);
}

long edge_rcv_pkt(enum edge_access_type type, unsigned long module, unsigned char *pkt, unsigned long size)
{
    long lret;

    if (EDGE_FUNC_ON != get_edge_function())
        return 1;

    if (NULL == pkt || 0 == size)
        return 1;

    pthread_rwlock_rdlock(&g_edge_rwlock);
    lret = edge_dp_rcv(type, module, pkt, size);    
    pthread_rwlock_unlock(&g_edge_rwlock);

    return lret;
}
