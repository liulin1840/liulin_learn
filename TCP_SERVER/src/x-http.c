/*****************************************************
@Copyright (c) 2016 GUN. All rights reserved
@Auther    : miaomiao
@Department: GUN
@E-mail	   : 545911533@qq.com
******************************************************/
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include <x-list.h>
#include <x-http.h>
#include <x-debug.h>
#include <x-poll.h>

#define INVALID_SOCKET 	-1

_s32 http_svr_fd = INVALID_SOCKET;

_s32 http_get_svr_fd(_vd)
{
	return http_svr_fd;
}


_s32 http_server_init(_u32 ip, _u16 port, _u32 listen_max)
{
	_s32 reuse = 1;
	_s32 ret = -1;
	_s32 trycnt = 10;
	struct sockaddr_in svr_addr;

	if((http_svr_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		goto ERR;
	}

	if(poll_setfd_noblocking(http_svr_fd))
	{
		goto ERR;
	}

	bzero(&svr_addr, sizeof(svr_addr));

	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(port);
	svr_addr.sin_addr.s_addr = ip;

	if(setsockopt(http_svr_fd, SOL_SOCKET, SO_REUSEADDR, (_s8*)&reuse, sizeof(reuse)))
	{
		goto ERR;
	}

	if(bind(http_svr_fd, (struct sockaddr*)&svr_addr, sizeof(struct sockaddr)))
	{
		while(trycnt--)
		{
			if(!(bind(http_svr_fd, (struct sockaddr*)&svr_addr, sizeof(struct sockaddr))))
			{
				goto NEXT;
			}
		}
		perror("bind");
		goto ERR;
	}
NEXT:

	if(listen(http_svr_fd, listen_max))
	{
		perror("listen");
		goto ERR;
	}

	INFO("Http server [%d.%d.%d.%d:%d] init OK.Listen max:%d.\n"
	,ip&0XFF000000,ip&0X00FF0000,ip&0X0000FF00,ip&0X000000FF,port,listen_max);
ERR:
	return ret;
}
