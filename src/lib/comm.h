/*
Definición de las estructuras de Petición y Respuesta
*/

#ifndef _COMM_H_
#define _COMM_H_

#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CHAR 256
#define SERVER_PUERTO 4500
#define SERVER_DIR 5

struct thread_params{
    int sd;
    struct sockaddr_in client;
};

#endif

