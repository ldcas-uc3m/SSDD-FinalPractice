/*
Server implementation for tuples
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "server_impl.h"
#include "linked_list.h"
#include "comm.h"
#include "../log.h"


List list = NULL;


int init() {
    /*
    Initializes the list. If it alredy exists, it deletes it.
    */
    
    if (list != NULL) {  // reset list
        destroy(&list);
    }

    initList(&list);
    
    return 0;
}

int connect_user(char* alias, char* IP, int port, int* nonSent, int* lastSent){

    return connectUser(&list, alias, IP, port, nonSent, lastSent);
}

int disconnect_user(char* username){

    return disconnectUser(&list, username);
}

int register_user(char* username, char* alias, char* datetime, char* IP, int port){

    return registerUser(&list, username, alias, datetime, IP, port);
}

int unregister_user(char* username){

    return unregisterUser(&list, username);
}

int sendMessage_store(char* aliasSender, char* aliasReceived, char* message, int* identifier){
    
    return sendMessageStore(&list, aliasSender, aliasReceived, message, identifier);
}

int sendMessage_deliver(char* aliasSender, char* aliasReceived, char* message, int identifier, char* IP, int* port){

    return sendMessageDeliver(&list, aliasSender, aliasReceived, message, identifier, IP, port);
}

int confirm_received(char* receiver, int identifier){

    return confirmReceived(&list, receiver, identifier);
}

int userConnected(char* username){

    return isConnected(&list, username);
}

int connectedUsers(int* connections, char** users) {
    if (numberConnected(&list, connections)!=-1){
        users = (char**) malloc(sizeof(char*) * *connections);
        int i;
        for (i=0;i++;i< *connections){
            users[i] = (char*)malloc(sizeof(char)*MAX_CHAR);
        }
        if (listConnected(&list, users)==-1){
            Log("Error when retrieving list of connected users\n");
            return -1;
        }
    }else{
        return -1;
    }
    return 0;
}


void destroy() {
    /*
    Destroys the list and the mutexes.
    */

    destroyList(&list);

    return;
}