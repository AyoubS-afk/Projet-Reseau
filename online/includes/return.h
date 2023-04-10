/** ====================================================================
**   Auteur  : GENY                   | Date    : 14/03/2022
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux
**  --------------------------------------------------------------------
**   Nom fichier : return.h     | Version : 1.0
**  --------------------------------------------------------------------
**   Description : declaration des retours des fonctions
** =====================================================================*/

#ifndef __RETURN_H_
#define __RETURN_H_

/*
definition des erreurs
*/
#define _NOT_COMPLET_     -2
#define _CANT_ACCEPT_     -1
#define _SUCCESS_          0
#define _ERR_ARGS_         1
#define _ERR_              2
#define _ERR_INIT_IPC_     3
#define _ERR_ALLOC_        4
#define _NOT_INIT_         5
#define _ERR_SOCKET_       6
#define _ERR_BIND_         7
#define _ERR_SEND_         8
#define _ERR_RECV_         9
#define _ERR_NOT_CONNECT_ 10
#define _INFO_REQUEST_    11
#define _SEND_REQUEST_    12
#define _PROTO_SEND_FILE_ 13

#endif