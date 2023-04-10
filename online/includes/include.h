/** ====================================================================
**   Auteur  : GENY                   | Date    : 09/03/2022
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux
**  --------------------------------------------------------------------
**   Nom fichier : include.h          | Version : 1.0
**  --------------------------------------------------------------------
**   Description : header du fichier main.c
** =====================================================================*/

#ifndef __INCLUDE_H_
#define __INCLUDE_H_

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>
#include <arpa/inet.h>

#include "../includes/itoa.h"
#include "../includes/netipc.h"
#include "../includes/multi.h"
#include "../includes/protocoleIPC.h"
#include "../includes/return.h"
#include "../includes/string_net.h"
#include "../includes/color.h"

#define FILE_NAME         "online/name.info"
#define FOLDER_SAVE_MULTI "saveMulti/"
#define BUFFER            1024

#endif