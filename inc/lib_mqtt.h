#ifndef __ECE_LIB_MQTT_H_
#define __ECE_LIB_MQTT_H_

#include <mosquitto.h>

#define MQTT_EDGE_LIB_WLOC_TOPIC "edge-client-wloc"
#define MQTT_OASIS_EDGE_SERVER_TOPIC "oasis-edge-server"
#define MQTT_OASIS_APP "oasis-app"
#define HOST "localhost"
#define PORT  1883
#define KEEP_ALIVE 60

typedef void (*msg_cb)(struct mosquitto *mosq, void *data, const struct mosquitto_message *msg);
typedef void (*cnt_cb)(struct mosquitto *mosq, void *data, int res);
extern void mqtt_main_process(msg_cb msg, cnt_cb cnt);
extern void send_mqtt_msg(char *topic, void *data, unsigned long len);

#endif
