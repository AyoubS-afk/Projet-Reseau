/** ====================================================================
**   Auteur  : GENY                   | Date    : 28/02/2022
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux
**  --------------------------------------------------------------------
**   Nom fichier : include.h          | Version : 1.0
**  --------------------------------------------------------------------
**   Description : declaration des fonctions du fichier main.c
** =====================================================================*/

#ifndef __INCLUDE_H_
#define __INCLUDE_H_

#include <stdlib.h>

#define _EXIT_ERR_ 1

char* ft_itoa(int value, char* buffer, int base);
char* reverse(char *buffer, int i, int j);
void swap(char *x, char *y);

#endif