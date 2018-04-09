#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "base_def.h"
#include "lib_mqtt.h"
#include "lib_misc.h"
#include "edge_pub.h"

pthread_cond_t g_app_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t g_app_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned char *g_app_pkt;
unsigned long g_app_pkt_len;
unsigned long g_app_pkt_module;

#if 0
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
#endif

extern int cli_main(void);

static void *app_cli_main(void *arg)
{
    cli_main();
    return NULL;
}

static int app_run_cli(void)
{
    pthread_t thread;

    if (0 != pthread_create(&thread, NULL, app_cli_main, NULL))
        return 1;
    return 0;
}

static void app_run(void)
{
    long ret;

    while (1) {
        g_app_pkt = NULL;
        g_app_pkt_len = 0;
        g_app_pkt_module = 0;

        pthread_mutex_lock(&g_app_mutex);
        pthread_cond_wait(&g_app_cond, &g_app_mutex);
        if (NULL != g_app_pkt && 0 != g_app_pkt_len) {
            app_printf("receive a packet from device");
            print_pkt(g_app_pkt_module, g_app_pkt, g_app_pkt_len);
            ret = edge_rcv_pkt(ACCESS_WLOC, g_app_pkt_module, g_app_pkt, g_app_pkt_len);
            ret = ret;
        }
        pthread_mutex_unlock(&g_app_mutex);
    }
    return;
}

int
main()
{
    app_printf("app begin");

    /* initcalls */
    mosquitto_lib_init();

    /* functions */
#if 0
    (void)app_run_mqtt();
#endif

    (void)app_run_cli();

    app_run();

    return 0;
}
