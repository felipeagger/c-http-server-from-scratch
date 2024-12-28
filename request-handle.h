#ifndef REQUEST_HANDLE_H
#define REQUEST_HANDLE_H

#include "threadpool.h"

void handle_request(TaskArgs* args); //(int client_fd, char *buffer);

#endif