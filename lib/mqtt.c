#include <stdio.h>

#include "base_def.h"
#include "mqtt.h"

void mqtt_main_process(msg_cb msg, cnt_cb cnt)
{
    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        printf("mosquitto create client failed..\n");
        return;
    }

    mosquitto_connect_callback_set(mosq, cnt);
    mosquitto_message_callback_set(mosq, msg);

    if (mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE)) {
        printf("mosquitto unable to connect.\n");
        return;
    }

    mosquitto_loop_forever(mosq, -1, 1);
    mosquitto_destroy(mosq);
    return;
}

static void on_publish(struct mosquitto *mosq, void *userdata, int mid)
{
    mosquitto_disconnect(mosq);
}

void send_mqtt_msg(char *topic, void *data, unsigned long len)
{
    struct mosquitto *mosq;

    mosq = mosquitto_new(NULL, true, NULL);
    mosquitto_publish_callback_set(mosq, on_publish);
    if (mosquitto_connect(mosq, HOST, PORT, 60)) {
        printf("mosquitto unable to connect.\n");
    }
    mosquitto_publish(mosq, 0, topic, len, data, 0, 0);
    mosquitto_loop_forever(mosq, -1, 1);
    mosquitto_destroy(mosq); 
    return;
}

