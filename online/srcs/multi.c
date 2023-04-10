/** ====================================================================
**   Auteur  : GENY                   | Date    : 09/03/2022
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux
**  --------------------------------------------------------------------
**   Nom fichier : multi.c            | Version : 1.0
**  --------------------------------------------------------------------
**   Description : fonctions pour communiquer entre deux multi-joueur
**                 via le reseaux en TCP
** =====================================================================*/

#include "../includes/multi.h"

/*
init_srv permet d'initialise la structure multi_t par default, les structures
d'adresse seront initialise a 0 et les socket a -1

input : none

output: _ERR_ALLOC_ -> si probleme d'allocation memoire
        addr struct -> si pas d'erreur
*/
multi_t *init_srv()
{
    multi_t *new_net = NULL;
    int      i = -1;

    new_net = malloc(sizeof(multi_t));
    if (new_net == NULL) // si probleme d'allocation de memoire
        return (multi_t*)_ERR_ALLOC_;

    /* inite de la structure par default */
    new_net->srv.sock_srv = -1;
    new_net->isPlaying = 1;
    new_net->filesSize = 0;
    new_net->my_id     = -1;
    bzero(&(new_net->srv.srv_addr), sizeof(struct sockaddr_in));
    bzero(new_net->srv.name, sizeof(char)*30);
    while (++i < 3)
    {
        new_net->connexion[i].sockfd = -1;
        new_net->connexion[i].port = -1;
        bzero(&(new_net->connexion[i].clt_addr), sizeof(struct sockaddr_in));
        bzero(new_net->connexion[i].name, sizeof(char)*30);
        bzero(new_net->connexion[i].frame, sizeof(char)*BUFFER);
        new_net->connexion[i].id = -1;
    }
    FD_ZERO(&(new_net->readfds));
    new_net->max_sd = 0;
    return new_net;
}

/*
connect_to_server permet de se connecter au serveur distant qui permet de jouer
en multi-joueur

input : multi_t ** -> structure contenant les connections reseau multi-joueur
        char *     -> addresse ip du serveur
        int        -> port de connection du serveur

output: _NOT_INIT_   -> si la structure n'est pas initialise
        _ERR_SOCKET_ -> si une erreur de connection est survenue
        _SUCCESS_    -> si pas d'erreur
*/
int connect_to_server(multi_t **net, char *ip_addr, int port)
{
    if (*net == NULL)
        return _NOT_INIT_;

    /* renseigner les informations du serveur */
    (*net)->connexion[0].clt_addr.sin_family      = AF_INET;
    (*net)->connexion[0].clt_addr.sin_addr.s_addr = inet_addr(ip_addr);
    (*net)->connexion[0].clt_addr.sin_port        = htons(port);

    (*net)->connexion[0].sockfd = socket(AF_INET, SOCK_STREAM, 0);
    (*net)->connexion[0].port = port;
    if ((*net)->connexion[0].sockfd < 0) // si erreur d'ouverture de socket
        return _ERR_SOCKET_;

    if ( connect((*net)->connexion[0].sockfd,
                 (struct sockaddr*)&((*net)->connexion[0].clt_addr),
                 sizeof(struct sockaddr)) < 0 ) // tentative de connection
        return _ERR_SOCKET_;
    return _SUCCESS_;
}

/*
connect_to_other_srv permet de se connecter a  un serveur qui est deja connecte
au serveur d'origine

input : multi_t ** -> structure contenant les connections reseau multi-joueur
        char *     -> addresse ip du serveur
        int        -> port de connection du serveur

output: _NOT_INIT_        -> si la structure n'est pas initialise
        _ERR_SOCKET_      -> si une erreur de connection est survenue
        _ERR_NOT_CONNECT_ -> si la connection ne peut etre etablie par manque de
                             place dans la structure
        _SUCCESS_         -> si pas d'erreur
*/
int connect_to_other_srv(multi_t **net, char *ip_addr, char *port)
{
    int i = -1;

    if (*net == NULL)
        return _NOT_INIT_;

    while (++i < 3)
    {
        if ((*net)->connexion[i].sockfd < 0) /* si une place memoire est disponible */
        {
            (*net)->connexion[i].clt_addr.sin_family      = AF_INET;
            (*net)->connexion[i].clt_addr.sin_addr.s_addr = atoi(ip_addr);
            (*net)->connexion[i].clt_addr.sin_port        = htons(atoi(port));

            (*net)->connexion[i].sockfd = socket(AF_INET, SOCK_STREAM, 0);
            (*net)->connexion[i].port = atoi(port);
            if ((*net)->connexion[i].sockfd < 0) // si erreur d'ouverture de socket
                return _ERR_SOCKET_;
            if ( connect((*net)->connexion[i].sockfd,
                 (struct sockaddr*)&((*net)->connexion[i].clt_addr),
                 sizeof(struct sockaddr)) < 0 ) // tentative de connection
                return _ERR_SOCKET_;
            send_id((*net), i);
            send_name((*net), i); // envoie du nom au serveur
            send_port((*net), i); // on donne le port du nouveaux serveur au serveur
            return _SUCCESS_;
        }
    }
    return _ERR_NOT_CONNECT_;
}

/*
send_name permet d'envoyer son nom dans le jeu au une connexion reseaux
dont l'indice est le destinataire base sur la structure multi_t

input : multi_t * -> structure contenant les connections reseau multi-joueur
        int       -> indice qui precise une connexion

output: _NOT_INIT_        -> si la structure n'est pas initialise
        _ERR_NOT_CONNECT_ -> si la connecion n'est pas etablie sur cette indice
        _SUCCESS_         -> si pas d'erreur
*/
int send_name(multi_t *net, int indice)
{
    char message[BUFFER];
    int i;

    if (net == NULL)
        return _NOT_INIT_;

    if (net->connexion[indice].sockfd == -1)
        return _ERR_NOT_CONNECT_;
    /* Creation du message a envoyer */
    bzero(message, sizeof(char)*BUFFER);
    strcat(message, "i{n{");
    strcat(message, net->srv.name);
    strcat(message, "}}");

    send(net->connexion[indice].sockfd, message, strlen(message), 0);
    return _SUCCESS_;
}

/*
send_name permet d'envoyer son port a une connexion reseaux
dont l'indice est le destinataire base sur la structure multi_t

input : multi_t * -> structure contenant les connections reseau multi-joueur
        int       -> indice qui precise une connexion

output: _NOT_INIT_        -> si la structure n'est pas initialise
        _ERR_NOT_CONNECT_ -> si la connecion n'est pas etablie sur cette indice
        _SUCCESS_         -> si pas d'erreur
*/
int send_port(multi_t *net, int indice)
{
    char message[BUFFER];
    char port[10];
    int i;

    if (net == NULL)
        return _NOT_INIT_;

    if (net->connexion[indice].sockfd == -1)
        return _ERR_NOT_CONNECT_;
    /* Creation du message a envoyer */
    bzero(message, sizeof(char)*BUFFER);
    bzero(port, sizeof(char)*10);
    ft_itoa(ntohs(net->srv.srv_addr.sin_port), port, 10);

    strcat(message, "i{p{");
    strcat(message, port);
    strcat(message, "}}");

    send(net->connexion[indice].sockfd, message, strlen(message), 0);
    return _SUCCESS_;
}

int send_id(multi_t *net, int indice)
{
    char message[BUFFER];
    char id[10];

    if (net == NULL)
        return _NOT_INIT_;
    if (net->connexion[indice].sockfd == -1)
        return _ERR_NOT_CONNECT_;
    /* Creation du message a envoyer */
    bzero(message, sizeof(char)*BUFFER);
    bzero(id, sizeof(char)*10);
    ft_itoa(net->my_id, id, 10);

    strcat(message, "i{i{");
    strcat(message, id);
    strcat(message, "}}");

    send(net->connexion[indice].sockfd, message, strlen(message), 0);
    return _SUCCESS_;
}

/*
set_name permet d'ajouter un nom au joueur pour pouvoir le partage au autres
joueur connecte en reseau

input : multi_t ** -> structure contenant les connections reseau multi-joueur
        char *     -> nom du joueur

output: _NOT_INIT_ -> si la structure n'est pas initialise
        _SUCCESS_  -> si pas d'erreur
*/
int set_name(multi_t **net, char *new_name)
{
    if ((*net) == NULL)
        return _NOT_INIT_;

    strcpy((*net)->srv.name, new_name);
    return _SUCCESS_;
}

/*
send_message permet d'envoyer un message a une connection TCP etablie

input : multi_t * -> structure des connections
        char *    -> message a envoyer
        int       -> indice de la connection a envoyer le message

output: _NOT_INIT_ -> si la structure n'est pas initialise
        _SUCCESS_  -> si pas d'erreur
*/
int send_message(multi_t *net, char *message, int indice)
{
    if (net == NULL)
        return _NOT_INIT_;

    send(net->connexion[indice].sockfd, message, strlen(message), 0);
    return _SUCCESS_;
}

/*
send_message permet d'envoyer un message a toutes les connexions qui
sont etablie

input : multi_t * -> structure des connections
        char *    -> message a envoyer

output: _NOT_INIT_ -> si la structure n'est pas initialise
        _SUCCESS_  -> si pas d'erreur
*/
int send_all_message(multi_t *net, char *message)
{
    int i = -1;
    if (net == NULL)
        return _NOT_INIT_;

    while (++i < 3)
        if (net->connexion[i].sockfd > 0)
            send(net->connexion[i].sockfd, message, strlen(message), MSG_DONTWAIT);
    return _SUCCESS_;
}

/*
strart_server permet de demarer un serveur TCP pour que les autres utilisateur puisse
se connecte. La fonction va chercher un port qui n'est pas utilise pour pouvoir ouvrir
le serveur

input : multi_t ** -> structure contenant les connections reseau multi-joueur

output: _NOT_INIT_   -> si la structure n'est pas initialise
        _ERR_SOCKET_ -> si probleme de socket
        _SUCCESS_    -> si pas d'erreur
*/
int start_server(multi_t **net)
{
    int port = 1024;
    int i, y;

    if ((*net) == NULL)
        return _NOT_INIT_;

    /* renseigner les informations du serveur */
    (*net)->srv.srv_addr.sin_family = AF_INET;
    (*net)->srv.srv_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    // chercher un port non utilise
    do
    {
        (*net)->srv.sock_srv = socket(AF_INET, SOCK_STREAM, 0);
        if ((*net)->srv.sock_srv < 0)
            return _ERR_SOCKET_;
        (*net)->srv.srv_addr.sin_port = 0;
        (*net)->srv.srv_addr.sin_port = htons(port);
        i = bind((*net)->srv.sock_srv,
                 (struct sockaddr*)&((*net)->srv.srv_addr),
                 sizeof(struct sockaddr));
        y = listen((*net)->srv.sock_srv, 3);
        if (i < 0 || y < 0)
            close((*net)->srv.sock_srv);
    } while ((i < 0 || y < 0) && (++port < 65535));

    if (i < 0 || y < 0) // si tous les port ont etais essaye
        return _ERR_SOCKET_;
    return _SUCCESS_;
}

/*
init_select_multi permet de remplir les informations necessaire pour utilise
la fonction select qui permet de voir si il y a des informations recu en
reseau

input : multi_t **  -> structure contenant les connections reseau multi-joueur
        net_ipc_t * -> structure contenant la connection local avec python

output: _SUCCESS_ -> si pas d'erreur
*/
int init_select_multi(multi_t **net, net_ipc_t *ipc)
{
    int sd;
    int i=-1;

    FD_ZERO(&((*net)->readfds));

    FD_SET(ipc->sock_ipc, &((*net)->readfds));
    (*net)->max_sd = ipc->sock_ipc;

    FD_SET((*net)->srv.sock_srv, &((*net)->readfds));
    sd = (*net)->srv.sock_srv;
    if (sd > (*net)->max_sd)
        (*net)->max_sd = sd;

    while (++i < 3)
    {
        if ((*net)->connexion[i].sockfd > 0)
        {
            sd = (*net)->connexion[i].sockfd;
            if (sd > 0)
                FD_SET(sd, &((*net)->readfds));
            if(sd > (*net)->max_sd)
                (*net)->max_sd = sd;
        }
    }

    return _SUCCESS_;
}

/*
add_new_connection permet d'ajouter une nouvelle connection si nous avons moins de 
trois connection initie

input : multi_t **  -> structure contenant les connections reseau multi-joueur
        int         -> socket ouverte pour cette connection
        struct sockaddr_in -> information de la nouvelle connection

output: _NOT_INIT_    -> si la structure multi_t n'est pas initialise
        _CANT_ACCEPT_ -> si le nombre de connection max est atteinte
        connection index -> si pas d'erreur
*/
int add_new_connection (multi_t **net, int new_socket, struct sockaddr_in new_addr)
{
    int i = -1;

    if ((*net) == NULL)
        return _NOT_INIT_;

    while (++i < 3)
    {
        if ((*net)->connexion[i].sockfd == -1)
        {
            (*net)->connexion[i].sockfd = new_socket;
            (*net)->connexion[i].clt_addr = new_addr;
            return i;
        }
    }
    return _CANT_ACCEPT_;
}

/*
del_connection permet de supprimer les informations d'un connection qui a etais
interompu

input : multi_t ** -> structure des connections
        int        -> index de la connection qui a etais interompu

output: _NOT_INIT_ -> si la structure n'est pas initialise
        _SUCCESS_  -> si pas d'erreur
*/
int del_connection (multi_t **net, int index)
{
    if ((*net) == NULL)
        return _NOT_INIT_;

    (*net)->connexion[index].sockfd = -1;
    (*net)->connexion[index].port = -1;
    (*net)->connexion[index].id = -1;
    bzero((*net)->connexion[index].name, sizeof(char)*30);
    bzero(&((*net)->connexion[index].clt_addr), sizeof(struct sockaddr_in));
    return _SUCCESS_;
}

/*
del_net permet d'arret l'integralite des connections TCP

input : multi_t ** -> structure des connections

output: _NOT_INIT_ -> si la structure n'est pas initialise
        _SUCCESS_  -> si pas d'erreur
*/
int del_net(multi_t **net)
{
    int i = -1;

    if ((*net) == NULL)
        return _NOT_INIT_;

    while (++i < 3)
    {
        if ((*net)->connexion[i].sockfd > 0) /* fermeture des connections ouvertes */
        {
            send_message((*net), _STOP_COMM_TCP_, i);
            shutdown((*net)->connexion[i].sockfd, SHUT_RDWR);
            del_connection(&(*net), i);
        }
    }
    free((*net));
    (*net) = NULL;
    return _SUCCESS_;
}

int get_id_player(multi_t *net)
{
    if(net == NULL)
        return _NOT_INIT_;
    /*
    Demender au serveur un id
    */
    return _SUCCESS_;
}