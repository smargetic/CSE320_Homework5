#include <stdlib.h>

//#include "protocol.h"
#include "trader.h"

#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "debug.h"


int proto_send_packet(int fd, BRS_PACKET_HEADER *hdr, void *payload){
    uint16_t sizeOfPayload = hdr->size;

    //convert data of header from host byte order to network byte order
    hdr->size = htons(hdr->size);

    hdr->timestamp_sec = htonl(hdr->timestamp_sec);
    hdr->timestamp_nsec = htonl(hdr->timestamp_nsec);


    int success1 = write(fd, hdr, sizeof(BRS_PACKET_HEADER));
    if(success1<0){
        return -1;
    }

    if(sizeOfPayload>0){
        //convert payload data from host byte order to network byte order

        int success2 = write(fd, payload, sizeOfPayload);
        if(success2<0){
            return -1;
        }
    }
    return 0;
}


int proto_recv_packet(int fd, BRS_PACKET_HEADER *hdr, void **payloadp){
    debug("\n In proto");
    debug("\nValue of fd: %d", fd);
    //void* buf = malloc(sizeof(BRS_PACKET_HEADER));
    //debug("\n max size val %ld", sizeof(BRS_PACKET_HEADER));
    //int success1 = read(fd, buf, sizeof(BRS_PACKET_HEADER));
    int success1 = read(fd, hdr, sizeof(BRS_PACKET_HEADER));
    //hdr = (BRS_PACKET_HEADER*)buf;
    debug("\nValue of success1 %d", success1);
    if(success1<0){
        return -1;
    }
    debug("\nblah");
    //I get the header values
    debug("\nTemp Size: %d", hdr->size);

    hdr->size = ntohs(hdr->size);
    hdr->timestamp_sec = ntohl(hdr->timestamp_sec);
    hdr->timestamp_nsec = ntohl(hdr->timestamp_nsec);

    int sizePayload = hdr->size;
    debug("\nAfter some stuff");
    if(sizePayload>0){
        debug("\n Payload >0");

        //I read the payload from the fd
        debug("\nSIZE OF PAYLOAD: %d", sizePayload);
        debug("\nfd here %d", fd);
        *payloadp = (void*)malloc(sizeof(char)*sizePayload);
        int success2 = read(fd, *payloadp, sizePayload*sizeof(char));
        debug("\nValue of success: %d", success2);
        if(success2<0){
            debug("\nHere?");
            return -1;
        }

    }

    debug("\nout");

    return 0;
}
