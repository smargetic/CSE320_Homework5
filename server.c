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

#include "server.h"
#include "csapp.h"
#include "protocol.h"
#include <time.h>
#include <sys/time.h>

#include "protocol.h"
#include "debug.h"
#include "exchange.h"

struct stats{
    int* currentBids;
    int* askVals;
    int lastTradePrice;
    int accountBalance;
};

void *brs_client_service(void *arg){
    debug("\nIn client thread");

    int fd = *((int *)arg);
    debug("\nLocal copy %p", arg);
    //debug("\nvalue of local copy %d", (int)*arg);
    //fd = 4;

    debug("\nValue of fd2 %d", fd);
    //printf("\n does this work %d", *(int*)arg);

    void* temp = NULL;

    pthread_detach(pthread_self());
    free(arg);
    //CLIENT_REGISTRY *cr; //is this initialized? --> get the one from main
    //creg_register(cr ,fd);
    int loginDone =0; //can preform other actions once login is recieved
    //struct stats stat;


    TRADER *td;
    //BRS_STATUS_INFO *tdrStatus = (BRS_STATUS_INFO *)malloc(sizeof(BRS_STATUS_INFO));


    while(1){
        debug("\n IN while loop");
        //get packets from user in infinite loop

        BRS_PACKET_HEADER *hdr = (BRS_PACKET_HEADER*)malloc(sizeof(BRS_PACKET_HEADER));
        debug("\nHere 1");
        void** payload = malloc(sizeof(void**)); //NEED TO ADJUST THIS
        debug("\nHere2");
        int recieved = proto_recv_packet(fd, hdr, payload);
        debug("\nValue of recieved: %d", recieved);
        debug("\nPast first statement");
        if(recieved ==0){
            debug("\nSomething recieved");
            //something has been recieved
            int type = (int)hdr->type;
            //struct timespec time;
            if(loginDone==0){
                //only thing that can be done is login
                //struct timespec time;

                if(type == BRS_LOGIN_PKT){
                    //try to login
                        td= trader_login(fd, *payload);

                        if(td==NULL){ //trader login failed -->maybe payload is null
                            //send nack
                            void* pack = malloc(sizeof(void*));
                            trader_send_nack(td);
                            //proto_send_packet(fd, hdr2, pack);
                            free(pack);
                        } else{
                            debug("\nIn login");
                            //send ack
                            /*
                            BRS_PACKET_HEADER *hdr2 = malloc(sizeof(BRS_PACKET_HEADER));
                            hdr2->type = BRS_ACK_PKT;
                            hdr2->size =0;
                            //time
                            clock_gettime(CLOCK_REALTIME, &time);
                            hdr2->timestamp_sec = time.tv_sec;
                            hdr2->timestamp_sec = time.tv_nsec;
                            */
                            void* pack = malloc(sizeof(void*));
                            //proto_send_packet(fd, hdr2, pack);
                            trader_send_ack(td, NULL);
                            exchange_get_status(exchange, pack);
                            free(pack);
                            loginDone = 1;
                        }
                    //}
                }
            } else {
                void* pack = (BRS_STATUS_INFO*)malloc(sizeof(BRS_STATUS_INFO));
                //can do whatever (but log in)
                if(type==BRS_STATUS_PKT){

                    //void* pack = (BRS_STATUS_INFO*)malloc(sizeof(BRS_STATUS_INFO));

                    //EXCHANGE *temp = malloc(sizeof(exchange));
                    exchange_get_status(exchange, pack);
                    trader_send_ack(td, pack);
                    //*pack = stat;
                    //pack = &stat;
                    free(pack);

                    //proto_send_packet(fd, hdr2, pack);

                } else if(type == BRS_DEPOSIT_PKT){
                    //void* pack = (BRS_STATUS_INFO*)malloc(sizeof(BRS_STATUS_INFO));
                    //I find out the deposit in the packet
                    int moneyToAdd = *((int*)*payload);
                    trader_increase_balance(td, moneyToAdd);
                    trader_send_ack(td, pack);

                    //tdrStatus->balance = tdrStatus->balance + moneyToAdd;
                    //trader_send_ack(td,tdrStatus);


                } else if(type == BRS_WITHDRAW_PKT){
                    //void* pack = (BRS_STATUS_INFO*)malloc(sizeof(BRS_STATUS_INFO));
                    //I find out the deposit in the packet
                    uint32_t moneyToRemove = *((uint32_t*)*payload);
                    int success = trader_decrease_balance(td, moneyToRemove);
                    if(success ==0){
                        trader_send_ack(td, pack);

                    } else{
                        trader_send_nack(td);
                    }
                } else if(type == BRS_ESCROW_PKT){
                    //void* pack = (BRS_STATUS_INFO*)malloc(sizeof(BRS_STATUS_INFO));
                    BRS_ESCROW_INFO *inventory2Increase = (BRS_ESCROW_INFO*)*payload;
                    quantity_t increaseQuant = inventory2Increase->quantity;
                    trader_increase_inventory(td, increaseQuant);
                    trader_send_ack(td, pack);
                    //BRS_PACKET_HEADER *hdr2 = malloc(sizeof(BRS_PACKET_HEADER));
                    //hdr2->size = 0;

                } else if(type == BRS_RELEASE_PKT){
                    //void* pack = (BRS_STATUS_INFO*)malloc(sizeof(BRS_STATUS_INFO));
                    BRS_ESCROW_INFO *inventory2Release = (BRS_ESCROW_INFO*)*payload;
                    //(BRS_ESCROW_INFO*)malloc(sizeof(BRS_ESCROW_INFO));
                    //inventory2Release = (BRS_ESCROW_INFO*)*payload;
                    quantity_t releaseQuant = inventory2Release->quantity;
                    int success = trader_decrease_inventory(td, releaseQuant);
                    if(success==0){
                        trader_send_ack(td, pack);
                    } else{
                        trader_send_nack(td);
                    }
                    //free(inventory2Release);
                    //BRS_PACKET_HEADER *hdr2 = malloc(sizeof(BRS_PACKET_HEADER));
                    //hdr2->size = 0;

                } else if (type == BRS_BUY_PKT){
                    //BRS_ORDER_INFO *buyData = (BRS_ORDER_INFO *)(*payload);
                    //BRS_PACKET_HEADER *hdr2 = malloc(sizeof(BRS_PACKET_HEADER));
                    //hdr2->size = 0;

                } else if(type == BRS_SELL_PKT){
                    //BRS_PACKET_HEADER *hdr2 = malloc(sizeof(BRS_PACKET_HEADER));
                    //hdr2->size = 0;


                } else if(type == BRS_CANCEL_PKT){
                    //BRS_PACKET_HEADER *hdr2 = malloc(sizeof(BRS_PACKET_HEADER));
                    //hdr2->size = 0;

                }
                free(pack);
            }


        }
        free(payload);
        free(hdr);
    }


    printf("\nValue of fd: %d", fd);
    return temp;
}

