

#ifndef __NETIPC_H_
#define __NETIPC_H_

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "../includes/protocoleIPC.h"
#include "../includes/return.h"
#include "../includes/itoa.h"

#define BUFFER 1024

typedef struct
{
	int sock_ipc;
	int port_srv;
	socklen_t len;
	struct sockaddr_in serv_addr;
} net_ipc_t;

net_ipc_t *init_ipc(int port);
int del_ipc(net_ipc_t *ipc, char *retour);
int send_ipc(net_ipc_t *ipc, const char *message);
int send_ipc_port(net_ipc_t *ipc, int port);
int recv_ipc(net_ipc_t *ipc, char *dest);

#endif