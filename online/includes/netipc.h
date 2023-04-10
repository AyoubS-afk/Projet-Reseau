/** ====================================================================
**   Auteur  : GENY                   | Date    : 09/03/2022
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux
**  --------------------------------------------------------------------
**   Nom fichier : netipc.h           | Version : 1.0
**  --------------------------------------------------------------------
**   Description : header du fichier netipc.c
** =====================================================================*/

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
	int sock_ipc;                  // socket qui serrat utilise pour communiquer avec le python
	int port_srv;                  // port du serveur du processuce python
	socklen_t len;                 // taille de la socket
	struct sockaddr_in serv_addr;  // information pour se connecter au serveur
} net_ipc_t;

/*
*** Declaration des fonctions
*/
net_ipc_t *init_ipc(int port);
int        del_ipc(net_ipc_t *ipc, char *retour);
int        send_ipc(net_ipc_t *ipc, const char *message);
int        send_ipc_port(net_ipc_t *ipc, int port);
int        recv_ipc(net_ipc_t *ipc, char *dest);

#endif