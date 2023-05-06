#include <unistd.h>

#ifndef _LINES_H_
#define _LINES_H_

#define MAX_LINE 	256


int sendMessage(int socket, char *buffer, int len);
int recvMessage(int socket, char *buffer, int len);
ssize_t readLine(int fd, void *buffer, size_t n);


#endif