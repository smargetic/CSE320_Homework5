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
    //fd = 4;

    debug("\nValue of fd2 %d", fd);
    //printf("\n does this work %d", *(int*)arg);

    void* temp = NULL;

    pthread_detach(pthread_self());
    free(arg);
    int loginDone =0; //can preform other actions once login is recieved


    TRADER *td;

    while(1){
        debug("\n IN while loop");
        //get packets from user in infinite loop

        BRS_PACKET_HEADER *hdr = (BRS_PACKET_HEADER*)malloc(sizeof(BRS_PACKET_HEADER));
        debug("\nHere 1");
        void** payload = malloc(sizeof(void**));
        debug("\nHere2");
        int recieved = proto_recv_packet(fd, hdr, payload);
        debug("\nValue of recieved: %d", recieved);
        debug("\nPast first statement");
        if(recieved ==0){
            debug("\nSomething recieved");
            //something has been recieved
            int type = (int)hdr->type;
            if(loginDone==0){
                //only thing that can be done is login

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
                            void* pack = malloc(sizeof(void*));
                            //proto_send_packet(fd, hdr2, pack);
                            trader_send_ack(td, NULL);
                            exchange_get_status(exchange, pack);
                            free(pack);
                            loginDone = 1;
                        }
                }
            } else {
                BRS_STATUS_INFO* pack = (BRS_STATUS_INFO*)malloc(sizeof(BRS_STATUS_INFO));
                //can do whatever (but log in)
                if(type==BRS_STATUS_PKT){
                    exchange_get_status(exchange, pack);
                    trader_send_ack(td, pack);

                } else if(type == BRS_DEPOSIT_PKT){
                   //I find out the deposit in the packet

                    int moneyToAdd = *((int*)*payload);
                    trader_increase_balance(td, moneyToAdd);
                    trader_send_ack(td, pack);


                } else if(type == BRS_WITHDRAW_PKT){
                    //I find out the deposit in the packet
                    uint32_t moneyToRemove = *((uint32_t*)*payload);
                    int success = trader_decrease_balance(td, moneyToRemove);
                    if(success ==0){
                        trader_send_ack(td, pack);

                    } else{
                        trader_send_nack(td);
                    }
                } else if(type == BRS_ESCROW_PKT){
                    BRS_ESCROW_INFO *inventory2Increase = (BRS_ESCROW_INFO*)*payload;
                    quantity_t increaseQuant = inventory2Increase->quantity;
                    trader_increase_inventory(td, increaseQuant);
                    trader_send_ack(td, pack);
                    free(inventory2Increase);

                } else if(type == BRS_RELEASE_PKT){
                    BRS_ESCROW_INFO *inventory2Release = (BRS_ESCROW_INFO*)*payload;
                    quantity_t releaseQuant = inventory2Release->quantity;
                    int success = trader_decrease_inventory(td, releaseQuant);
                    if(success==0){
                        trader_send_ack(td, pack);
                    } else{
                        trader_send_nack(td);
                    }

                    free(inventory2Release);

                } else if (type == BRS_BUY_PKT){
                    BRS_ORDER_INFO *buyData = (BRS_ORDER_INFO *)(*payload);
                    quantity_t quant = buyData->quantity;
                    funds_t cost = buyData->price;
                    orderid_t id = exchange_post_buy(exchange, td, quant, cost);

                    if(id==0){
                        //buy could not go through
                        trader_send_nack(td);
                    } else {
                        trader_send_ack(td, pack);
                    }
                    free(buyData);

                } else if(type == BRS_SELL_PKT){
                    BRS_ORDER_INFO *sellData = (BRS_ORDER_INFO *)(*payload);
                    quantity_t quant = sellData->quantity;
                    funds_t cost = sellData->price;
                    orderid_t id = exchange_post_sell(exchange, td, quant, cost);

                    if(id==0){
                        //buy could not go through
                        trader_send_nack(td);
                    } else {
                        trader_send_ack(td, pack);
                    }
                    free(sellData);


                } else if(type == BRS_CANCEL_PKT){
                    BRS_CANCEL_INFO *cancelInf = (BRS_CANCEL_INFO*)(*payload);
                    orderid_t orderID = cancelInf->order;
                    quantity_t* quant = malloc(sizeof(quantity_t));
                    int success2 = exchange_cancel(exchange, td, orderID, quant);
                    if(success2==0){
                        //successful cancel
                        trader_send_ack(td, pack);
                    } else{
                        trader_send_nack(td);
                    }
                    free(quant);

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

