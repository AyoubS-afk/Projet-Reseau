/** ====================================================================
**   Auteur  : GENY                   | Date    : 09/03/2022
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux
**  --------------------------------------------------------------------
**   Nom fichier : netipc.c           | Version : 1.0
**  --------------------------------------------------------------------
**   Description : fonctions pour communiquer entre deux processuce
**                 via le reseaux localhost
** =====================================================================*/

#include "../includes/netipc.h"

/*
init_ipc permet de generer la structure net_ipc_t qui permet la communication
entre deux processuce en UDP

input : int port -> port du serveur du processuce

output: _ERR_ALLOC_  -> probleme d'allocation de memoire
        _ERR_SOCKET_ -> probleme pour creer une socket
        _ERR_BIND_   -> probleme lors de la fonction bind
        addr_struct  -> si pas d'erreur
*/
net_ipc_t *init_ipc(int port)
{
    net_ipc_t *new_ipc = NULL;

    new_ipc = malloc(sizeof(net_ipc_t));
    if (new_ipc == NULL)
        return (net_ipc_t*)_ERR_ALLOC_;

    new_ipc->port_srv = port;
    bzero(&(new_ipc->serv_addr), sizeof(struct sockaddr_in));

    new_ipc->sock_ipc = socket(AF_INET, SOCK_DGRAM, 0); // ouverture d'une socket
    if (new_ipc->sock_ipc < 0) // si erreur a l'ouverture
        return (net_ipc_t*)_ERR_SOCKET_;

    // initialisation de la structure sockaddr_in
    new_ipc->serv_addr.sin_family = AF_INET;
    new_ipc->serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    new_ipc->serv_addr.sin_port = htons(new_ipc->port_srv);
    new_ipc->len = (socklen_t)sizeof(new_ipc->serv_addr);

    return new_ipc;
}

/*
del_ipc permet de liberer la memoire de la structure net_ipc_t

input : net_ipc_t* -> addresse de la strcture a liberer

output: _NOT_INIT_ -> si la structure n'est pas initialise
        _SUCCESS_  -> si pas d'erreur
*/
int del_ipc(net_ipc_t *ipc, char *retour)
{
    if (ipc == NULL)
        return _NOT_INIT_;
    send_ipc(ipc, retour);
    free(ipc);
    return _SUCCESS_;
}

/*
send_ipc permet d'envoyer un message a un processuce connecter en resau UDP
via la structure net_ipc_t

input : net_ipc_t *  -> structure de la connection
        const char * -> message a envoyer

output: _NOT_INIT_ -> si la structure n'est pas initialise
        _ERR_SEND_ -> si une erreur c'est produit lors de l'envoie
        _SUCCESS_  -> si pas d'erreur
*/
int send_ipc(net_ipc_t *ipc, const char *message)
{
    int i;

    if (ipc == NULL) // si la structure net_ipc_t n'est pas initialise
        return _NOT_INIT_;

    i = sendto(ipc->sock_ipc,
               (char*)message,
               sizeof(char)*strlen(message),
               MSG_DONTWAIT,
               (struct sockaddr*)&(ipc->serv_addr),
               ipc->len); // envoie du message

    if (i < 0) // si une erreur lors de l'envoie du message
        return _ERR_SEND_;

    return _SUCCESS_;
}

int send_ipc_port(net_ipc_t *ipc, int port)
{
    char buf[1024];
    char tmp[10];

    bzero(buf, sizeof(char)*1024);
    bzero(tmp, sizeof(char)*10);
    ft_itoa(port, tmp, 10);
    strcpy(buf, "i{p{");
    strcat(buf, tmp);
    strcat(buf, "}}");
    send_ipc(ipc, buf);
    return _SUCCESS_;
}

/*
recv_ipc permet de recuperer un message recu par un processuce en reseau
local UDP via la structure net_ipc_t

input : net_ipc_t *  -> structure de la connection
        char *       -> destination du message recu

output: _NOT_INIT_ -> si la structure net_ipc_t n'est pas initialise
        _ERR_RECV_ -> si une erreur lors de la reception du message
        _SUCCESS_  -> si pas d'erreur
*/
int recv_ipc(net_ipc_t *ipc, char *dest)
{
    int i;

    if (ipc == NULL)
        return _NOT_INIT_;

    i = recvfrom(ipc->sock_ipc,
                 dest,
                 sizeof(char)*BUFFER,
                 MSG_DONTWAIT,
                 (struct sockaddr*)&(ipc->serv_addr),
                 &(ipc->len));

    if (i < 0)
        return _ERR_RECV_;

    return _SUCCESS_;
}