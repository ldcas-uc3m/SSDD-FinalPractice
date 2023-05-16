/*
Server implementation for linked list
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "server_impl.h"
#include "linked_list.h"
#include "comm.h"
#include "log.h"

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

int getIPPort(char* aliasSender, char* IP, int* port){

    return ipPortInfo(&list, aliasSender, IP, port);
}


int disconnect_user(char* username, char* IP){

    return disconnectUser(&list, username, IP);
}

int register_user(char* username, char* alias, char* datetime){

    return registerUser(&list, username, alias, datetime);
}

int unregister_user(char* alias){

    return unregisterUser(&list, alias);
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

int userConnected(char* IP){

    return isConnected(&list, IP);
}

int userConnectedUsername(char* username){

    return isConnectedUsername(&list, username);
}

int seeNumberConnected(int* connections){
    
    return numberConnected(&list, connections);
}

int connectedUsers(char** users) {
    if (listConnected(&list, users)==-1){
        Log("Error when retrieving list of connected users\n");
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