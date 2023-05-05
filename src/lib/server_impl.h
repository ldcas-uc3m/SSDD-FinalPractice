/*
Definition of server implementation
*/

#ifndef _SERVER_IMPL_H_
#define _SERVER_IMPL_H_


int init();
int connect_user(char* alias, char* IP, int port, int* nonSent, int* lastSent);
int disconnect_user(char* username);
int register_user(char* username, char* alias, char* datetime, char* IP, int port);
int unregister_user(char* username);
int sendMessage_store(char* aliasSender, char* aliasReceived, char* message, int* identifier);
int sendMessage_deliver(char* aliasSender, char* aliasReceived, char* message, int identifier, char* IP, int* port);
int confirm_received(char* receiver, int identifier);
int userConnected(char* username);
int connectedUsers(int* connections, char** users);
int unregister_user(char* username);
void destroy();

#endif