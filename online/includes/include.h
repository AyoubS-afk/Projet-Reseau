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