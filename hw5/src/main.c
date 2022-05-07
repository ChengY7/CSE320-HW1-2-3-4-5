#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


#include "pbx.h"
#include "tu.h"
#include "server.h"
#include "debug.h"

#include "csapp.h"


static void terminate(int status);
void sighup_handler(int signal) {
    terminate(EXIT_SUCCESS);
}
int isNumber(char *str) {
    while(*str!='\0') {
        if(*str<48 || *str>57) {
            return 0;
        }
        str++;
    }
    return 1;
}

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    int option;
    char *port;
    if(argc==1) {
        fprintf(stderr, "Usage: demo/pbx -p <port>\n");
        exit(EXIT_SUCCESS);
    }
    if(argc==2) {
        if(*argv[1]!='-' || *(argv[1]+1)!='p') {
            fprintf(stderr, "Usage: demo/pbx -p <port>\n");
            exit(EXIT_FAILURE);
        }
    }
    int optionflag=1;
    while ((option=getopt(argc, argv, "p:"))!=-1) {
        switch(option) {
            case 'p':
                port=optarg;
                optionflag=1;
                break;
            default:
                optionflag=0;
                break;
        }
    }
    if(optionflag==0) {
        fprintf(stderr, "Usage: demo/pbx -p <port>\n");
        exit(EXIT_FAILURE);
    }
    if(isNumber(port)==0) {
        fprintf(stderr, "Usage: demo/pbx -p <port>\n");
        exit(EXIT_FAILURE);
    }
    int num = atoi(port);
    if(num<1024) {
        fprintf(stderr, "bind: Permission denied\n");
        exit(EXIT_FAILURE);
    }
    // Perform required initialization of the PBX module.
    debug("Initializing PBX...");
    pbx = pbx_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags=0;
    action.sa_handler=sighup_handler;
    sigaction(SIGHUP, &action, NULL);

    int listenfd;
    listenfd=open_listenfd(port);
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    int connfd;
    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd=Accept(listenfd, (SA*) &clientaddr, &clientlen);
        int *connfdp=malloc(sizeof(int));
        *connfdp=connfd;
        pthread_create(&tid, NULL, pbx_client_service, connfdp);
    }
    fprintf(stderr, "You have to finish implementing main() "
	    "before the PBX server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    debug("PBX server terminating");
    exit(status);
}
