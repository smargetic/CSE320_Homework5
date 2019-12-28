#include "client_registry.h"
#include <semaphore.h>
#include <unistd.h>
#include <sys/socket.h>

sem_t mutex;

struct client_registry{
    //int* arr; //holds array of open fd's
    char* name; //holds name associated with particular fd
    int fdInCR;
    CLIENT_REGISTRY *next;
    CLIENT_REGISTRY *prev;
    //CLIENT_REGISTRY *head;
};

CLIENT_REGISTRY *head;

//CLIENT_REGISTRY *arrClient;

void creg_fini(CLIENT_REGISTRY *cr){
    //removes client from client_registry
    //I get the previous CR
    CLIENT_REGISTRY *prevCR = cr->prev;
    CLIENT_REGISTRY *nextCR = cr->next;
    if(prevCR == nextCR){
        //at head, nothing to remove
        return;
    } else{
        //I remove it
        prevCR->next = nextCR;
        nextCR->prev = prevCR;
    }

}

int creg_register(CLIENT_REGISTRY *cr, int fd){
    //adds client to client registry
    cr->fdInCR = fd;
    CLIENT_REGISTRY *temp = head->prev;
    head->prev =cr;
    temp->next = cr;
    cr->next = head;
    cr->prev = temp;

    return 0;
}

int creg_unregister(CLIENT_REGISTRY *cr, int fd){
    CLIENT_REGISTRY *prevCR = cr->prev;
    CLIENT_REGISTRY *nextCR = cr->next;
    if(prevCR == nextCR){
        //at head, nothing to remove
        return -1;
    } else{
        //I remove it
        prevCR->next = nextCR;
        nextCR->prev = prevCR;
        //unregister a client fd
        int success = close(fd);
        if(success<0){
            return -1;
        }

    }
    return 0;
}

void creg_wait_for_empty(CLIENT_REGISTRY *cr){
    //use semaphore
    sem_init(&mutex, 0, 1);

    sem_wait(&mutex);

    //wait until clients are 0 --> ie ->next = ->prev
    sem_post(&mutex);

    sem_destroy(&mutex);

    //assume that creg_shutdown_all should be called
}

void creg_shutdown_all(CLIENT_REGISTRY *cr){
    //use shutdown
    cr = head;
    while((cr->next)!=(cr->prev)){
        shutdown(cr->fdInCR, SHUT_RDWR);

        //remove from registry
        CLIENT_REGISTRY *temp = cr->next;
        creg_unregister(cr, cr->fdInCR);
        cr= temp;

    }
    //assume that EOF should be sent to all threads
}
