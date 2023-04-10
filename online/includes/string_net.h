

#ifndef __STRING_NET_H_
#define __STRING_NET_H_

#include <strings.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>

#include "../includes/return.h"
#include "../includes/multi.h"
#include "../includes/netipc.h"
#include "../includes/itoa.h"
#include "../includes/color.h"

#define BUFFER 1024
#define FOLDER_SAVE_MULTI "saveMulti/"

int clear_string(char *str);
int translate_string(char *str, int index);
int identify_message(char *str, char *dest);
int message_nature(char *str, char *dest);
int identify_info_connection(char *str, multi_t **net, net_ipc_t *ipc, int index, int size_str);
int identify_info_ipc(char *str, multi_t **net, net_ipc_t *ipc);
int extract_double_info(char *str, char *info);
int send_msg_connection(multi_t *net, char *msg);

#endif