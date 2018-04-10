
#include "mgt.h"
#include "edge_pub.h"

long edge_rcv_pkt(enum edge_access_type type, unsigned long module, unsigned char *pkt, unsigned long size)
{
    if (EDGE_FUNC_ON != get_edge_function())
        return 1;

    return 0;
}
