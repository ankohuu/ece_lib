#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "base_def.h"
#include "lib_mqtt.h"

static void srv_msg_cb(struct mosquitto *mosq, void *data, const struct mosquitto_message *msg)
{
    if (msg->payloadlen) {
        printf("srv get mqtt message topic:%s oth:%s\n", msg->topic, (char *)msg->payload);

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
        mosquitto_subscribe(mosq, NULL, "oasis-edge-server", 2);
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

