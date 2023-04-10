#ifndef __MULTI_H_
#define __MULTI_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <strings.h>

#include "../includes/return.h"
#include "../includes/itoa.h"
#include "../includes/netipc.h"
#include "../includes/protocoleIPC.h"

#define BUFFER 1024


typedef struct
{
	int                sock_srv; 
	struct sockaddr_in srv_addr; 
	char               name[30]; 
} multi_srv_t;


typedef struct
{
	int                sockfd;        
	struct sockaddr_in clt_addr;      
	char               name[30];      
	int                port;          
	char               frame[BUFFER]; 
	int                id;            
} multi_connexion_t;


typedef struct
{
	multi_srv_t       srv;          
	multi_connexion_t connexion[3]; 
	fd_set            readfds;
	int               max_sd;
	int               isPlaying;
	int               filesSize;
	int               my_id;
} multi_t;


multi_t *init_srv();
int      connect_to_server(multi_t **net, char *ip_addr, int port);
int      connect_to_other_srv(multi_t **net, char *ip_addr, char *port);
int      send_name(multi_t *net, int indice);
int      send_port(multi_t *net, int indice);
int      send_id(multi_t *net, int indice);
int      set_name(multi_t **net, char *new_name);
int      start_server(multi_t **net);
int      init_select_multi(multi_t **net, net_ipc_t *ipc);
int      add_new_connection (multi_t **net, int new_socket, struct sockaddr_in new_addr);
int      del_connection (multi_t **net, int index);
int      send_message(multi_t *net, char *message, int indice);
int      send_all_message(multi_t *net, char *message);
int      del_net(multi_t **net);
int      get_id_player(multi_t *net);

#endif