#include <stdio.h>
#include <pthread.h>

#include "base_def.h"
#include "lib_mqtt.h"
#include "mgt.h"

static void do_mgt_periodic(void)
{
    return;
}

static void mgt_msg_cb(struct mosquitto *mosq, void *data, const struct mosquitto_message *msg)
{
    if (msg->payloadlen) {
        printf("get mqtt message topic:%s oth:%s\n", msg->topic, (char *)msg->payload);
        /* execute commands */
        #if 0
        if (!strncmp((char *)msg->topic, "cmd", strlen((char *)msg->topic)))
            exec_cmd((char *)msg->payload);
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
        mosquitto_subscribe(mosq, NULL, "edge-client-wloc", 2);
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

int mgt_init(void)
{
    pthread_t thread;
    if (0 != pthread_create(&thread, NULL, mgt_main, NULL))
        return 1;
    return 0;
}

