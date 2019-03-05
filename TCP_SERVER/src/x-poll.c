/*****************************************************
@Copyright (c) 2016 WAYOS.Co.,Ltd. All rights reserved
@Auther    : Jeaham Chain
@Department: R&D
@E-mail	   : chenjinhan@wayos.cn
******************************************************/
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include <x-poll.h>
#include <x-http.h>
#include <x-memory.h>
#include <x-debug.h>
#include <x-session.h>

struct pollfd *poll_fds;
_s32 poll_max_size = 0;
_s32 poll_curfds = 0;

_vd poll_add_write(_u32 pid)
{
	poll_fds[pid].events |= POLLOUT;
	return;
}

_vd poll_add_read(_u32 pid)
{
	poll_fds[pid].events |= POLLIN;
	return;
}

_vd poll_del_write(_u32 pid)
{
	poll_fds[pid].events &= ~POLLOUT;
	return;
}

_vd poll_del_read(_u32 pid)
{
	poll_fds[pid].events &= ~POLLIN;
	return;
}

_s32 poll_add(_s32 fd)
{
	_s32 i, ret = -1;

	for(i = 1; i < poll_max_size; i++)
	{
		if(poll_fds[i].fd < 0)
		{
			poll_fds[i].fd = fd;
			poll_fds[i].events = POLLIN;
			ret = i;
			break;
		}
	}

	if(i == poll_curfds)
	{
		poll_curfds++;
	}

	return ret;
}

_vd poll_del(_s32 index)
{
	close(poll_fds[index].fd);
	poll_fds[index].fd = -1;
	poll_fds[index].events = POLLIN;	
}

_s32 poll_setfd_noblocking(_s32 fd)
{
 
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0)|O_NONBLOCK))
        return -1;
    return 0;
}

_s32 poll_events_handler(_s32 index)
{
	_s32 fd = poll_fds[index].fd;

	if(poll_fds[index].revents & POLLERR)
	{
		goto OUT;
	}

	if(poll_fds[index].revents & POLLOUT)
	{
		if(ses_send(fd))
		{
			goto OUT;
		}

		if(ses_check_send_state(fd))
		{
			goto OUT;
		}
	}

	if(poll_fds[index].revents & POLLIN)
	{
		if(ses_recv(fd))
		{
			goto OUT;
		}
	}
	
	return 0;
OUT:
	
	ses_release_session(fd, index);
	return 0;
}

_s32 poll_process_run(_s32 interval)
{

	_s32 i, nready, new_fd, ret = -1;
	socklen_t addr_size;
	struct sockaddr_in peer_addr;

	nready = poll(poll_fds, poll_curfds, interval);

	if(nready < 1)
	{
		goto OUT;
	}

	if(poll_fds[0].revents & POLLIN)
	{
		while(1)
		{
			addr_size = sizeof(struct sockaddr_in);
			new_fd = accept(http_get_svr_fd(), (struct sockaddr*)&peer_addr, &addr_size);

			if(new_fd < 0)
			{
				break;	
			}
			else
			{
				if(ses_create_new_session(new_fd, &peer_addr))
				{
					close(new_fd);
				}
			}
		}

		if(--nready <= 0)
		{
			goto OUT;
		}
	}

	for(i = 1; i < poll_curfds; i++)
	{
		if(poll_fds[i].fd < 0)
		{
			continue;
		}

		if(poll_fds[i].revents &(POLLIN|POLLERR|POLLOUT))
		{
			if(poll_events_handler(i))
			{
				goto ERR;
			}
		}

		if(--nready <= 0)
		{
			break;
		}
	}

OUT:
	ret = 0;
ERR:
	return ret;
};

_s32 poll_init(_s32 poll_max)
{
	_s32 ret = -1, i;

	if(poll_max <= 1)
	{
		goto ERR;
	}

	poll_max_size = poll_max;
	poll_fds = (struct pollfd*)wys_malloc(sizeof(struct pollfd)*poll_max);

	if(!poll_fds)
	{
		perror("malloc");
		goto ERR;
	}

	assert(http_get_svr_fd() != -1);

	poll_curfds++;
	poll_fds[0].fd = http_get_svr_fd();
	poll_fds[0].events = POLLIN;

	for(i = 1; i < poll_max; i++)
	{
		poll_fds[i].fd = -1;
		poll_fds[i].events = POLLIN;
	}

	ret = 0;
ERR:	
	return ret;
}

