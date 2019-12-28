#include <stdlib.h>

#include "client_registry.h"
#include "exchange.h"
#include "trader.h"
#include "debug.h"


extern EXCHANGE *exchange;
extern CLIENT_REGISTRY *client_registry;

static void terminate(int status);

#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "csapp.h"

//#include "server.h"

/*
 * "Bourse" exchange server.
 *
 * Usage: bourse <port>
 */
void handler(int sig){
    //signal handler
    printf("\nWEnt here");
    terminate(EXIT_FAILURE);
    //close(servfd);
}

void *thread(void* varg){
    printf("\nIn thread");
    //int conf = *((int *)varg);
    pthread_detach(pthread_self());
    //printf("\nValue of varg %d", *(int*)varg);
    free(varg);

    //echo(conf);
    printf("\nIn thread");
    //close(conf);
    return NULL;

}

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    //I set up the signal handler
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGHUP, &sa, NULL);


    //can either give -p, -h, -q, or nothing
    char* port;


    for(int i =0; i<argc; i++){
        if(strcmp(argv[i],"-p")==0){
            if((i+1)<argc){
                //there is another value, which will be the port
                port = argv[i+1];
                break;
            } else{
                printf("\nUsage: %s -p <port>", argv[0]);
                //there is some sort of error
            }
        }
    }



    // Perform required initializations of the client_registry,
    // maze, and player modules.
    client_registry = creg_init();
    exchange = exchange_init();
    trader_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function brs_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    socklen_t clientlen;
    int listenfd;
    int *connfd;

    pthread_t tid;
    struct sockaddr_storage clientaddr;
    int portNum = atoi(port);
    printf("\nValue of port %d", portNum);
    listenfd = Open_listenfd(portNum);
    printf("\nValue of listenfd %d", listenfd);
    if(listenfd<0){
        //bad file descriptor
        terminate(EXIT_FAILURE);
    }
    printf("\nValue of listenfd %d", listenfd);
    while(1){
        printf("\nIN While");
        connfd = Malloc(sizeof(int));
        clientlen = sizeof(struct sockaddr_storage);
        *connfd = Accept(listenfd, (SA*) &clientaddr, &clientlen);
        printf("\nValue of Connfd %d", *connfd);
        Pthread_create(&tid, NULL, thread, connfd);

    }


    terminate(EXIT_SUCCESS);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    exchange_fini(exchange);
    trader_fini();

    debug("Bourse server terminating");
    exit(status);
}
