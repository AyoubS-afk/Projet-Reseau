/** ====================================================================
**   Auteur  : GENY                   | Date    : 14/03/2022
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux
**  --------------------------------------------------------------------
**   Nom fichier : protocoleIPC.h     | Version : 1.0
**  --------------------------------------------------------------------
**   Description : declaration des protocole qui n'on pas besoin d'etre
**                 complete
** =====================================================================*/

#ifndef __PROTOCOLEIPC_H_
#define __PROTOCOLEIPC_H_

/*
definition des protocole strandard de communication
*/
#define _RET_NO_ERR_    "i{r{0}}"
#define _RET_ERR_FILE_  "i{r{1}}"
#define _RET_ERR_SOCK_  "i{r{2}}"
#define _RET_ERR_ALLOC_ "i{r{5}}"

#define _STOP_COMM_TCP_ "i{d{0}}"

#endif