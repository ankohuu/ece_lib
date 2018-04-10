#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "base_def.h"
#include "lib_cli.h"
#include "lib_misc.h"
#include "pdt.h"
#include "mgt.h"

extern pthread_cond_t g_app_cond;
extern pthread_mutex_t g_app_mutex;
extern unsigned char *g_app_pkt;
extern unsigned long g_app_pkt_len;
extern unsigned long g_app_pkt_module;
static unsigned char g_app_file_pkt[512];
extern struct edge_mgt_control client_ctl;
extern struct edge_mgt_control g_edge_mgt_ctl;

#define CLITEST_PORT                8000
#define MODE_CONFIG_INT             10
#define MODE_CONFIG_WLOC            20
#define MODE_CONFIG_SERVER          21
#define MODE_CONFIG_EDGE            23

unsigned int regular_count = 0;
unsigned int debug_regular = 0;

struct my_context {
  int value;
  char* message;
};

int cmd_test(struct cli_def *cli, const char *command, char *argv[], int argc)
{
    int i;
    cli_print(cli, "called %s with \"%s\"", __func__, command);
    cli_print(cli, "%d arguments:", argc);
    for (i = 0; i < argc; i++)
        cli_print(cli, "        %s", argv[i]);

    return CLI_OK;
}

int cmd_set(struct cli_def *cli, UNUSED(const char *command), char *argv[],
    int argc)
{
    if (argc < 2 || strcmp(argv[0], "?") == 0)
    {
        cli_print(cli, "Specify a variable to set");
        return CLI_OK;
    }

    if (strcmp(argv[1], "?") == 0)
    {
        cli_print(cli, "Specify a value");
        return CLI_OK;
    }

    if (strcmp(argv[0], "regular_interval") == 0)
    {
        unsigned int sec = 0;
        if (!argv[1] && !&argv[1])
        {
            cli_print(cli, "Specify a regular callback interval in seconds");
            return CLI_OK;
        }
        sscanf(argv[1], "%u", &sec);
        if (sec < 1)
        {
            cli_print(cli, "Specify a regular callback interval in seconds");
            return CLI_OK;
        }
        cli->timeout_tm.tv_sec = sec;
        cli->timeout_tm.tv_usec = 0;
        cli_print(cli, "Regular callback interval is now %d seconds", sec);
        return CLI_OK;
    }

    cli_print(cli, "Setting \"%s\" to \"%s\"", argv[0], argv[1]);
    return CLI_OK;
}

int cmd_config_int(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    if (argc < 1)
    {
        cli_print(cli, "Specify an interface to configure");
        return CLI_OK;
    }

    if (strcmp(argv[0], "?") == 0)
        cli_print(cli, "  test0/0");
    else if (strcasecmp(argv[0], "test0/0") == 0)
        cli_set_configmode(cli, MODE_CONFIG_INT, "test");
    else
        cli_print(cli, "Unknown interface %s", argv[0]);

    return CLI_OK;
}

int cmd_config_wloc(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    cli_set_configmode(cli, MODE_CONFIG_WLOC, "wloc");
    return CLI_OK;
}

struct pkt
{
    unsigned long module;
    unsigned long len;
    unsigned char data[128];
};

//static char pkt_file_path[] = "./doc/pkt";

static struct pkt pkt_array[] = {
    {0, 10, {1,2,3,4,5,6,7,8,9,10}},
    {1, 15, {0x83,0x41,0x20,0xc7,0xdd,0x11,0x10,0x00,0x00,0x02,0x1,0x3,0x0,0xa,0x0}},
    {2, 33, {
    1,2,3,4,5,6,7,8,9,10,
    1,2,3,4,5,6,7,8,9,10,
    1,2,3,4,5,6,7,8,9,10,1,9,8
    }},
};

int cmd_wloc_rcv_builtin_pkt(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    unsigned long max = sizeof(pkt_array)/sizeof(struct pkt) - 1;
    unsigned long num = -1;

    if (argc != 1)
    {
        cli_print(cli, "packet num <0-%lu>", max);
        return CLI_OK;
    }

    if (strcmp(argv[0], "?") == 0)
        cli_print(cli, "packet num <0-%lu>", max);
    else {
        sscanf(argv[0], "%lu", &num);
        if (num > max)
            cli_print(cli, "something wrong with packet num");
        else {
            pthread_mutex_lock(&g_app_mutex);
            g_app_pkt_len = pkt_array[num].len;
            g_app_pkt_module = pkt_array[num].module;
            g_app_pkt = pkt_array[num].data;
            pthread_cond_signal(&g_app_cond);
            pthread_mutex_unlock(&g_app_mutex);
        }
    }

    return CLI_OK;
}

int cmd_wloc_rcv_file_pkt(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    char *default_path = "./doc/one_pkt";
    char *path = NULL;
    int len;
    FILE *fp;

    if (argc >= 1 && strcmp(argv[0], "?") == 0) {
        cli_print(cli, "packet file path /xx/yy/zz");
        return CLI_OK;
    }

    if (argc != 1)
    {
        cli_print(cli, "use pkt file %s", default_path);
        path = default_path;
    } else
        path = argv[0];

    fp = fopen(path, "rb");
    if (NULL == fp) {
        cli_print(cli, "wrong path %s", path);
        cli_print(cli, "packet file path /xx/yy/zz");
        return CLI_OK;
    }

    memset(g_app_file_pkt, 0, sizeof(g_app_file_pkt));
    len = fread(g_app_file_pkt, sizeof(char), sizeof(g_app_file_pkt), fp);
    if (0 >= len) {
        fclose(fp);
        cli_print(cli, "packet file path /xx/yy/zz");
        return CLI_OK;
    }
    pthread_mutex_lock(&g_app_mutex);
    g_app_pkt_len = len;
    g_app_pkt_module = 3;
    g_app_pkt = g_app_file_pkt;
    pthread_cond_signal(&g_app_cond);
    pthread_mutex_unlock(&g_app_mutex);
    fclose(fp);

    return CLI_OK;
}

int cmd_wloc_rcv_cli_pkt(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    int len;

    if (argc != 1 || strcmp(argv[0], "?") == 0) {
        cli_print(cli, "packet hex context");
        return CLI_OK;
    }

    memset(g_app_file_pkt, 0, sizeof(g_app_file_pkt));
    len = ch_to_hex(argv[0], g_app_file_pkt);
    if (0 == len) {
        cli_print(cli, "packet hex context");
        return CLI_OK;
    }

    pthread_mutex_lock(&g_app_mutex);
    g_app_pkt_len = len;
    g_app_pkt_module = 4;
    g_app_pkt = g_app_file_pkt;
    pthread_cond_signal(&g_app_cond);
    pthread_mutex_unlock(&g_app_mutex);
    return CLI_OK;
}


int cmd_config_server(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    cli_set_configmode(cli, MODE_CONFIG_SERVER, "server");
    return CLI_OK;
}

extern void srv_add_pdt(unsigned int topic, enum edge_pdt_endian endian);
int cmd_server_add_pdt(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
	unsigned int topic, len, num = 0;
    if ((argc >= 1 && strcmp(argv[0], "?") == 0) || argc != 2)
    {
        cli_print(cli, "Usage:topic[hex] endian[1-big 2-small]");
        return CLI_OK;
    }

	len = ch_to_hex(argv[0], (unsigned char *)&topic);
    if (4 != len)
        return CLI_OK;
	topic = ntohl(topic);
	sscanf(argv[1], "%u", &num);
    if (num > 2 || num == 0)
    	return CLI_OK;        
	//printf("topic:%x endian:%u\r\n", topic, num);
	srv_add_pdt(topic, num);
    return CLI_OK;
}

extern void srv_del_pdt(unsigned int topic);
int cmd_server_del_pdt(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
	unsigned int topic, len;
    if ((argc >= 1 && strcmp(argv[0], "?") == 0) || argc != 1)
    {
        cli_print(cli, "Usage:topic[hex]");
        return CLI_OK;
    }

	len = ch_to_hex(argv[0], (unsigned char *)&topic);
    if (4 != len)
        return CLI_OK;
	topic = ntohl(topic);
	//printf("topic:%x\r\n", topic);
	srv_del_pdt(topic);
    return CLI_OK;
}

extern void show_all_pdt(void);
int cmd_server_show_pdt(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    show_all_pdt();
    return CLI_OK;
}

int cmd_config_edge(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    cli_set_configmode(cli, MODE_CONFIG_EDGE, "edge");
    return CLI_OK;
}

int cmd_edge_on(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    client_ctl.function = EDGE_FUNC_ON;
    return CLI_OK;
}

int cmd_edge_off(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    client_ctl.function = EDGE_FUNC_OFF;
    return CLI_OK;
}

int cmd_edge_hello(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    unsigned int num;
    if (argc != 1)
    {
        cli_print(cli, "hello interval:%u", g_edge_mgt_ctl.hello_interval);
        return CLI_OK;
    }

    if (strcmp(argv[0], "?") == 0)
        cli_print(cli, "int num");
    else {
        sscanf(argv[0], "%u", &num);
        if (num != 0) {
            g_edge_mgt_ctl.hello_interval = num;
        }
    }
    return CLI_OK;
}

int cmd_edge_timeout(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    unsigned int num;
    if (argc != 1)
    {
        cli_print(cli, "timeout number:%u", g_edge_mgt_ctl.timeout_num);
        return CLI_OK;
    }

    if (strcmp(argv[0], "?") == 0)
        cli_print(cli, "int num");
    else {
        sscanf(argv[0], "%u", &num);
        if (num != 0) {
            g_edge_mgt_ctl.timeout_num = num;
        }
    }
    return CLI_OK;
}

int cmd_edge_online(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    client_ctl.status = EDGE_STATUS_ONLINE;
    return CLI_OK;
}

int cmd_edge_offline(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    client_ctl.status = EDGE_STATUS_OFFLINE;
    return CLI_OK;
}

int cmd_config_int_exit(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
    cli_set_configmode(cli, MODE_CONFIG, NULL);
    return CLI_OK;
}

int cmd_show_regular(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    cli_print(cli, "cli_regular() has run %u times", regular_count);
    return CLI_OK;
}

int cmd_debug_regular(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    debug_regular = !debug_regular;
    cli_print(cli, "cli_regular() debugging is %s", debug_regular ? "enabled" : "disabled");
    return CLI_OK;
}

int cmd_context(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
    struct my_context *myctx = (struct my_context *)cli_get_context(cli);
    cli_print(cli, "User context has a value of %d and message saying %s", myctx->value, myctx->message);
    return CLI_OK;
}

int check_auth(const char *username, const char *password)
{
    if (strcasecmp(username, "fred") != 0)
        return CLI_ERROR;
    if (strcasecmp(password, "nerk") != 0)
        return CLI_ERROR;
    return CLI_OK;
}

int regular_callback(struct cli_def *cli)
{
    regular_count++;
    if (debug_regular)
    {
        cli_print(cli, "Regular callback - %u times so far", regular_count);
        cli_reprompt(cli);
    }
    return CLI_OK;
}

int check_enable(const char *password)
{
    return !strcasecmp(password, "topsecret");
}

int idle_timeout(struct cli_def *cli)
{
    cli_print(cli, "Custom idle timeout");
    return CLI_QUIT;
}

void pc(UNUSED(struct cli_def *cli), const char *string)
{
    printf("%s\n", string);
}

int cli_main()
{
    struct cli_command *c;
    struct cli_def *cli;
    int s, x;
    struct sockaddr_in addr;
    int on = 1;

    signal(SIGCHLD, SIG_IGN);

    // Prepare a small user context

#if 0
    char mymessage[] = "I contain user data!";
    struct my_context myctx;
    myctx.value = 5;
    myctx.message = mymessage;
#endif

    cli = cli_init();
    cli_set_banner(cli, "edge compute environment");
    cli_set_hostname(cli, "cli");
    cli_telnet_protocol(cli, 1);
    cli_set_idle_timeout_callback(cli, 120, idle_timeout); 

    cli_register_command(cli, NULL, "wloc", cmd_config_wloc, PRIVILEGE_UNPRIVILEGED, MODE_EXEC,
                         "Configure wloc process");
    c = cli_register_command(cli, NULL, "packet", NULL, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_WLOC, 
                         "Send/Receive packet");
    c = cli_register_command(cli, c, "recv", NULL, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_WLOC, 
                         "Receive packet");
    cli_register_command(cli, c, "built-in", cmd_wloc_rcv_builtin_pkt, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_WLOC, 
                         "Receive built-in packets");
    cli_register_command(cli, c, "file", cmd_wloc_rcv_file_pkt, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_WLOC, 
                         "Receive packet file");
    cli_register_command(cli, c, "cli", cmd_wloc_rcv_cli_pkt, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_WLOC, 
                         "Receive packets from cli");

    
    cli_register_command(cli, NULL, "server", cmd_config_server, PRIVILEGE_UNPRIVILEGED, MODE_EXEC,
                         "Configure edge server");
	c = cli_register_command(cli, NULL, "product", NULL, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_SERVER, 
                         "Product add/delete");
	cli_register_command(cli, c, "add", cmd_server_add_pdt, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_SERVER, 
                         "add product");
	cli_register_command(cli, c, "delete", cmd_server_del_pdt, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_SERVER, 
                         "delete product");
	cli_register_command(cli, c, "show", cmd_server_show_pdt, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_SERVER, 
                         "show all products");

    cli_register_command(cli, NULL, "edge", cmd_config_edge, PRIVILEGE_UNPRIVILEGED, MODE_EXEC,
                         "Configure edge client");
    c = cli_register_command(cli, NULL, "status", NULL, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_EDGE, 
                         "Online/Offline");
    cli_register_command(cli, c, "online", cmd_edge_online, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_EDGE, 
                         "Online");
    cli_register_command(cli, c, "offline", cmd_edge_offline, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_EDGE, 
                         "Offline");
    c = cli_register_command(cli, NULL, "function", NULL, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_EDGE, 
                         "On/Off edge compute");
    cli_register_command(cli, c, "on", cmd_edge_on, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_EDGE, 
                         "On");
    cli_register_command(cli, c, "off", cmd_edge_off, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_EDGE, 
                         "Off");
    cli_register_command(cli, NULL, "hello-interval", cmd_edge_hello, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_EDGE, 
                         "change hello packet interval");
    cli_register_command(cli, NULL, "timeout-num", cmd_edge_timeout, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG_EDGE, 
                         "change client timeout number");
#if 0
    cli_regular(cli, regular_callback);
    cli_regular_interval(cli, 5); // Defaults to 1 second
    cli_register_command(cli, NULL, "test", cmd_test, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);

    cli_register_command(cli, NULL, "simple", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);

    cli_register_command(cli, NULL, "simon", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);

    cli_register_command(cli, NULL, "set", cmd_set, PRIVILEGE_PRIVILEGED, MODE_EXEC, NULL);

    c = cli_register_command(cli, NULL, "show", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);

    cli_register_command(cli, c, "regular", cmd_show_regular, PRIVILEGE_UNPRIVILEGED, MODE_EXEC,
                         "Show the how many times cli_regular has run");

    cli_register_command(cli, c, "counters", cmd_test, PRIVILEGE_UNPRIVILEGED, MODE_EXEC,
                         "Show the counters that the system uses");

    cli_register_command(cli, c, "junk", cmd_test, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);

    cli_register_command(cli, NULL, "interface", cmd_config_int, PRIVILEGE_PRIVILEGED, MODE_EXEC,
                         "Configure an interface");

    cli_register_command(cli, NULL, "exit", cmd_config_int_exit, PRIVILEGE_PRIVILEGED, MODE_CONFIG_INT,
                         "Exit from interface configuration");

    cli_register_command(cli, NULL, "address", cmd_test, PRIVILEGE_PRIVILEGED, MODE_CONFIG_INT, "Set IP address");
    cli_register_command(cli, NULL, "addressa", cmd_test, PRIVILEGE_PRIVILEGED, MODE_CONFIG_INTA, "Set IP address");
    cli_register_command(cli, NULL, "addressb", cmd_test, PRIVILEGE_PRIVILEGED, MODE_CONFIG_INTB, "Set IP address");

    c = cli_register_command(cli, NULL, "debug", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);

    cli_register_command(cli, c, "regular", cmd_debug_regular, PRIVILEGE_UNPRIVILEGED, MODE_EXEC,
                         "Enable cli_regular() callback debugging");

    // Set user context and its command
    cli_set_context(cli, (void*)&myctx);
    cli_register_command(cli, NULL, "context", cmd_context, PRIVILEGE_UNPRIVILEGED, MODE_EXEC,
                         "Test a user-specified context");

    //cli_set_auth_callback(cli, check_auth);
    //cli_set_enable_callback(cli, check_enable);
    // Test reading from a file
    {
        FILE *fh;

        if ((fh = fopen("clitest.txt", "r")))
        {
            // This sets a callback which just displays the cli_print() text to stdout
            cli_print_callback(cli, pc);
            cli_file(cli, fh, PRIVILEGE_UNPRIVILEGED, MODE_EXEC);
            cli_print_callback(cli, NULL);
            fclose(fh);
        }
    }
#endif

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return 1;
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(CLITEST_PORT);
    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    if (listen(s, 50) < 0)
    {
        perror("listen");
        return 1;
    }

    //printf("cli listening on port %d\n", CLITEST_PORT);
    while ((x = accept(s, NULL, 0)))
    {
#if 0
        int pid = fork();
        if (pid < 0)
        {
            perror("fork");
            return 1;
        }

        /* parent */
        if (pid > 0)
        {
            socklen_t len = sizeof(addr);
            if (getpeername(x, (struct sockaddr *) &addr, &len) >= 0)
                printf(" * accepted connection from %s\n", inet_ntoa(addr.sin_addr));

            close(x);
            continue;
        }

        /* child */
        close(s);
        cli_loop(cli, x);
        exit(0);
#endif
        cli_loop(cli, x);
        close(x);
    }

    cli_done(cli);
    return 0;
}

