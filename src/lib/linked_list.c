/*
Implementación de una linked list que guarda tuplas (int key, char value1[], int value2, double value3)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "linked_list.h"
#include "comm.h"
#include "../log.h"

pthread_mutex_t mutex_list;

/*
Todas estas funciones devuelve 0 si se ejecutan con éxito y -1 en caso de algún error.
*/

int destroyMessages(Mensajes* l){
       /*
    Elimina todos los elementos de la lista l.
    */

    Mensajes aux = *l;

    while (aux != NULL) {
        Mensajes tmp = aux;
        aux = aux->next;
        free(tmp->mensaje);
        free(tmp->sender);
        free(tmp);
    }

    return 0; 
}

int numberNonSent(Mensajes* l, char* alias){
    /*
    Recorre todos los elementos de la lista l y mira cuantos mensajes quedan por enviar
    */
    int counter = 0;
    Mensajes aux = *l;
    while (aux != NULL) {
        if (aux->sent == false){
            counter +=1;
        }
        aux = aux->next;
    }

    Log("]\n");

    return counter;
}

int getMessage(Mensajes* l, char* aliasSender, char* message, int identifier){
    Mensajes aux = *l;
    bool found = false;
    while (aux != NULL && found ==false) {
        if (aux->id == identifier){
            found =true;
            if (aux->sent==true){
                Log("Message already sent\n");
                return -1;
            }

            strcpy(aliasSender, aux->sender);
            strcpy(message, aux->mensaje);
        }
        aux = aux->next;
    }

    if (found==false){
        Log("Identifier not found");
        return -1;
    }

    return 0;
}

int storeMessage(Mensajes* l, char* aliasSender, char* message, int* identifier){
    /*
    Inserta una nueva tupla en la lista l.
    La inserción se hace al principio de la lista.
    */

    // traverse the list
    int id = 0;
    Mensajes aux = *l;  // head
    if (aux!=NULL){
        id = aux->id;
    }

    while (aux != NULL) {
        aux = aux->next;
    }

    // aux is now tail
    
    // insert new element

    // new node
    struct Mensaje* ptr;
    ptr = (struct Mensaje*) malloc(sizeof(struct Mensaje));
    
    if (ptr == NULL) {
        Log("malloc() fail\n");

        return -1;
    }

    // allocate memory for value1
    ptr->sender = (char*) malloc(MAX_CHAR);  // new string
    ptr->mensaje = (char*) malloc(MAX_CHAR);  // new string
    if (ptr->sender == NULL || ptr->sender == NULL) {  // failed allocation
        free(ptr);

        Log("malloc() fail\n");

        return -1;
    }

    // form node
    strcpy(ptr->sender,aliasSender);
    strcpy(ptr->mensaje, message);
    ptr->sent = false;
    ptr->id = id + 1;
    *identifier = id+1;
    ptr->next = NULL;


    // link to the list
    if (l == NULL) {  // emtpy list, insert in head
        // pthread_mutex_lock(&mutex_list);
        *l = ptr;
    }
    else {  // insert in head
        // pthread_mutex_lock(&mutex_list);
        ptr->next = *l;
        *l = ptr;
    }
    return 0;
}

int notifyReceived(Mensajes* l, int identifier){
    Mensajes aux = *l;
    bool found = false;
    while (aux != NULL && found ==false) {
        if (aux->id == identifier){
            found =true;
            if (aux->sent==true){
                Log("Message already sent\n");
                return -1;
            }else{
                aux->sent = true;
                return 0;
            }
        }
        aux = aux->next;
    }

    if (found==false){
        Log("Identifier not found");
        return -1;
    }

    return 0;
}

int initList(List* l) {
    /*
    Inicializa una lista como lista vacía. 
    Una lista vacía es una lista que apunta a NULL.
    */
    
    // init mutex
    pthread_mutex_init(&mutex_list, NULL);

    // start list
    *l = NULL;

    return 0;
}

int connectUser(List* l, char* alias, char* IP, int port, int* nonSent, int* lastSent){
        /*
    Connects the user with a specific alias
    */
    
    // traverse the list
    pthread_mutex_lock(&mutex_list);
    
    List aux = *l;  // head
    while (aux != NULL) {
        if (strcmp(aux->alias, alias)==0) {  // found
            if (aux->connected == false){
                aux->connected = true;
                strcpy(aux->IP, IP);
                aux->port = port;
                *nonSent = numberNonSent(&(aux->listMessages),alias);
                *lastSent = aux->lastIDSent;
                pthread_mutex_unlock(&mutex_list);
                return 0;
            }else{
                Log("User already connected\n");
                pthread_mutex_unlock(&mutex_list);
                return 2;
            }
        }
        else {  // next element
            aux = aux->next;
        }
    }
    pthread_mutex_unlock(&mutex_list);

    Log("Element not found\n");

    return 1;  // not found
}

int disconnectUser(List* l, char* alias, char* IP){
    
    /*
    Disconnects the user with a specific alias
    */
    
    // traverse the list
    pthread_mutex_lock(&mutex_list);
    
    List aux = *l;  // head
    while (aux != NULL) {
        if (strcmp(aux->alias,alias)==0) {  // found
            if (aux->connected == true){
                if (strcmp(IP, aux->IP)==0){
                    aux->connected = false;
                    strcpy(aux->IP, "None");
                    aux->port = -1;
                    pthread_mutex_unlock(&mutex_list);
                    return 0;
                }else{
                    return 3;
                }
            }else{
                Log("User already disconnected\n");
                pthread_mutex_unlock(&mutex_list);
                return 2;
            }
        }else {  // next element
            aux = aux->next;
        }
    }
    pthread_mutex_unlock(&mutex_list);

    Log("Element not found\n");

    return 1;  // not found
}

int registerUser(List* l, char* username, char* alias, char* datetime){
    pthread_mutex_lock(&mutex_list);    
    List aux = *l;  // head
    while (aux != NULL) {
        if (strcmp(aux->alias,alias)==0) {  // key is already inserted

            Log("Username already inserted\n");

            pthread_mutex_unlock(&mutex_list);
            return 1;
        }
        else {  // next element
            aux = aux->next;
        }
    }

    // aux is now tail
    
    // insert new element

    // new node
    struct Node* ptr;
    ptr = (struct Node*) malloc(sizeof(struct Node));
    
    if (ptr == NULL) {
        Log("malloc() fail\n");

        return 2;
    }

    // allocate memory for value1
    ptr->username = (char*) malloc(MAX_CHAR);  // new string
    if (ptr->username == NULL) {  // failed allocation
        free(ptr);

        Log("malloc() fail\n");

        return 2;
    }
    ptr->alias = (char*) malloc(MAX_CHAR);  // new string
    if (ptr->alias == NULL) {  // failed allocation
        free(ptr);

        Log("malloc() fail\n");

        return 2;
    }
    ptr->datetime = (char*) malloc(MAX_CHAR);  // new string
    if (ptr->datetime == NULL) {  // failed allocation
        free(ptr);

        Log("malloc() fail\n");

        return 2;
    }
    ptr->IP= (char*) malloc(MAX_CHAR);  // new string
    if (ptr->IP == NULL) {  // failed allocation
        free(ptr);

        Log("malloc() fail\n");

        return 2;
    }

    // form node
    strcpy(ptr->username, username);
    strcpy(ptr->alias, alias);
    strcpy(ptr->datetime, datetime);
    Mensajes listMessages = NULL;
    ptr->listMessages = listMessages;
    ptr->next = NULL;

    Log("Guardando %s\n",ptr->username);
    Log("Guardando %s\n",ptr->alias);
    Log("Guardando %s\n",ptr->datetime);


    // TODO link to the list
    if (*l == NULL) {  // emtpy list, insert in head
        // pthread_mutex_lock(&mutex_list);
        *l = ptr;
        pthread_mutex_unlock(&mutex_list);
    }else {  // insert in head
        // pthread_mutex_lock(&mutex_list);
        ptr->next = *l;
        *l = ptr;
        pthread_mutex_unlock(&mutex_list);
    }
    return 0;
}

int unregisterUser(List* l, char* alias){
     /*
    Elimina un par de la lista l, identificado por su username.
    */
    
    // primer elemento de la lista
    pthread_mutex_lock(&mutex_list);

    List aux, back;
    if (*l == NULL) { // lista vacia
        pthread_mutex_unlock(&mutex_list);
        return 1;
    }
    
    if (strcmp(alias,(*l)->alias) == 0) {
        aux = *l;
        *l = (*l)->next;
        free(aux->username);
        free(aux->alias);
        free(aux->datetime);
        free(aux->IP);
        free(aux->listMessages);
        free(aux);

        pthread_mutex_unlock(&mutex_list);
        return 0;
    }

    aux = (*l)->next;
    back = *l;

    // resto de elementos
    while (aux != NULL) {
        if (strcmp(aux->alias,alias)==0) {  // found
            back->next = aux->next;
            free(aux->username);
            free(aux->alias);
            free(aux->datetime);
            free(aux->IP);
            free(aux->listMessages);
            free(aux);

            pthread_mutex_unlock(&mutex_list);
            return 0;  
        }
        else {
            back = aux;
            aux = aux->next;
        }
    }
    pthread_mutex_unlock(&mutex_list);

    Log("User not found\n");
    return 1;  // not found
}

int sendMessageStore(List* l, char* aliasSender, char* aliasRecieved, char* message, int* identifier){
        pthread_mutex_lock(&mutex_list);
    
    List aux = *l;
    
    List node1 = NULL;
    List node2 = NULL;
    while (aux != NULL) {
        if (strcmp(aux->alias,aliasRecieved)==0) {
            node1 = aux;
        }
        if (strcmp(aux->alias,aliasSender)==0) {
            node2 = aux;
        }
        aux = aux->next;
        if (node1 != NULL && node2!=NULL) {
            break;
        }
    }

    if (node1 == NULL) { // key not found
        Log("Receiver not found\n");
        pthread_mutex_unlock(&mutex_list);
        return -1;
    } 

    if (node2 == NULL) { // key not found
        Log("Sender not found\n");
        pthread_mutex_unlock(&mutex_list);
        return -1;
    } 

    int response = storeMessage(&(node1->listMessages), aliasSender, message, identifier); 
    if(response==-1){
        Log("Error when storing the message");
        pthread_mutex_unlock(&mutex_list);
        return -1;
    }
    pthread_mutex_unlock(&mutex_list);
    return 0;
}

int ipPortInfo(List* l, char* aliasSender, char* IP, int* port){
        /*
    Connects the user with a specific alias
    */
    
    // traverse the list
    pthread_mutex_lock(&mutex_list);
    
    List aux = *l;  // head
    while (aux != NULL) {
        if (strcmp(aux->alias,aliasSender)==0) {  // found
            strcpy(IP, aux->IP);
            *port = aux->port;
        }else {  // next element
            aux = aux->next;
        }
    }
    pthread_mutex_unlock(&mutex_list);

    Log("Element not found\n");

    return 1;  // not found
}

int sendMessageDeliver(List* l, char* aliasSender, char* aliasReceived, char* message, int identifier, char* IP, int* port){
    // traverse the list
    pthread_mutex_lock(&mutex_list);
    
    List aux = *l;
    
    List node1 = NULL;
    while (aux != NULL) {
        if (strcmp(aux->alias,aliasReceived)==0) {
            if(aux->connected==false){
                Log("User not connected\n");
                pthread_mutex_unlock(&mutex_list);
                return -1;
            }
            node1 = aux;
        }
        aux = aux->next;
        if (node1 != NULL) {
            break;
        }
    }

    if (node1 == NULL) { // key not found
        Log("User not found\n");
        pthread_mutex_unlock(&mutex_list);
        return -1;
    } 

    char* alSender = (char*) malloc(MAX_CHAR);
    char* mes = (char*) malloc(MAX_CHAR);
    int response = getMessage(&(node1->listMessages), alSender, mes, identifier);
    if (response == 0){
        strcpy(aliasSender, alSender);
        strcpy(message, mes);
        strcpy(IP, node1->IP);
        *port = node1->port;
        pthread_mutex_unlock(&mutex_list);
        return 0;
    }else{
        Log("Message not found\n");
        pthread_mutex_unlock(&mutex_list);
        return -1;
    }
}

int numberConnected(List* l, int* connections){
    pthread_mutex_lock(&mutex_list);
    int number = 0;
    List aux = *l;

    while (aux!=NULL){
        if (aux->connected == true){
            number+=1;
        }
        aux = aux->next;
    }

    *connections = number;

    pthread_mutex_unlock(&mutex_list);
    return 0;
}

int listConnected(List* l, char** users){
    pthread_mutex_lock(&mutex_list);
    int number = 0;
    List aux = *l;

    while (aux!=NULL){
        if (aux->connected == true){
            strcpy(users[number],aux->username);
        }
        aux = aux->next;
    }

    pthread_mutex_unlock(&mutex_list);
    return 0;
}

int isConnected(List *l, char* IP){
    pthread_mutex_lock(&mutex_list);    
    List aux = *l;  // head
    while (aux != NULL) {
        if (strcmp(aux->IP,IP)==0) {
            
            if (aux->connected==true){
                pthread_mutex_unlock(&mutex_list);
                return 1;
            }else{
                pthread_mutex_unlock(&mutex_list);
                return 0;
            }
        }
        else {  // next element
            aux = aux->next;
        }
    }

    Log("User not found\n");
    pthread_mutex_unlock(&mutex_list);
    return -1;
}

int isConnectedUsername(List *l, char* alias){
    pthread_mutex_lock(&mutex_list);    
    List aux = *l;  // head
    while (aux != NULL) {
        if (strcmp(aux->alias,alias)==0) {
            
            if (aux->connected==true){
                pthread_mutex_unlock(&mutex_list);
                return 1;
            }else{
                pthread_mutex_unlock(&mutex_list);
                return 0;
            }
        }
        else {  // next element
            aux = aux->next;
        }
    }

    Log("User not found\n");
    pthread_mutex_unlock(&mutex_list);
    return -1;
}

int confirmReceived(List *l, char* aliasReceiver, int identifier){
    // traverse the list
    pthread_mutex_lock(&mutex_list);
    
    List aux = *l;
    
    List node1 = NULL;
    while (aux != NULL) {
        if (strcmp(aux->alias,aliasReceiver)==0) {
            node1 = aux;
        }
        aux = aux->next;
        if (node1 != NULL) {
            break;
        }
    }

    if (node1 == NULL) { // key not found
        Log("User not found\n");
        pthread_mutex_unlock(&mutex_list);
        return -1;
    } 

    int response = notifyReceived(&(node1->listMessages), identifier);
    if(response == -1){
        Log("Error when setting a message as sent\n");
        pthread_mutex_unlock(&mutex_list);

    }
    node1->lastIDSent = identifier;
    pthread_mutex_unlock(&mutex_list);
    return 0;
}

int destroyList(List *l){
    /*
    Elimina todos los elementos de la lista l.
    */

    List aux = *l;

    pthread_mutex_lock(&mutex_list);

    while (aux != NULL) {
        List tmp = aux;
        aux = aux->next;
        free(tmp->username);
        free(tmp->alias);
        free(tmp->datetime);
        free(tmp->IP);
        destroyMessages(&(tmp->listMessages));
        free(tmp);
    }

    pthread_mutex_unlock(&mutex_list);

    pthread_mutex_destroy(&mutex_list);

    return 0;
}