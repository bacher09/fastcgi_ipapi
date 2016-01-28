#include "geoip_api.h"
#include "http_utils.h"
#include "config.h"

#include "fcgi_config.h"
#include "fcgiapp.h"

#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

#if(__STDC_VERSION__ >= 20112L)
#include <stdnoreturn.h>
#endif

#ifdef WITH_CHROOT
#define USAGE_STRING "Usage: prog [-s socket-path] [-c chrootdir]"
#else
#define USAGE_STRING "Usage: prog [-s socket-path]"
#endif

#define BACKLOG 16
#define MAX_KEY 40
#define MAX_VALUE 300


static const char *not_found_str = (
    "Status: 404\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<html><body>Not Found<body></html>\n");

static const char *bad_request_str = (
    "Status: 400\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<html><body>Bad Request</body></html>\n");


struct doit_args {
    int thread_num;
    int socket;
};

struct handler {
    const char *path;
    void (*handler)();
};

void ip_handler(FCGX_Request*);
void user_agent_handler(FCGX_Request*);
void country_handler(FCGX_Request*);

const struct handler url_hanlders[] = {
    {"/ip", ip_handler},
    {"/user-agent", user_agent_handler},
    {"/country", country_handler},
    {NULL, NULL}
};

static void not_found404(FCGX_Request *request){
    FCGX_PutS(not_found_str, request->out);
}

static void bad_request(FCGX_Request *request) {
    FCGX_PutS(bad_request_str, request->out);
}

void ip_handler(FCGX_Request *request){
    char *ip_address = FCGX_GetParam("REMOTE_ADDR", request->envp);
    FCGX_PutS("Content-Type: text/plain\r\n\r\n", request->out);
    FCGX_PutS(ip_address, request->out);
}

void user_agent_handler(FCGX_Request *request){
    char *user_agent = FCGX_GetParam("HTTP_USER_AGENT", request->envp);
    FCGX_PutS("Content-Type: text/plain\r\n\r\n", request->out);
    FCGX_PutS(user_agent, request->out);
}

void country_handler(FCGX_Request *request){
#define MAX_IP 60
#define MAX_TYPE 10
    char key[MAX_KEY];
    char value[MAX_VALUE];
    char ip_address[MAX_IP] = {0};
    char type[MAX_TYPE] = {0};
    char *ip;
    query_param qp;
    int country_id;
    int i = 0;
    const char *country_code, *country_code3, *name, *xon_code;
    char *query_string = FCGX_GetParam("QUERY_STRING", request->envp);

    qp.key = key;
    qp.value = value;
    qp.max_key = MAX_KEY;
    qp.max_value = MAX_VALUE;

    while(iterate_qs(&qp, &i, query_string)) {
        if(qp.key_len >= MAX_KEY || qp.value_len >= MAX_VALUE)
            continue;

        key[qp.key_len] = '\0';
        value[qp.value_len] = '\0';
        if(strcmp(key, "ip") == 0 && strnlen(value, MAX_IP + 1) < MAX_IP)
            strcpy(ip_address, value);
        
        if(strcmp(key, "type") == 0 && strnlen(value, MAX_TYPE + 1) < MAX_TYPE)
            strcpy(type, value);
    }
    if(ip_address[0] == '\0')
        ip = FCGX_GetParam("REMOTE_ADDR", request->envp);
    else
        ip = ip_address;

    if(ip == NULL) {
        bad_request(request);
        return;
    }
    country_id = lookup_country_id(ip);
    FCGX_PutS("Content-Type: text/plain\r\n\r\n", request->out);
    country_code = country_code_by_id(country_id);
    if (country_code == NULL) {
        country_code = "Undefined";
        xon_code = "--";
    } else {
        xon_code = country_code;
    }

    country_code3 = country_code3_by_id(country_id);
    if (country_code3 == NULL)
        country_code3 = "Undefined";

    name = country_name_by_id(country_id);
    if(name == NULL)
        name = "Undefined";

    if(strcmp(type, "all") == 0) {
        FCGX_FPrintF(request->out, "%s\n%s\n%s", country_code, country_code3, name);
    } else if(strcmp(type, "code3") == 0) {
        FCGX_PutS(country_code3, request->out);
    } else if(strcmp(type, "name") == 0) {
        FCGX_PutS(name, request->out);
    } else if(strcmp(type, "xonotic") == 0) {
        FCGX_FPrintF(request->out, "%s %s", xon_code, ip);
    } else {
        FCGX_PutS(country_code, request->out);
    }


#undef MAX_IP
#undef MAX_TYPE
}


static void dispatch(FCGX_Request *request, struct doit_args *thread_args) {
    _Bool not_found = true;
    char *url = FCGX_GetParam("SCRIPT_NAME", request->envp);
    for(int i = 0; url_hanlders[i].path != NULL; i++) {
        if(strcmp(url, url_hanlders[i].path) == 0){
            url_hanlders[i].handler(request);
            not_found = false;
            break;
        }
    }
    if(not_found)
        not_found404(request);
}

static void *doit(struct doit_args *thread_args) {
    int rc;
    FCGX_Request request;
    FCGX_InitRequest(&request, thread_args->socket, 0);
    for(;;) {
        // on some systems this require lock
        rc = FCGX_Accept_r(&request);
        if(rc < 0) // if error then exit
            break;

        dispatch(&request, thread_args);
        FCGX_Finish_r(&request);
    }

    return NULL;
}

void Cleanup_App(){
    puts("Cleanup");
    FCGX_ShutdownPending();
    Geoip_Cleanup();
}

void handle_signal(int signal) {
    exit(EXIT_SUCCESS);
}


void Init_App() {
    struct sigaction sa;

    FCGX_Init();
    Geoip_Init();
    sa.sa_handler = &handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGTERM, &sa, NULL) < 0)
        goto err;

    if(sigaction(SIGINT, &sa, NULL) < 0)
        goto err;

    atexit(Cleanup_App);
    return;
    err:
        perror("Error setting signal handler");
        exit(EXIT_FAILURE);
}

#if(__STDC_VERSION__ >= 20112L)
static noreturn void usage()
#else
static void usage()
#endif
{
    fputs(USAGE_STRING "\n", stderr);
    exit(EXIT_FAILURE);
}



int main(int argc, char *argv[]) {
    int sock_fd, opt;
#ifdef WITH_CHROOT
    const char *chroot_dir = NULL;
#endif
    pthread_t id[THREAD_COUNT];
    struct doit_args thread_args[THREAD_COUNT - 1];
    sock_fd = 0;

    while ((opt = getopt(argc, argv, "s:c:")) != -1) {
        switch (opt) {
            case 's':
                sock_fd = FCGX_OpenSocket(optarg, BACKLOG);
                break;
#ifdef WITH_CHROOT
            case 'c':
                chroot_dir = optarg;
                break;
#endif
            default:
                usage(); // no return
        }
    }

    if(sock_fd < 0) {
        perror("Error opening socket\n");
        return EXIT_FAILURE;
    }

    Init_App();

#ifdef WITH_CHROOT
    if (chroot_dir != NULL) {
        if (chdir(chroot_dir) != 0 || chroot(chroot_dir) != 0) {
            perror("Failed to chroot");
            exit(EXIT_FAILURE);
        }
    }
#endif
    thread_args[0].thread_num = 0;
    thread_args[0].socket = sock_fd;
    for(int i = 1; i < THREAD_COUNT; i++) {
        pthread_create(&id[i - 1], NULL, (void*(*)(void*))doit, (void *)&thread_args[i]);
    }

    doit(&thread_args[0]);
    return 0;
}
