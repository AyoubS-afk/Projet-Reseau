#include "../includes/string_net.h"

int clear_string(char *str)
{
    int i = -1;

    while (str[++i] != 0)
    {
        if (str[i] == "\n" ||
            str[i] == "\r")
            str[i] = 0;
    }
    return _SUCCESS_;
}

int translate_string(char *str, int index)
{
    int i = -1;
    int nb_translate = strlen(str)-index;

    while (++i < nb_translate)
        str[i] = str[index++];
    str[i] = 0;
    return _SUCCESS_;
}

int identify_message(char *str, char *dest)
{
    int i = -1, y=-1;
    int open = 0;
    int close = 0;

    bzero(dest, sizeof(char)*BUFFER);
    while (str[++i] != 0)
    {
        if (str[i] == '{')
            open++;
        if (str[i] == '}')
            close++;

        if (open > 0 && open == close)
        {
            while(++y <= i)
                dest[y] = str[y];
            dest[y] = 0;
            translate_string(str, i+1);
            return i+1;
        }
    }
    return _NOT_COMPLET_;
}

int message_nature(char *str, char *dest)
{
    if (identify_message(str, dest) < 0)
        return _NOT_COMPLET_;
    if(dest[0] == 's')
        return _SEND_REQUEST_;
    if(dest[0] == 'i')
        return _INFO_REQUEST_;
    return _ERR_;
}


int extract_info(char *str, char *dest)
{
    int open = 0;
    int i=-1, y=-1;

    while (str[++i] != 0)
    {
        if (str[i] == '{')
            open ++;
        if (open == 2)
        {
            while(str[i] != '}')
                dest[++y] = str[++i];
            dest[y] = 0;
            return _SUCCESS_;
        }
    }
    return _NOT_COMPLET_;
}


int extract_double_info(char *str, char *info)
{
    int i=-1, y=-1, z=-1;

    while(str[++i] != 0)
    {
        if (str[i] == ',')
        {
            while (++y < i)
                info[y] = str[y];
            while (str[++i] != 0)
                str[++z] = str[i];
            str[z+1] = 0;
            return _SUCCESS_;
        }
    }
    return _NOT_COMPLET_;
}

int identify_info_connection(char *str, multi_t **net, net_ipc_t *ipc, int index, int size_str)
{
    char information[BUFFER];
    char buf[10];
    int  i = -1, p=0, n, y, e;
    char *tmp;

    char extract_file[BUFFER];
    char file[20];
    int fd = -1;

    if ((*net) == NULL)
        return _NOT_INIT_;

    if (extract_info (str, information) == _NOT_COMPLET_)
        return _NOT_COMPLET_;

    switch (str[2])
    {
        case 'n':
            if ((*net)->connexion[index].name[0] == 0)
            {
                strcpy((*net)->connexion[index].name, information);
                send_name((*net), index);
                ft_itoa((*net)->connexion[index].id, buf, 10);
                strcpy(information, "i{c{");
                strcat(information, (*net)->connexion[index].name);
                strcat(information, ",");
                strcat(information, buf);
                strcat(information, "}}");
                send_ipc(ipc, information);
            }
            break;
        case 'p': 
            if ((*net)->connexion[index].port < 0)
            {
                (*net)->connexion[index].port = atoi(information);
                send_port((*net), index);
            }
            break;
        case 'l': 
            while (++i < 3)
            {
                if (i != index && (*net)->connexion[i].sockfd > 0)
                {
                    ft_itoa((*net)->connexion[i].clt_addr.sin_addr.s_addr, buf, 10);
                    bzero(information, sizeof(char)*BUFFER);
                    strcpy(information, "i{s{");
                    strcat(information, buf);
                    strcat(information, ",\0");
                    ft_itoa((*net)->connexion[i].port, buf, 10);
                    strcat(information, buf);
                    strcat(information, "}}");
                    send_message((*net), information, index);
                }
            }
            break;
        case 's':
            extract_double_info(information, buf);
            printf("[%sc%s] %snew serveur at %s%s:%s%s\n", GREEN, DEFAULT_COLOR, CYAN, YELLOW, buf, information, DEFAULT_COLOR);
            connect_to_other_srv(&(*net), buf, information);
            break;
        case 'd': 
            ft_itoa((*net)->connexion[index].id, buf, 10);
            strcpy(information, "i{d{");
            strcat(information, buf);
            strcat(information, "}}");
            send_ipc(ipc, information);
            shutdown((*net)->connexion[index].sockfd, SHUT_RDWR);
            del_connection(&(*net), index);
            break;
        case 'g':
            strcpy(buf, information);
            strcpy(information, "i{g{");
            strcat(information, buf);
            strcat(information, "}}");
            send_ipc(ipc, information);
            break;
        case 'i': 
            y = atoi(information);
            if (y < 0)
            {
                e = (*net)->my_id;
                for (i=0; i < 3; i++)
                    if ((*net)->connexion[i].id > e)
                        e = (*net)->connexion[i].id;
                e++;
                ft_itoa(e, buf, 10);
                strcpy(information, "i{a{");
                strcat(information, buf);
                strcat(information, "}}");
                send_message((*net), information, index);
                y = e;
            }
            if ((*net)->connexion[index].id < 0)
            {
                (*net)->connexion[index].id = y;
                ft_itoa((*net)->my_id, buf, 10);
                strcpy(information, "i{i{");
                strcat(information, buf);
                strcat(information, "}}");
                send_message((*net), information, index);
            }
            break;
        case 'a':
            y = atoi(information);
            (*net)->my_id = y;
            ft_itoa((*net)->my_id, buf, 10);
            strcpy(information, "i{a{");
            strcat(information, buf);
            strcat(information, "}}");
            send_ipc(ipc, information);
            break;
    }
    return _SUCCESS_;
}


int identify_info_ipc(char *str, multi_t **net, net_ipc_t *ipc)
{
    char information[BUFFER];
    char buf[10];
    struct dirent *dir = NULL;
    DIR *d;
    int i = 0, e = 1, fd = 0, n, p = 0;
    float s;
    struct stat sb;

    if ((*net) == NULL)
        return _NOT_INIT_;

    if (extract_info (str, information) == _NOT_COMPLET_)
        return _NOT_COMPLET_;

    switch (str[2])
    {
        case 'd':
            if (information[0] == '0')
                (*net)->isPlaying = 0;
            break;
        case 'g':
            if (information[0] == '0') 
                send_all_message((*net), "i{g{0}}"); 
            break;
    }
    return _SUCCESS_;
}


int send_msg_connection(multi_t *net, char *msg)
{
    int i=-1;

    if (net == NULL)
        return _NOT_INIT_;

    while (++i < 3)
        if (net->connexion[i].sockfd > 0)
            send_message(net, msg, i);
    return _SUCCESS_;
}


























