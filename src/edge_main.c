#include <stdio.h>

#include "base_def.h"
#include "edge_pub.h"
#include "mgt.h"

int edge_init(void)
{
    lib_printf("edge lib init");
    (void)mgt_init();
    return 0;
}

