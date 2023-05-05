#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "lib/server_impl.h"
#include "lib/comm.h"
#include "lib/lines.h"

#include "log.h"


// sync petitions
pthread_mutex_t mutex_sd;  // mutex for petition
pthread_cond_t c_sd;  // variable condicional de bloqueo
bool copiado = false;  // variable condicional de control

// shutdown_server
bool shutdown_server = false;
pthread_mutex_t mutex_shutdown_server;

int sd;


void sigintHandler(int sig_num) {
    // signal handler for SIGINT

    pthread_mutex_lock(&mutex_shutdown_server);
    shutdown_server = true;
    pthread_mutex_unlock(&mutex_shutdown_server);

    printf("\nShutting down...\n");

    close(sd);
    destroy();

	pthread_cond_destroy(&c_sd);
	pthread_mutex_destroy(&mutex_sd);
	pthread_mutex_destroy(&mutex_shutdown_server);

    fflush(stdout);

	exit(0);
}


void *tratar_peticion(int* sd) {
    struct Peticion pet;
    char buffer[MAX_LINE];

    // copy sd
	pthread_mutex_lock(&mutex_sd);

    int local_sd = *sd;  // copy sd

    copiado = true;  // update conditional variable
	pthread_cond_signal(&c_sd);  // awake main
	pthread_mutex_unlock(&mutex_sd);


    // read opcode
    readLine(local_sd, buffer, MAX_LINE);
    pet.opcode = atoi(buffer);

    // treat petition
    struct Respuesta res;
    

    switch (pet.opcode) {
        case 0:  // init
            Log("Client %i: Received init\n", local_sd);

            // execute
            res.result = init();

            // answer
            sprintf(buffer, "%i", res.result);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);

            // pthread_mutex_lock(&mutex_stdout);
            // printf("Sent: {result: %s}\n", buffer);
            // pthread_mutex_unlock(&mutex_stdout);
            
            break;

        case 1:  // set
            // get arguments
            readLine(local_sd, buffer, MAX_LINE);
            pet.value.clave = atoi(buffer);

            readLine(local_sd, &(pet.value.value1), MAX_LINE);

            readLine(local_sd, buffer, MAX_LINE);
            pet.value.value2 = atoi(buffer);
            Log("Clave %i | Value 1 %s | Value 2 %i \n", pet.value.clave, pet.value.value1, pet.value.value2);

            readLine(local_sd, buffer, MAX_LINE);
            pet.value.value3 = atof(buffer);

            Log("Client %i: Received {opcode: %i (set), key: %i, value1: %s, value2: %i, value3: %f}\n", local_sd, pet.opcode, pet.value.clave, pet.value.value1, pet.value.value2, pet.value.value3);
            // execute
            res.result = set(pet.value.clave, pet.value.value1, pet.value.value2, pet.value.value3);

            // answer
            sprintf(buffer, "%i", res.result);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);
            
            break;

        case 2:  // get
            
            // get arguments
            readLine(local_sd, buffer, MAX_LINE);
            pet.value.clave = atoi(buffer);

            Log("Client %i: Received {opcode: %i (get), key: %i}\n", local_sd, pet.opcode, pet.value.clave);

            // execute
            res.result = get(pet.value.clave, res.value.value1, &(res.value.value2), &(res.value.value3));

            // answer
            sprintf(buffer, "%i", res.result);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);

            sendMessage(local_sd, res.value.value1, strlen(res.value.value1) + 1);

            sprintf(buffer, "%i", res.value.value2);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);
            
            sprintf(buffer, "%f", res.value.value3);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);

            break;
        
        case 3:  // modify
            // get arguments
            readLine(local_sd, buffer, MAX_LINE);
            pet.value.clave = atoi(buffer);

            readLine(local_sd, &(pet.value.value1), MAX_LINE);

            readLine(local_sd, buffer, MAX_LINE);
            pet.value.value2 = atoi(buffer);

            readLine(local_sd, buffer, MAX_LINE);
            pet.value.value3 = atof(buffer);

            Log("Modify\n");

            Log("Client %i: Received {opcode: %i (modify), key: %i, value1: %s, value2: %i, value3: %f}\n", local_sd, pet.opcode, pet.value.clave, pet.value.value1, pet.value.value2, pet.value.value3);

            // execute
            res.result = modify(pet.value.clave, pet.value.value1, pet.value.value2, pet.value.value3);

            // answer
            sprintf(buffer, "%i", res.result);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);

            break;
        
        case 4:  // exist
            // get arguments
            readLine(local_sd, buffer, MAX_LINE);
            pet.value.clave = atoi(buffer);

            Log("Client %i: Received {opcode: %i (exist), key: %i}\n", local_sd, pet.opcode, pet.value.clave);

            // execute
            res.result = exist(pet.value.clave);

            // answer
            sprintf(buffer, "%i", res.result);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);

            break;

        case 5:  // copyKey

            // get arguments
            readLine(local_sd, buffer, MAX_LINE);
            pet.value.clave = atoi(buffer);
            
            readLine(local_sd, buffer, MAX_LINE);
            pet.alt_key = atoi(buffer);

            Log("Client %i: Received {opcode: %i (copy_key), key: %i, alt_key: %i}\n", local_sd, pet.opcode, pet.value.clave, pet.alt_key);

            // execute
            res.result = copy_key(pet.value.clave, pet.alt_key);

            // answer
            sprintf(buffer, "%i", res.result);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);

            break;

        case 6: //deleteKey
            // get arguments
            readLine(local_sd, buffer, MAX_LINE);
            pet.value.clave = atoi(buffer);

            Log("Client %i: Received {opcode: %i (delete_key), key: %i}\n", local_sd, pet.opcode, pet.value.clave);

            // execute
            res.result = delete_key(pet.value.clave);

            // answer
            sprintf(buffer, "%i", res.result);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);
            break;

        default:
            res.result = -1;
            Log("Undefined operation code\n");

            // answer
            sprintf(buffer, "%i", res.result);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);

            break;
    }

    // print_list();
    
    close(local_sd);

    pthread_exit(NULL);
}


int main(int argc, char* argv[]) {
    int newsd;
    socklen_t size;
    struct sockaddr_in server_addr, client_addr;

    LogInit();

    // Iniciar estructuras de datos del servidor
    init();


    // signal handler
    signal(SIGINT, sigintHandler);


    // thread stuff
    pthread_attr_t t_attr;
    pthread_t thid;


    // init mutex and cond
	pthread_cond_init(&c_sd, NULL);
	pthread_mutex_init(&mutex_sd, NULL);
	pthread_mutex_init(&mutex_shutdown_server, NULL);

	pthread_attr_init(&t_attr);
    pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);


    // configure socket
    if ((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("Error in socket");
        exit(1);
    }

    int val = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*) &val, sizeof(int)) < 0) {
        perror("Error in option");
        exit(1);
    }
    
    bzero((char*) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;


    // bind
    if (bind(sd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error in bind\n");
        return -1;
    }

    /* MAIN LOOP */
    listen(sd, SOMAXCONN);  // open socket
    while (true) {

        // accept petition
        newsd = accept(sd, (struct sockaddr*) &client_addr, &size);
        if (newsd < 0) {
            perror("Error in accept\n");
            return -1;
        }
        // printf("Client %i connected\n", newsd);

        // create thread
        if (pthread_create(&thid, &t_attr, (void*) tratar_peticion, (void*) &newsd) == 0) {  // wait to copy petition
            // mutex logic
            pthread_mutex_lock(&mutex_sd);

            while (!copiado) {  // wait for thread to copy petition
                pthread_cond_wait(&c_sd, &mutex_sd);
            }

            copiado = false;  // reset variable
            pthread_mutex_unlock(&mutex_sd);

        } else {
            perror("Error al crear el thread\n");
            return -1;
        }
        
        // exit condition
        pthread_mutex_lock(&mutex_shutdown_server);
        if (shutdown_server) {
            pthread_mutex_unlock(&mutex_shutdown_server);
            break;
        }
        pthread_mutex_unlock(&mutex_shutdown_server);
    }

    // cleanup
    close(sd);
    destroy();

	pthread_cond_destroy(&c_sd);
	pthread_mutex_destroy(&mutex_sd);
	pthread_mutex_destroy(&mutex_shutdown_server);

	exit(0);
}

