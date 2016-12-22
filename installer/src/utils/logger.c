/****************************************************************************
 * Copyright (C) 2015
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common/common.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "logger.h"

#ifdef DEBUG_LOGGER
static int log_socket = -1;
static volatile int log_lock = 0;


void log_init(const char * ipString)
{
	log_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (log_socket < 0)
		return;

	struct sockaddr_in connect_addr;
	memset(&connect_addr, 0, sizeof(connect_addr));
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = 4405;
	inet_aton(ipString, &connect_addr.sin_addr);

	if(connect(log_socket, (struct sockaddr*)&connect_addr, sizeof(connect_addr)) < 0)
	{
	    socketclose(log_socket);
	    log_socket = -1;
	}
}

void log_deinit(void)
{
    if(log_socket >= 0)
    {
        socketclose(log_socket);
        log_socket = -1;
    }
}

void log_print(const char *str)
{
    // socket is always 0 initially as it is in the BSS
    if(log_socket < 0) {
        return;
    }

    while(log_lock)
        usleep(1000);
    log_lock = 1;

    int len = strlen(str);
    int ret;
    while (len > 0) {
        int block = len < 1400 ? len : 1400; // take max 1400 bytes per UDP packet
        ret = send(log_socket, str, block, 0);
        if(ret < 0)
            break;

        len -= ret;
        str += ret;
    }

    log_lock = 0;
}

void log_printf(const char *format, ...)
{
    if(log_socket < 0) {
        return;
    }

	char * tmp = NULL;

	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
        log_print(tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}
#endif
