#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "pbx.h"
#include "server.h"
#include "debug.h"

#include "csapp.h"


static void terminate(int status);
void sighup_handler(int signal) {
    printf("123");
    terminate(EXIT_SUCCESS);
}

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */
int main(int argc, char* argv[]){
    printf("%d\n", PBX_MAX_EXTENSIONS);
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    int option;
    char *port;
    while ((option=getopt(argc, argv, "p:"))!=-1) {
        switch(option) {
            case 'p':
                port=optarg;
                break;
            default:
                printf("testing");
                break;
        }
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
    action.sa_handler=sighup_handler;
    sigaction(SIGHUP, &action, NULL);

    int listenfd, *connfd;
    listenfd=open_listenfd(port);
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    while(1) {
        connfd = malloc(sizeof(int));
        clientlen = sizeof(struct sockaddr_storage);
        *connfd=Accept(listenfd, (SA*) &clientaddr, &clientlen);
        pthread_create(&tid, NULL, pbx_client_service, connfd);
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
