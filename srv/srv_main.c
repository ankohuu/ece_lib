#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "base_def.h"
#include "lib_mqtt.h"
#include "lib_misc.h"
#include "mgt.h"

extern char *edge_msg[EDGE_PRO_BUTT];
struct edge_mgt_control client_ctl = {.status = EDGE_STATUS_ONLINE, .function = EDGE_FUNC_ON};
struct edge_mgt_stat client_stat;

void srv_send_edge_msg(unsigned char *msg, unsigned long len)
{
    send_mqtt_msg(MQTT_EDGE_LIB_WLOC_TOPIC, (void *)msg, len);
    return;
}

void rcv_edge_msg(unsigned char *msg, unsigned long len)
{
    struct edge_mgt_tlv *tlv = (struct edge_mgt_tlv *)msg;

    if (EDGE_STATUS_OFFLINE == client_ctl.status)
        return;

    if (len < sizeof(*tlv))
        return;
    len -= sizeof(*tlv);
    srv_printf("edge server get [%s] msg", edge_msg[tlv->type]);

    switch (tlv->type)
    {
        case EDGE_PRO_HELLO:
        {
            struct edge_mgt_control *ctl;
            memcpy(&client_stat, msg + sizeof(*tlv) + sizeof(client_ctl), sizeof(client_stat));
            srv_printf("hello from gateway[%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x] link service type:wloc",
                client_stat.mac[0],  
                client_stat.mac[1],  
                client_stat.mac[2],  
                client_stat.mac[3],  
                client_stat.mac[4],  
                client_stat.mac[5]  
            );

            tlv = malloc(sizeof(*tlv) + sizeof(*ctl));
            if (NULL == tlv)
                break;
            memset(tlv, 0, sizeof(*tlv) + sizeof(*ctl));
            tlv->type = EDGE_PRO_ACK;
            tlv->len = sizeof(*ctl);
            ctl = (struct edge_mgt_control *)tlv->val;
            ctl->status = client_ctl.status;
            ctl->function = client_ctl.function;
            srv_send_edge_msg((unsigned char *)tlv, sizeof(*tlv) + sizeof(*ctl));
            free(tlv);
            break;
        }
        default:
            break;
    }

    return;
}

static void srv_msg_cb(struct mosquitto *mosq, void *data, const struct mosquitto_message *msg)
{

    if (msg->payloadlen) {
        if (!strncmp((char *)msg->topic, MQTT_OASIS_EDGE_SERVER_TOPIC, strlen((char *)msg->topic)))
            rcv_edge_msg(msg->payload, msg->payloadlen);
        /* execute commands */
#if 0
        if (!strncmp((char *)msg->topic, "cmd", strlen((char *)msg->topic)))
            printf("%s (null)\n", msg->topic);
        else if (!strncmp((char *)msg->topic, "edge", strlen((char *)msg->topic)))
            printf("%s (null)\n", msg->topic);
#endif
    }

#if 0
    else {
        printf("%s (null)\n", msg->topic);
    }
#endif
    return;
}

static void srv_cnt_cb(struct mosquitto *mosq, void *data, int res)
{
    if (!res) {
        /* Subscribe to broker information topics on successful connect. */
        mosquitto_subscribe(mosq, NULL, MQTT_OASIS_EDGE_SERVER_TOPIC, 2);
    } else {
        printf("mosquitto connect failed\n");
    }
    return;
}

static void *srv_mqtt_main(void *arg)
{
    srv_printf("oasis server init");
    (void)mqtt_main_process(srv_msg_cb, srv_cnt_cb);
    return NULL;
}

int srv_init(void)
{
    pthread_t thread;

    if (0 != pthread_create(&thread, NULL, srv_mqtt_main, NULL))
        return 1;
    return 0;
}

