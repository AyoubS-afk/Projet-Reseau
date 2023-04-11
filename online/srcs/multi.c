#include "../includes/multi.h"

multi_t *init_srv()
{
    multi_t *new_net = NULL;
    int      i = -1;

    new_net = malloc(sizeof(multi_t));
    if (new_net == NULL)
        return (multi_t*)_ERR_ALLOC_;


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

int connect_to_server(multi_t **net, char *ip_addr, int port)
{
    if (*net == NULL)
        return _NOT_INIT_;

    (*net)->connexion[0].clt_addr.sin_family      = AF_INET;
    (*net)->connexion[0].clt_addr.sin_addr.s_addr = inet_addr(ip_addr);
    (*net)->connexion[0].clt_addr.sin_port        = htons(port);

    (*net)->connexion[0].sockfd = socket(AF_INET, SOCK_STREAM, 0);
    (*net)->connexion[0].port = port;
    if ((*net)->connexion[0].sockfd < 0) 
        return _ERR_SOCKET_;

    if ( connect((*net)->connexion[0].sockfd,
                 (struct sockaddr*)&((*net)->connexion[0].clt_addr),
                 sizeof(struct sockaddr)) < 0 ) 
        return _ERR_SOCKET_;
    return _SUCCESS_;
}

int connect_to_other_srv(multi_t **net, char *ip_addr, char *port)
{
    int i = -1;

    if (*net == NULL)
        return _NOT_INIT_;

    while (++i < 3)
    {
        if ((*net)->connexion[i].sockfd < 0)
        {
            (*net)->connexion[i].clt_addr.sin_family      = AF_INET;
            (*net)->connexion[i].clt_addr.sin_addr.s_addr = atoi(ip_addr);
            (*net)->connexion[i].clt_addr.sin_port        = htons(atoi(port));

            (*net)->connexion[i].sockfd = socket(AF_INET, SOCK_STREAM, 0);
            (*net)->connexion[i].port = atoi(port);
            if ((*net)->connexion[i].sockfd < 0)
                return _ERR_SOCKET_;
            if ( connect((*net)->connexion[i].sockfd,
                 (struct sockaddr*)&((*net)->connexion[i].clt_addr),
                 sizeof(struct sockaddr)) < 0 )
                return _ERR_SOCKET_;
            send_id((*net), i);
            send_name((*net), i); 
            send_port((*net), i); 
            return _SUCCESS_;
        }
    }
    return _ERR_NOT_CONNECT_;
}

int send_name(multi_t *net, int indice)
{
    char message[BUFFER];
    int i;

    if (net == NULL)
        return _NOT_INIT_;

    if (net->connexion[indice].sockfd == -1)
        return _ERR_NOT_CONNECT_;
    bzero(message, sizeof(char)*BUFFER);
    strcat(message, "i{n{");
    strcat(message, net->srv.name);
    strcat(message, "}}");

    send(net->connexion[indice].sockfd, message, strlen(message), 0);
    return _SUCCESS_;
}

int send_port(multi_t *net, int indice)
{
    char message[BUFFER];
    char port[10];
    int i;

    if (net == NULL)
        return _NOT_INIT_;

    if (net->connexion[indice].sockfd == -1)
        return _ERR_NOT_CONNECT_;

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
    bzero(message, sizeof(char)*BUFFER);
    bzero(id, sizeof(char)*10);
    ft_itoa(net->my_id, id, 10);

    strcat(message, "i{i{");
    strcat(message, id);
    strcat(message, "}}");

    send(net->connexion[indice].sockfd, message, strlen(message), 0);
    return _SUCCESS_;
}

int set_name(multi_t **net, char *new_name)
{
    if ((*net) == NULL)
        return _NOT_INIT_;

    strcpy((*net)->srv.name, new_name);
    return _SUCCESS_;
}

int send_message(multi_t *net, char *message, int indice)
{
    if (net == NULL)
        return _NOT_INIT_;

    send(net->connexion[indice].sockfd, message, strlen(message), 0);
    return _SUCCESS_;
}

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

int start_server(multi_t **net)
{
    int port = 1024;
    int i, y;

    if ((*net) == NULL)
        return _NOT_INIT_;

    (*net)->srv.srv_addr.sin_family = AF_INET;
    (*net)->srv.srv_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

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

    if (i < 0 || y < 0) 
        return _ERR_SOCKET_;
    return _SUCCESS_;
}

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

int del_net(multi_t **net)
{
    int i = -1;

    if ((*net) == NULL)
        return _NOT_INIT_;

    while (++i < 3)
    {
        if ((*net)->connexion[i].sockfd > 0) 
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
    return _SUCCESS_;
}