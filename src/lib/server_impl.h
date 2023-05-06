/*
Definition of server implementation
*/

#ifndef _SERVER_IMPL_H_
#define _SERVER_IMPL_H_


int init();
int connect_user(char* alias, char* IP, int port, int* nonSent, int* lastSent);
int disconnect_user(char* username, char* IP);
int register_user(char* username, char* alias, char* datetime);
int unregister_user(char* username);
int sendMessage_store(char* aliasSender, char* aliasReceived, char* message, int* identifier);
int sendMessage_deliver(char* aliasSender, char* aliasReceived, char* message, int identifier, char* IP, int* port);
int confirm_received(char* receiver, int identifier);
int userConnected(char* IP);
int userConnectedUsername(char* username);
int connectedUsers(char** users);
int unregister_user(char* username);
int seeNumberConnected(int* connections);
void destroy();
int getIPPort(char* aliasSender, char* IP, int* port);

#endif