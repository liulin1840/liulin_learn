/*****************************************************
@Copyright (c) 2016 GUN. All rights reserved
@Auther    : miaomiao
@Department: GUN
@E-mail	   : 545911533@qq.com
******************************************************/
#ifndef _X_SESSION_H
#define _X_SESSION_H

#include <netinet/in.h>
#include <stdio.h>
#include <x-types.h>
#include <x-list.h>
#include <x-timer.h>

#define SES_PARMS_MAX 128
#define SES_RCV_MAX 20480	

#define SESSION_STAT_RECV_INIT				(1 << 1)
#define SESSION_STAT_RECV_AGAIN				(1 << 2)

#define SESSION_STAT_KEEP_ALIVE				(1 << 8)

#define SESSION_STAT_SEND_ERROR				(1 << 16)
#define SESSION_STAT_SEND_AGAIN				(1 << 17)
#define SESSION_STAT_SEND_FINISH			(1 << 18)
#define SESSION_STAT_LUA_ERROR				(1 << 24)

typedef struct session_st
{
	struct list_head	alive_session_list;
	struct hlist_node	fid_to;
	struct hlist_node	sid_to;


	_u32 state;

	_s32 fid;
	_s32 sid;
	_u32 pid;
	
	_s8 error_info[128];
	
	struct sockaddr_in addr;
	struct timer_list timer;

	_s8	 *rcv_buf;
	_s32 rcv_buflen;
	_s32 rcv_len;
	_s8  *snd_buf;
	_s32 snd_len;

}x_session_t;

_s32 ses_create_new_session(_s32 fd, struct sockaddr_in *addr);
_vd ses_session_init(_vd);
_s32 ses_recv(_s32 fid);
_s32 ses_send(_s32 fid);
_s32 ses_check_send_state(_s32 fid);
_s32 ses_release_session(_s32 fid, _u32 index);

#endif

