/** ====================================================================
**   Auteur  : GENY                   | Date    : 09/03/2022
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux
**  --------------------------------------------------------------------
**   Nom fichier : multi.h            | Version : 1.0
**  --------------------------------------------------------------------
**   Description : header du fichier multi.c
** =====================================================================*/

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

/*
*** multi_srv_t permet de garder les informations sur le serveur qui serrat
*** ouvert par le processuce
*/
typedef struct
{
	int                sock_srv; // socket du serveur ouvert par le processuce
	struct sockaddr_in srv_addr; // information sur le serveur ouvert
	char               name[30]; // nom du joueur
} multi_srv_t;

/*
*** multi_connexion_t permet de garder les informations sur une connexion
*** etablie des clients et incluent aussi le serveur premier
*/
typedef struct
{
	int                sockfd;        // socket du client
	struct sockaddr_in clt_addr;      // information sur le client
	char               name[30];      // nom du client
	int                port;          // port de sont serveur
	char               frame[BUFFER]; // request envoyer par la connexion
	int                id;            // id in game
} multi_connexion_t;

/*
*** multi_t permet de garder les informations pour le reseau multi-joueur
*/
typedef struct
{
	multi_srv_t       srv;          // information sur le serveur qui serrat ouvert
	multi_connexion_t connexion[3]; // information sur les clients
	fd_set            readfds;
	int               max_sd;
	int               isPlaying;
	int               filesSize;
	int               my_id;
} multi_t;

/*
struct sockaddr_in
{
	sa_family_t    sin_family; // famille d'adresse : AF_INET
	uint16_t       sin_port;   // port dans l'ordre d'octets reseau
	struct in_addr sin_addr;   // adresse internet
};

struct in_addr
{
	uint32_t s_addr; // adresse dans l'ordre d'octet reseau
};
*/

/*
*** Declaration des fonctions
*/
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