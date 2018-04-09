#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "base_def.h"
#include "mqtt.h"

static void app_msg_cb(struct mosquitto *mosq, void *data, const struct mosquitto_message *msg)
{
    if (msg->payloadlen) {
        printf("get mqtt message topic:%s oth:%s\n", msg->topic, (char *)msg->payload);

        /* execute commands */
        if (!strncmp((char *)msg->topic, "cmd", strlen((char *)msg->topic)))
            printf("%s (null)\n", msg->topic);
        else if (!strncmp((char *)msg->topic, "edge", strlen((char *)msg->topic)))
            printf("%s (null)\n", msg->topic);
    } 

#if 0
    else {
        printf("%s (null)\n", msg->topic);
    }
#endif
    return;
}

static void app_cnt_cb(struct mosquitto *mosq, void *data, int res)
{
    if (!res) {
        /* Subscribe to broker information topics on successful connect. */
        mosquitto_subscribe(mosq, NULL, "pkt", 2);
    } else {
        printf("mosquitto connect failed\n");
    }
    return;
}

static void *app_mqtt_main(void *arg)
{
    (void)mqtt_main_process(app_msg_cb, app_cnt_cb);
    return NULL;
}

static int app_run_mqtt(void)
{
    pthread_t thread;

    if (0 != pthread_create(&thread, NULL, app_mqtt_main, NULL))
        return 1;
    return 0;
}

int
main()
{
    app_printf("app begin");

    /* initcalls */
    mosquitto_lib_init();

    /* functions */
    (void)app_run_mqtt();

    while (1);

    return 0;
}
