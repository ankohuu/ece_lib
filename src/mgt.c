#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

#include "base_def.h"
#include "lib_mqtt.h"
#include "mgt.h"
#include "edge_pub.h"

struct edge_mgt_control g_edge_mgt_ctl = {EDGE_STATUS_OFFLINE, EDGE_FUNC_ON, 20, 3};
static struct edge_mgt_stat g_edge_mgt_stat;

char *edge_msg[EDGE_PRO_BUTT] = {"Hello", "Acknowledge"};

static void mgt_send_msg(unsigned char *msg, unsigned long len)
{
    send_mqtt_msg(MQTT_OASIS_EDGE_SERVER_TOPIC, (void *)msg, len);
    return;
}

static unsigned long mgt_send_hello_msg(void)
{
    struct edge_mgt_tlv *tlv; 
    struct edge_mgt_control *ctl;
    struct edge_mgt_stat *stat;
    
    tlv = malloc(sizeof(*tlv) + sizeof(*ctl) + sizeof(*stat));
    if (NULL == tlv)
        return 1;

    tlv->type = EDGE_PRO_HELLO;
    tlv->len = sizeof(*ctl) + sizeof(*stat);
    ctl = (struct edge_mgt_control *)tlv->val;
    ctl->status = g_edge_mgt_ctl.status;
    ctl->function = g_edge_mgt_ctl.function;
    stat = (struct edge_mgt_stat *)(ctl + 1);
    memcpy(stat->mac, g_edge_mgt_stat.mac, 6);
    stat->link_type = g_edge_mgt_stat.link_type;

    mgt_send_msg((unsigned char *)tlv, sizeof(*tlv) + sizeof(*ctl) + sizeof(*stat));

    if (EDGE_STATUS_ONLINE == g_edge_mgt_ctl.status)
        g_edge_mgt_stat.timeout++;
    
    free(tlv);
    return 0;
}

static unsigned long mgt_rcv_msg(unsigned char *msg, unsigned long len)
{
    struct edge_mgt_tlv *tlv = (struct edge_mgt_tlv *)msg;

    if (len < sizeof(*tlv))
        return 1;

    g_edge_mgt_stat.timeout = 0;
    msg += sizeof(*tlv);
    len -= sizeof(*tlv);
    lib_printf("edge get [%s] msg", edge_msg[tlv->type]);

    switch (tlv->type)
    {
        case EDGE_PRO_ACK:
        {
            struct edge_mgt_control *ctl = (struct edge_mgt_control *)msg;
            if (len < sizeof(struct edge_mgt_control))
                return 1;
            if (g_edge_mgt_ctl.status != EDGE_STATUS_ONLINE)
                lib_printf("edge client goes online");
            g_edge_mgt_ctl.status = EDGE_STATUS_ONLINE;
            if (ctl->function != g_edge_mgt_ctl.function) {
                lib_printf("edge client function is %s", (EDGE_FUNC_ON == ctl->function)?"on":"off");
                g_edge_mgt_ctl.function = ctl->function;
            }
            break;
        }
        default:
            break;
    }

    return 0;
}

static void do_mgt_periodic(void)
{
    static unsigned long tick = 0;

    if (0 == tick%g_edge_mgt_ctl.hello_interval)
        mgt_send_hello_msg();

    if (g_edge_mgt_stat.timeout >= g_edge_mgt_ctl.timeout_num 
        && EDGE_STATUS_ONLINE == g_edge_mgt_ctl.status) {
        g_edge_mgt_ctl.status = EDGE_STATUS_OFFLINE;
        lib_printf("edge client goes offline");
    }

    tick++;
    return;
}

static void mgt_msg_cb(struct mosquitto *mosq, void *data, const struct mosquitto_message *msg)
{
    if (msg->payloadlen) {
        if (!strncmp((char *)msg->topic, MQTT_EDGE_LIB_WLOC_TOPIC, strlen((char *)msg->topic)))
            mgt_rcv_msg((unsigned char *)msg->payload, msg->payloadlen);

        /* execute commands */
        #if 0
        else if (!strncmp((char *)msg->topic, "edge", strlen((char *)msg->topic)))
            edge_rule_engine((char *)msg->payload, msg->payloadlen);
        #endif
        //json_doit((char *)msg->payload); 
    } else {
        printf("%s (null)\n", msg->topic);
    }
    //fflush(stdout);
    return;
}

static void mgt_cnt_cb(struct mosquitto *mosq, void *data, int res)
{
    //printf("msg cnt cb.\r\n");
    if (!res) {
        /* Subscribe to broker information topics on successful connect. */
        mosquitto_subscribe(mosq, NULL, MQTT_EDGE_LIB_WLOC_TOPIC, 2);
    } else {
        printf("mosquitto connect failed\n");
    }
    return;
}

static void *mgt_main(void *arg)
{
    int ret = 0;
    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        printf("mosquitto create client failed..\n");
        mosquitto_lib_cleanup();
        return NULL;
    }

    mosquitto_connect_callback_set(mosq, mgt_cnt_cb);
    mosquitto_message_callback_set(mosq, mgt_msg_cb);

    if (mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE)) {
        printf("mosquitto unable to connect.\n");
        return NULL;
    }

    while (1) {
        ret = mosquitto_loop(mosq, 1000, 1);
        if (0 == ret) {
            do_mgt_periodic();
        }
    }
    mosquitto_destroy(mosq);
    return NULL;
}

static void get_device_mac(unsigned char *mac)
{
    struct ifreq ifreq;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > sock) {
        perror("app create socket error\r\n");
        close(sock);
        return;
    }
    strcpy(ifreq.ifr_name, "eth0");
    if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        perror("app socket ioctl error\r\n");
        close(sock);
        return;
    }
    memcpy(mac, ifreq.ifr_hwaddr.sa_data, 6);

#if 0
    int i = 0;
    for(i = 0; i < 6; i++){
        printf("%2x ", mac[i]);
    }
    printf("\n");
#endif
    close(sock);
    return;
}

int mgt_init(void)
{
    pthread_t thread;

    /* init status */
    memset(&g_edge_mgt_stat, 0x00, sizeof(g_edge_mgt_stat));
    get_device_mac(g_edge_mgt_stat.mac);
    g_edge_mgt_stat.link_type = ACCESS_WLOC;

    if (0 != pthread_create(&thread, NULL, mgt_main, NULL))
        return 1;
    return 0;
}

