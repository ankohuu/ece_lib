#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mgt.h"
#include "edge_pub.h"

extern pthread_rwlock_t g_edge_rwlock;

static long edge_dp_rcv(enum edge_access_type type, unsigned long module, unsigned char *pkt, unsigned long size)
{
    return 0;
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
