#include "../includes/netipc.h"

net_ipc_t *init_ipc(int port)
{
    net_ipc_t *new_ipc = NULL;

    new_ipc = malloc(sizeof(net_ipc_t));
    if (new_ipc == NULL)
        return (net_ipc_t *)_ERR_ALLOC_;

    new_ipc->port_srv = port;
    bzero(&(new_ipc->serv_addr), sizeof(struct sockaddr_in));

    new_ipc->sock_ipc = socket(AF_INET, SOCK_DGRAM, 0);
    if (new_ipc->sock_ipc < 0)
        return (net_ipc_t *)_ERR_SOCKET_;

    new_ipc->serv_addr.sin_family = AF_INET;
    new_ipc->serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    new_ipc->serv_addr.sin_port = htons(new_ipc->port_srv);
    new_ipc->len = (socklen_t)sizeof(new_ipc->serv_addr);

    return new_ipc;
}

int del_ipc(net_ipc_t *ipc, char *retour)
{
    if (ipc == NULL)
        return _NOT_INIT_;
    send_ipc(ipc, retour);
    free(ipc);
    return _SUCCESS_;
}

int send_ipc(net_ipc_t *ipc, const char *message)
{
    int i;

    if (ipc == NULL)
        return _NOT_INIT_;

    i = sendto(ipc->sock_ipc,
               (char *)message,
               sizeof(char) * strlen(message),
               MSG_DONTWAIT,
               (struct sockaddr *)&(ipc->serv_addr),
               ipc->len);

    if (i < 0)
        return _ERR_SEND_;

    return _SUCCESS_;
}

int send_ipc_port(net_ipc_t *ipc, int port)
{
    char buf[1024];
    char tmp[10];

    bzero(buf, sizeof(char) * 1024);
    bzero(tmp, sizeof(char) * 10);
    ft_itoa(port, tmp, 10);
    strcpy(buf, "i{p{");
    strcat(buf, tmp);
    strcat(buf, "}}");
    send_ipc(ipc, buf);
    return _SUCCESS_;
}

int recv_ipc(net_ipc_t *ipc, char *dest)
{
    int i;

    if (ipc == NULL)
        return _NOT_INIT_;

    i = recvfrom(ipc->sock_ipc,
                 dest,
                 sizeof(char) * BUFFER,
                 MSG_DONTWAIT,
                 (struct sockaddr *)&(ipc->serv_addr),
                 &(ipc->len));

    if (i < 0)
        return _ERR_RECV_;

    return _SUCCESS_;
}