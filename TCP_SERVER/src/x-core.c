/*****************************************************
@Copyright (c) 2016 GUN. All rights reserved
@Auther    : miaomiao
@Department: GUN
@E-mail	   : 545911533@qq.com
******************************************************/
#include <sys/signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <x-types.h>
#include <x-debug.h>
#include <x-http.h>
#include <x-poll.h>
#include <x-memory.h>
#include <x-session.h>

_u8 core_stop = 0;

_vd core_exit(_s32 status)
{
	INFO("exit status:%d\n",status);
	exit(status);
}

static _vd	core_sig_handler(_s32 signal)
{
	INFO("Catch Signal %d\n",signal);
	if(signal == 2)
	{
		wys_show_mem_info();
	}
	return core_exit(0);
}

_vd	core_signals_init(_vd)
{
	struct sigaction sa;

	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);
	
	signal(SIGINT, core_sig_handler);
	signal(SIGKILL, core_sig_handler);
	signal(SIGTERM, core_sig_handler);
	signal(SIGSEGV, core_sig_handler);
	signal(SIGCHLD, core_sig_handler);
	signal(SIGBUS, core_sig_handler);
}

_s32 core_mempool_init(_vd)
{
	wys_mem_init();
	return 0;
}

_s32 core_http_init(_vd)
{
	return	http_server_init(0, 80, 1024);
}

_s32 core_poll_init(_vd)
{
	if(poll_init(1024))
	{
		return -1;
	}

	return 0;
}

_s32 core_session_init(_vd)
{
	ses_session_init();

	return 0;
}

_s32 core_process_run()
{
	while(!core_stop)
	{
		poll_process_run(50);
	}

	return 0;
}

_s32 main(_s32 argc, _s8 **argv)
{

	core_signals_init();
	core_mempool_init();
	core_http_init();
	core_poll_init();
	core_session_init();
	core_process_run();
	return 0;	
}

