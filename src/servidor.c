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
#include <arpa/inet.h>
#include <netdb.h>

#include "lib/server_impl.h"
#include "lib/comm.h"
#include "lib/lines.h"
#include "lib/log.h"



// sync petitions
pthread_mutex_t mutex_sd;  // mutex for petition
pthread_cond_t c_sd;  // variable condicional de bloqueo
bool copiado = false;  // variable condicional de control

// shutdown_server
bool shutdown_server = false;
pthread_mutex_t mutex_shutdown_server;

int sd;


void sigintHandler(int sig_num) {
    /*
    Signal handler for SIGINT
    */

    pthread_mutex_lock(&mutex_shutdown_server);
    shutdown_server = true;
    pthread_mutex_unlock(&mutex_shutdown_server);

    printf("\ns> Shutting down...\n");

    close(sd);
    destroy();

	pthread_cond_destroy(&c_sd);
	pthread_mutex_destroy(&mutex_sd);
	pthread_mutex_destroy(&mutex_shutdown_server);

    fflush(stdout);

	exit(0);
}


int parser(int argc, char* argv[]) {
    /*
    Simple parser. 
    Returns the client port, or -1 on error.
    */

    if ((argc != 3) || (strcmp(argv[1], "-p") != 0)) { 
        printf("Usage: %s -p <port>\n", argv[0]);
        return -1; 
    }

    int port = atoi(argv[2]);

    if ((port < 1024) || (port > 65535)) { 
        printf("Error: Server port must be in the range 1024 <= port <= 65535.\n");
        return -1; 
    }

    return port;
}


char* getHostIP() {
    /*
    Returns the host IP
    */
    char host[256];
    char *IP;
    struct hostent *host_entry;

    // find hostname
    gethostname(host, sizeof(host));

    // find host information
    host_entry = gethostbyname(host);

    IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));

    return IP;
}


int registerUserServer(int local_sd, char* alias) {

    /*
    Receives a string with the username, another with the alias and other
    with the birth date
    */

    char username[MAX_LINE];
    char datetime[MAX_LINE];

    Log("Enters function\n");

    // Receive the username

    readLine(local_sd, username, MAX_LINE);

    Log("%s\n", username);

    // Recieve the alias

    readLine(local_sd, alias, MAX_LINE);

    Log("%s,%s\n", username, alias);

    // Receive the date of birth

    readLine(local_sd, datetime, MAX_LINE);

    Log("%s,%s,%s\n", username, alias, datetime);

    // Register the user

    return register_user(username, alias, datetime);

}

int unregisterUserServer(int local_sd, char* alias) {

    /*
    Receives a string with the alias and unregisters the user
    */

    // Recieve the alias

    readLine(local_sd, alias, MAX_LINE);

    // Unregister

    return unregister_user(alias);

}

int connectUserServer(int local_sd, char* alias, int* nonSent, int* lastSent, char* IP) {

    /*
    Receives the alias and the port of connection of the user and sets
    its status as connected
    */


    // Receive the alias

    readLine(local_sd, alias, MAX_LINE);

    // Receive the port

    char buffer[MAX_LINE];

    readLine(local_sd, buffer, MAX_LINE);
    int port = atoi(buffer);
    
    return connect_user(alias, IP, port, nonSent, lastSent);
}

int disconnectUserServer(int local_sd, char* alias, char* IP) {

    /*
    Receives the alias and sets the status as disconnected
    */

    // Receive the alias

    readLine(local_sd, alias, MAX_LINE);

    // Disconnect the user

    return disconnect_user(alias, IP);

}

int sendMessageStoreServer(int local_sd, char* alias, char* alias2, int* identifier) {
    /*
    Stores a message sent from a user to the other.
    */

    // Receive the sender alias

    readLine(local_sd, alias, MAX_LINE);

    // Receive the receiver alias

    readLine(local_sd, alias2, MAX_LINE);

    // Receive the message

    char message[MAX_LINE];

    readLine(local_sd, message, MAX_LINE);

    return sendMessage_store(alias, alias2, message, identifier);

}

int connectedUsersServer(int local_sd, int* connections, char** users, char* IP) {
    /*
    Show the list of connected users to the server
    */

    int result = userConnected(IP);

    if (result == 1) {
        
        if (connectedUsers(users) == 0) {
            return 0;
        } else {
            return 2;
        }
        
    } else if (result == 0) {
        Log("User not connected\n");
        return 1;
    } else {
        return 2;
    }

}

int sendAck(char* aliasSender, int identifier) {
    /*
    
    */

    int new_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in client_addr;

    int port;
    char* IP = (char*) malloc(MAX_LINE*sizeof(char));

    Log("Llegando %d\n", identifier);

    if (getIPPort(aliasSender, IP, &port) == 0) {
        client_addr.sin_addr.s_addr = inet_addr(IP);
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(port);
        Log("Llegando2 %d\n", identifier);

        if (connect(new_socket, (struct sockaddr *) &client_addr, sizeof(client_addr)) == -1){
            Log("Error when connecting to the user\n");
            return -1;
        } else {
            char buffer[MAX_LINE];
            sprintf(buffer, "%s", "SEND_MESS_ACK");
            sendMessage(new_socket, buffer, strlen(buffer) + 1);

            sprintf(buffer, "%i", identifier);
            sendMessage(new_socket, buffer, strlen(buffer) + 1);

            if (close(new_socket) == -1) {
                return -1;
            }
        }
    } else {
        return -1;
    }

    return 0;
}

int deliver_Message(char* alias, char* aliasSender, int identifier) {
    /*
    
    */

    int new_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in client_addr;
    char message[MAX_LINE];
    int port;
    char IP[MAX_LINE];

    Log("Delivering message\n");


    if (sendMessage_deliver(aliasSender, alias, message, identifier, IP, &port) == -1) {
        Log("Error when delivering the message\n");
        return -1;
    }

    client_addr.sin_addr.s_addr = inet_addr(IP);
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);

    if (connect(new_socket, (struct sockaddr*) &client_addr, sizeof(client_addr)) == -1) {
        Log("Error when connecting to the user\n");
        disconnect_user(alias, IP);
        return -1;
    } else {
        char buffer[MAX_LINE];
        sprintf(buffer, "%s", "SEND_MESSAGE");
        sendMessage(new_socket, buffer, strlen(buffer) + 1);

        sprintf(buffer, "%s", aliasSender);
        sendMessage(new_socket, buffer, strlen(buffer) + 1);

        sprintf(buffer, "%i", identifier);
        sendMessage(new_socket, buffer, strlen(buffer) + 1);

        sprintf(buffer, "%s", message);
        sendMessage(new_socket, buffer, strlen(buffer) + 1);

        Log("Antes de aqui el error\n");


        if (sendAck(aliasSender, identifier) == -1) {
            disconnect_user(alias, IP);
            close(new_socket);
            return -1;
        }

        if (close(new_socket) == -1) {
            disconnect_user(alias, IP);
            return -1;
        }
    }

    return 0;

}

int sendRemainingMessages(int local_sd, char* alias, int nonSent, int lastSent, char* IP) {
    /*
    
    */

    for (int i = 1; i <= nonSent; i++) {
        int identifier = lastSent + i;
        char aliasSender[MAX_LINE];

        if (deliver_Message(alias, aliasSender, identifier) == 0) {
            confirm_received(alias, identifier);
        } else {
            return -1;
        }
    }

    return 0;

}

int sendListUsers(int local_sd, int connections, char** users) {
    /*
    
    */

    char buffer[MAX_LINE];

    sprintf(buffer, "%i", connections);
    sendMessage(local_sd, buffer, strlen(buffer) + 1);

    for (int i = 0; i <= connections; i++){
        sprintf(buffer, "%s", users[i]);
        sendMessage(local_sd, buffer, strlen(buffer) + 1);
    }

    return 0;

}


void *tratar_peticion(void* args) {
    struct thread_params client = *((struct thread_params*) args);
    char buffer[MAX_LINE];

    // Get ip
    struct in_addr ipAddr = client.client.sin_addr;
    char ip_client[INET_ADDRSTRLEN];
    inet_ntop( AF_INET, &ipAddr, ip_client, INET_ADDRSTRLEN );

    // copy sd
	pthread_mutex_lock(&mutex_sd);

    int local_sd = client.sd;

    copiado = true;  // update conditional variable
	pthread_cond_signal(&c_sd);  // awake main
	pthread_mutex_unlock(&mutex_sd);

    // read opcode
    char opcode[MAX_CHAR];
    readLine(local_sd, opcode, MAX_LINE);

    // treat petition
    char alias[MAX_LINE];
    Log("%s\n",opcode);
    
    if (strcmp("REGISTER", opcode) == 0) {

        Log("Entering Register\n");

        int result = registerUserServer(local_sd, alias);

        if (result == 2){
            Log("Error when registering the user\n");
            printf("REGISTER %s FAIL\n", alias);
            
        } else if (result == 0) {
            printf("REGISTER %s OK\n", alias);

        } else if (result==1) {
            printf("REGISTER %s FAIL\n", alias);
        }

        sprintf(buffer, "%i", result);
        sendMessage(local_sd, buffer, strlen(buffer) + 1);

    } else if (strcmp("UNREGISTER", opcode) == 0) {

        Log("Entering unregister\n");

        int result = unregisterUserServer(local_sd,alias);

        if (result == 2) {
            Log("Error when unregistering the user\n");
            printf("UNREGISTER %s FAIL\n", alias);
        
        } else if (result == 1) {
            printf("UNREGISTER %s FAIL\n", alias);

        } else if (result == 0){
            printf("UNREGISTER %s OK\n", alias);
        }

        sprintf(buffer, "%i", result);
        sendMessage(local_sd, buffer, strlen(buffer) + 1);

    } else if (strcmp("CONNECT", opcode) == 0) {

        Log("Entering connect\n");

        int nonSent;
        int lastSent;

        int result = connectUserServer(local_sd, alias, &nonSent, &lastSent, ip_client);

        if (result == 3){
            Log("Error when connecting the user\n");
            printf("CONNECT %s FAIL\n", alias);

        } else if (result == 1) {
            printf("CONNECT %s FAIL\n", alias);

        } else if (result == 2) {
            printf("CONNECT %s FAIL\n", alias);

        } else if (result == 0){
            printf("CONNECT %s OK\n", alias);
        }

        sprintf(buffer, "%i", result);
        sendMessage(local_sd, buffer, strlen(buffer) + 1);

        if (result == 0){
            sendRemainingMessages(local_sd, alias, nonSent, lastSent, ip_client);
        }

    } else if (strcmp("DISCONNECT", opcode) == 0) {

        Log("Entering disconnect\n");

        int result = disconnectUserServer(local_sd, alias, ip_client);

        if (result == 3) {
            Log("Error when disconnecting the user\n");
            printf("DISCONNECT %s FAIL\n", alias);

        } else if (result == 1){
            printf("DISCONNECT %s FAIL\n", alias);

        } else if (result == 2){
            printf("DISCONNECT %s FAIL\n", alias);

        } else if (result == 0) {
            printf("DISCONNECT %s OK\n", alias);
        } 

        sprintf(buffer, "%i", result);
        sendMessage(local_sd, buffer, strlen(buffer) + 1); 
    
    } else if (strcmp("SEND", opcode) == 0) {

        Log("Entering Send\n");

        char* alias2 = (char*) malloc(MAX_CHAR * sizeof(char));
        int identifier;

        int result = sendMessageStoreServer(local_sd, alias, alias2, &identifier);

        if (result == 2) {
            Log("Error when storing the message the user\n");
            printf("SEND MESSAGE FROM %s TO %s FAIL\n", alias, alias2);

        } else if (result==1) {
            printf("SEND MESSAGE FROM %s TO %s FAIL\n", alias, alias2);
        }

        sprintf(buffer, "%i", result);
        sendMessage(local_sd, buffer, strlen(buffer) + 1);
    
        if (result == 0) {
            sprintf(buffer, "%i", identifier);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);

            if (deliver_Message(alias2, alias, identifier) == -1) {
                printf("SEND MESSAGE %d FROM %s TO %s STORED\n", identifier, alias, alias2);
            } else {
                printf("SEND MESSAGE %d FROM %s TO %s\n", identifier, alias, alias2);
                confirm_received(alias2, identifier);
            }  
        }

    } else if (strcmp("CONNECTEDUSERS", opcode)==0){

        Log("Entering ConnectedUsers\n");

        int connections;
        char** users;

        if (seeNumberConnected(&connections) != -1) {
            users = (char**) malloc(sizeof(char*) * connections);
            for (int i = 0; i <= connections; i++){
                users[i] = (char*) malloc(sizeof(char)*MAX_CHAR);
            }

            int result = connectedUsersServer(local_sd, &connections, users, ip_client);
            if (result == 2) {
                Log("Error when retrieving list of connected users\n");
                printf("CONNECTEDUSERS FAIL\n");

            } else if (result == 1) {
                printf("CONNECTEDUSERS FAIL\n");

            } else if (result == 0) {
                printf("CONNECTEDUSERS OK\n");
            }

            sprintf(buffer, "%i", result);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);

            if (result == 0) {
                sendListUsers(local_sd, connections, users);
            }
        } else {
            printf("CONNECTEDUSERS FAIL\n");
            int result = 2;
            sprintf(buffer, "%i", result);
            sendMessage(local_sd, buffer, strlen(buffer) + 1);
        }

    } else {
        Log("Not a valid operation code\n");
    }
    
    close(local_sd);

    // print status symbol
    printf("s> ");
    fflush(stdout);

    pthread_exit(NULL);
}


int main(int argc, char* argv[]) {
    int newsd;
    socklen_t size;
    struct sockaddr_in server_addr, client_addr;

    // get port
    int client_port = parser(argc, argv);
    if (client_port <= 0) { return -1; }

    // print status
    printf("s> init server %s:%i\n", getHostIP(), client_port);

    printf("s> ");
    fflush(stdout);

    // Iniciar logger
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
    server_addr.sin_port = htons(client_port);
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

        struct thread_params params;

        params.sd = newsd;
        params.client = client_addr;

        // create thread
        if (pthread_create(&thid, &t_attr, (void*) tratar_peticion, (void*) &params) == 0) {  // wait to copy petition
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