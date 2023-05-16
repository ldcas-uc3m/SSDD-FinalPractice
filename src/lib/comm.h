/*
Definición de las estructuras comunes
*/

#ifndef _COMM_H_
#define _COMM_H_


#include <netinet/in.h>

#define MAX_CHAR 256

struct thread_params {
    int sd;
    struct sockaddr_in client;
};

#endif

