/*
Definition of concurrent linked list library
*/

#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

struct Mensaje {
    char* mensaje;
    char* sender;
    bool sent;
    int id;
    struct Mensaje *next;
};

typedef struct Mensaje* Mensajes;

struct Node {
    char* username;
    char* alias;
    char* datetime;
    bool connected;
    char* IP;
    int port;
    Mensajes listMessages;
    int lastIDSent;
    struct Node *next;
};




typedef struct Node* List;

int initList(List* l);
int connectUser(List* l, char* alias, char* IP, int port, int* nonSent, int* lastSent);
int disconnectUser(List* l, char* username);
int registerUser(List* l, char* username, char* alias, char* datetime, char* IP, int port);
int unregisterUser(List* l, char* username);
int sendMessageStore(List* l, char* aliasSender, char* aliasRecieved, char* message, int* identifier);
int sendMessageDeliver(List* l, char* aliasSender, char* aliasReceived, char* message, int identifier, char* IP, int* port);
int listConnected(List* l, char** users);
int numberConnected(List* l, int* connections);
int confirmReceived(List *l, char* aliasReceiver, int identifier);
int destroyList(List *l);
int isConnected(List *l, char* username);

#endif