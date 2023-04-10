/** ====================================================================
**   Auteur  : GENY                   | Date    : 09/03/2022
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux
**  --------------------------------------------------------------------
**   Nom fichier : main.c             | Version : 1.0
**  --------------------------------------------------------------------
**   Description : permet la communication entre deux processuce via
**                 le reseau localhost, et communique en "pear-to-pear"
**                 dans le cadre de mettre un jeu en multijoueur
** =====================================================================*/

#include "../includes/include.h"

int main(int argc, char **argv, char **env)
{
	net_ipc_t         *ipc = NULL; // structure de communication avec le python
	multi_t           *net = NULL; // structure de communication en reseau
	int                i, n;
	int                activity;
	int                new_socket;
	struct sockaddr_in new_addr;
	char               message[BUFFER];
	char               buffer[BUFFER];
	int                addrlen = sizeof(struct sockaddr_in);

	bzero(message, sizeof(char)*BUFFER);
	bzero(buffer, sizeof(char)*BUFFER);
	/*
	*** si 2 argument -> serveur : ./online <port ipc>
	*** si 4 argument -> cleint  : ./online <port ipc> <ip addr srv> <port srv>
	*/
	if (argc < 2 || argc > 4 || argc == 3) // si pas le bon nombre d'argument
	{
		printf("%sERROR args : nombre attendu 2 ou 4 -> recu %d%s\n", RED, argc, DEFAULT_COLOR);
		return _ERR_ARGS_;
	}

	ipc = init_ipc(atoi(argv[1])); // initialisation de la structure net_ipc_t
	if (ipc == (net_ipc_t*)_ERR_ALLOC_  ||
		ipc == (net_ipc_t*)_ERR_SOCKET_ ||
		ipc == (net_ipc_t*)_ERR_BIND_     ) // erreur possible de la fonction init_ipc
	{
		printf("%sERROR init socket ipc : %d%s\n", RED, ipc, DEFAULT_COLOR);
		perror("FAIL");
		return _ERR_INIT_IPC_;
	}
	/*
	*** initialisation de la communication inter-processuce
	*/
	i = send_ipc(ipc, "i{p{0}}");
	if (i != _SUCCESS_)
	{
		del_ipc(ipc, _RET_ERR_SOCK_);
		printf("%sERROR send_ipc a l'initialisation : %d%s\n", RED, i, DEFAULT_COLOR);
		perror("FAIL");
		return _ERR_;
	}

	/*
	*** initialisation de la structure du serveur ou client
	*/
	net = init_srv();
	if (net == (multi_t*)_ERR_ALLOC_) // si la structure a une erreur d'allocation memoire
	{
		del_ipc(ipc, _RET_ERR_ALLOC_); // erret de communication avec python
		printf("%sERROR initialisation structure multi_t : %d%s\n", RED, net, DEFAULT_COLOR); // affichage d'erreur
		perror("FAIL");
		return _ERR_;
	}

	// recuperation du nom dans le jeu stocker dans un fichier
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
	set_name(&net, message); // ecriture du nom du joueur dans la structure

	if (argc == 4) // si nous devons nous connecter a un serveur
	{
		/* Tentative de connexion au serveur */
		i = connect_to_server(&net, argv[2], atoi(argv[3])); // tentative de connection au serveur
		if (i == _NOT_INIT_   ||
			i == _ERR_SOCKET_) // si la connexion a echoue
		{
			del_ipc(ipc, _RET_ERR_SOCK_); // erret de communication avec python
			del_net(&net);
			printf("%sERROR connection au serveur : %d%s\n", RED, i, DEFAULT_COLOR); // affichage d'erreur
			perror("FAIL");
			return _ERR_;
		}
		send_id(net, 0); // demende d'attribution d'un id
		send_name(net, 0); // envoie du nom au serveur
	}
	else
	{
		net->my_id = 1;
	}
	/* si nous avons un client il va ouvrir egalement un serveur pour */
	/* ne pas devoir passe par le serveur pour communiquer a plusieur */
	/* de cette maniere, les programmes entre eux ne pourons savoir   */
	/* qui est le premier serveur et donc avoir un "pear-to-pear"     */
	i = start_server(&net); // demarage du serveur
	if (i == _ERR_SOCKET_) // si le serveur n'a pue etre demare
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
	if (argc == 4) // si a l'origine c'est un client
	{
		send_port(net, 0); // on donne le port du nouveaux serveur au serveur
		send_message(net, "i{l{0}}", 0);
	}

	/*
	*** Debut de la communication multi-joueur
	*/
	while (net->isPlaying)
	{
		/* tant que la partie est en cours il faut preparer le select sur toute */
		/* les connexion etablies puis a la reception d'un message connetre son */
		/* point d'origine pour l'interpreter                                   */
		init_select_multi(&net, ipc); // premare pour la commande select

		activity = select(net->max_sd+1, &(net->readfds), NULL, NULL, NULL); // si il y a un mouvement sur
		                                                                     // une socket
        printf("[%sc%s] %shey il y a du mouvement sur le reseau%s\n", GREEN, DEFAULT_COLOR, CYAN, DEFAULT_COLOR);
        bzero(buffer, sizeof(char)*BUFFER);
        if ((activity < 0) && (errno != EINTR))
        {
            /* quitter le programme */
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
                    /* quitter le programme */
                    return _ERR_;
                }
                /* Ajout d'une connexion */
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
            		/* interprete message recu de python */
            		i = message_nature(buffer, message);
            		if (i == _SEND_REQUEST_) /* faire suivre le message du python */
            			send_msg_connection(net, message);
            		if (i == _INFO_REQUEST_) /* interpreter le message du python */
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
            			/* interpreter message recu d'un connexion */
            			while ( (n = message_nature(net->connexion[i].frame, message)) != _NOT_COMPLET_ )
            			{
            				printf("[%sc%s] %sinterpretation du message : %s%s%s\n", GREEN, DEFAULT_COLOR, CYAN, YELLOW, message, DEFAULT_COLOR);
	            			if (n == _SEND_REQUEST_) /* faire suivre le message */
		            			send_ipc(ipc, message);
		            		if (n == _INFO_REQUEST_) /* interpreter le message */
		            		{
		            			// filesSize
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
            			/* deconnexion d'un utilisateur */
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

	/*
	*** Arret du processuce
	*/
	del_ipc(ipc, _RET_NO_ERR_); // suppresion de la structure net_ipc_t
	del_net(&net);
	return _SUCCESS_;
}