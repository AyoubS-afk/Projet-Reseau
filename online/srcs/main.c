#include "../includes/include.h"

int main(int argc, char **argv, char **env)
{
	net_ipc_t         *ipc = NULL; 
	multi_t           *net = NULL;
	int                i, n;
	int                activity;
	int                new_socket;
	struct sockaddr_in new_addr;
	char               message[BUFFER];
	char               buffer[BUFFER];
	int                addrlen = sizeof(struct sockaddr_in);

	bzero(message, sizeof(char)*BUFFER);
	bzero(buffer, sizeof(char)*BUFFER);
	if (argc < 2 || argc > 4 || argc == 3)
	{
		printf("%sERROR args : nombre attendu 2 ou 4 -> recu %d%s\n", RED, argc, DEFAULT_COLOR);
		return _ERR_ARGS_;
	}

	ipc = init_ipc(atoi(argv[1])); 
	if (ipc == (net_ipc_t*)_ERR_ALLOC_  ||
		ipc == (net_ipc_t*)_ERR_SOCKET_ ||
		ipc == (net_ipc_t*)_ERR_BIND_     ) 
	{
		printf("%sERROR init socket ipc : %d%s\n", RED, ipc, DEFAULT_COLOR);
		perror("FAIL");
		return _ERR_INIT_IPC_;
	}
	i = send_ipc(ipc, "i{p{0}}");
	if (i != _SUCCESS_)
	{
		del_ipc(ipc, _RET_ERR_SOCK_);
		printf("%sERROR send_ipc a l'initialisation : %d%s\n", RED, i, DEFAULT_COLOR);
		perror("FAIL");
		return _ERR_;
	}

	net = init_srv();
	if (net == (multi_t*)_ERR_ALLOC_) 
	{
		del_ipc(ipc, _RET_ERR_ALLOC_); 
		printf("%sERROR initialisation structure multi_t : %d%s\n", RED, net, DEFAULT_COLOR); // affichage d'erreur
		perror("FAIL");
		return _ERR_;
	}

	i = open(FILE_NAME, O_RDONLY);
	if (i < 0)
	{
		del_ipc(ipc, _RET_ERR_FILE_);
		del_net(&net);
		printf("%sERROR recuperation du nom dans le jeu%s\n", RED, DEFAULT_COLOR);
		perror("FAIL");
		return _ERR_;
	}
	read(i, message, sizeof(char)*29);
	close(i);
	set_name(&net, message); 

	if (argc == 4) 
	{
		i = connect_to_server(&net, argv[2], atoi(argv[3])); 
		if (i == _NOT_INIT_   ||
			i == _ERR_SOCKET_) 
		{
			del_ipc(ipc, _RET_ERR_SOCK_);
			del_net(&net);
			printf("%sERROR connection au serveur : %d%s\n", RED, i, DEFAULT_COLOR); 
			perror("FAIL");
			return _ERR_;
		}
		send_id(net, 0); 
		send_name(net, 0); 
	}
	else
	{
		net->my_id = 1;
	}
	i = start_server(&net);
	if (i == _ERR_SOCKET_)
	{
		del_ipc(ipc, _RET_ERR_SOCK_);
		del_net(&net);
		printf("%sERROR demarage du serveur : %d%s\n", RED, i, DEFAULT_COLOR);
		perror("FAIL");
		return _ERR_;
	}
	printf("[%sc%s] %souverture du serveur sur le port %s%d%s\n", GREEN, DEFAULT_COLOR, CYAN, YELLOW, ntohs(net->srv.srv_addr.sin_port), DEFAULT_COLOR);
	bzero(message, sizeof(char)*BUFFER);
	send_ipc_port(ipc, ntohs(net->srv.srv_addr.sin_port));
	if (argc == 4)
	{
		send_port(net, 0); 
		send_message(net, "i{l{0}}", 0);
	}

	while (net->isPlaying)
	{
		init_select_multi(&net, ipc);

		activity = select(net->max_sd+1, &(net->readfds), NULL, NULL, NULL);
        printf("[%sc%s] %shey il y a du mouvement sur le reseau%s\n", GREEN, DEFAULT_COLOR, CYAN, DEFAULT_COLOR);
        bzero(buffer, sizeof(char)*BUFFER);
        if ((activity < 0) && (errno != EINTR))
        {
            return _ERR_;
        }
        if (activity > 0)
        {
        	if (FD_ISSET(net->srv.sock_srv, &(net->readfds)))
            {
                if ((new_socket = accept(net->srv.sock_srv,
                                         (struct sockaddr *)&new_addr,
                                         (socklen_t*)&addrlen)) < 0)
                {
                    return _ERR_;
                }
                i = add_new_connection(&net, new_socket, new_addr);
                if (i >= 0)
                	printf("[%sc%s] %sconnection d'un utilisateur : %s%s:%d%s\n", GREEN, DEFAULT_COLOR, CYAN, YELLOW, inet_ntoa(new_addr.sin_addr), ntohs(new_addr.sin_port), DEFAULT_COLOR);
            }
            if (FD_ISSET(ipc->sock_ipc, &(net->readfds)))
            {
            	if (recv_ipc(ipc, buffer) == _SUCCESS_)
            	{
            		clear_string(buffer);
            		printf("[%sc%s] %spython say : %s%s%s\n", GREEN, DEFAULT_COLOR, CYAN, YELLOW, buffer, DEFAULT_COLOR);
            		i = message_nature(buffer, message);
            		if (i == _SEND_REQUEST_)
            			send_msg_connection(net, message);
            		if (i == _INFO_REQUEST_)
            			identify_info_ipc(message, &net, ipc);
            	}
            }
            i = -1;
            while (++i < 3)
            {
            	if (net->connexion[i].sockfd > 0)
            	{
            		bzero(buffer, sizeof(char)*BUFFER);
            		n = recv(net->connexion[i].sockfd, buffer, sizeof(char)*1024, MSG_DONTWAIT);
            		if (n > 0)
            		{
            			clear_string(buffer);
            			strcat(net->connexion[i].frame, buffer);
            			printf("[%sc%s] %s%s(%s:%d) %ssay : %s%s%s\n", GREEN, DEFAULT_COLOR, YELLOW, net->connexion[i].name, inet_ntoa(net->connexion[i].clt_addr.sin_addr), ntohs(net->connexion[i].clt_addr.sin_port), CYAN, YELLOW, buffer, DEFAULT_COLOR);
            			while ( (n = message_nature(net->connexion[i].frame, message)) != _NOT_COMPLET_ )
            			{
            				printf("[%sc%s] %sinterpretation du message : %s%s%s\n", GREEN, DEFAULT_COLOR, CYAN, YELLOW, message, DEFAULT_COLOR);
	            			if (n == _SEND_REQUEST_) 
		            			send_ipc(ipc, message);
		            		if (n == _INFO_REQUEST_)
		            		{
		            			if (identify_info_connection(message, &net, ipc, i, n) == _PROTO_SEND_FILE_)
			            		{
			            			send_ipc(ipc, "i{g{0}}");
			            		}
		            		}
		            	}
            		}
            		if (n == 0)
            		{
            			printf("[%sc%s] %sdeconnection de %s%s(%s:%d)%s\n", GREEN, DEFAULT_COLOR, CYAN, YELLOW, net->connexion[i].name, inet_ntoa(net->connexion[i].clt_addr.sin_addr), ntohs(net->connexion[i].clt_addr.sin_port), DEFAULT_COLOR);
            			bzero(message, sizeof(char)*BUFFER);
            			ft_itoa(net->connexion[i].sockfd, message, 10);
            			strcpy(buffer, "i{d{");
            			strcat(buffer, message);
            			strcat(buffer, "}}");
            			send_ipc(ipc, buffer);
            			del_connection(&net, i);
            		}
            	}
            }
        }
	}
	del_ipc(ipc, _RET_NO_ERR_);
	del_net(&net);
	return _SUCCESS_;
}