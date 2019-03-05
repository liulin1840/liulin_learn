/*****************************************************
@Copyright (c) 2016 WAYOS.Co.,Ltd. All rights reserved
@Auther    : Jeaham Chain
@Department: R&D
@E-mail	   : chenjinhan@wayos.cn
******************************************************/
#include <string.h>
#include <assert.h>
#include <x-session.h>
#include <x-memory.h>
#include <x-poll.h>
#include <x-debug.h>
#include <x-lua.h>
#include <time.h>
#include <sys/time.h>

#define SES_FID_HASH_MAX 			0X0000400
#define SES_SID_HASH_MAX 			SES_FID_HASH_MAX
#define SES_FID_HASH_MASK 			(SES_FID_HASH_MAX-1)
#define SES_SID_HASH_MASK 			SES_FID_HASH_MASK
#define SES_FID_HASH_IDX(fid)		(((_u32)(fid)) & SES_FID_HASH_MASK)
#define SES_SID_HASH_IDX(sid)		(((_u32)(sid)) & SES_SID_HASH_MASK)
#define SES_FID_HASH_HEAD(fid) 		&ses_fid_hash[SES_FID_HASH_IDX(fid)]
#define SES_SID_HASH_HEAD(sid) 		&ses_sid_hash[SES_SID_HASH_IDX(sid)]
#define SES_OUT_FID_HASH(session) 	hlist_del_init(&session->fid_to)
#define SES_OUT_SID_HASH(session) 	hlist_del_init(&session->sid_to)
#define SES_ENTER_FID_HASH(session)	hlist_add_head(&session->fid_to, SES_FID_HASH_HEAD(session->fid)) 
#define SES_ENTER_SID_HASH(session)	hlist_add_head(&session->sid_to, SES_SID_HASH_HEAD(session->sid)) 

#define HTTP_EOF	"\r\n\r\n"
#define HTTP_SEOF	"\r\n"

#define X_SERVER "X-server/1.0 by Jeaham Chain"
#define X_MAX_ERR_PAGE_LEN 1024

#define CHAR_QUESTION_MARK	0X3F
#define CHAR_SPACE			0X20
#define CHAR_AND_MARK		0X26

struct hlist_head ses_fid_hash[SES_FID_HASH_MAX];
struct hlist_head ses_sid_hash[SES_SID_HASH_MAX];

x_session_t *ses_find_session_by_fid(_s32 fid)
{
	x_session_t* session;
	struct hlist_node *pos;
	struct hlist_head *head;

	head =  SES_FID_HASH_HEAD(fid);

	hlist_for_each(pos, head)
	{
		session = hlist_entry(pos, x_session_t, fid_to);

		if(session->fid == fid)
		{
			return session;
		}
	}

	return NULL;
}

x_session_t *ses_find_session_by_sid(_s32 sid)
{
	x_session_t* session;
	struct hlist_node *pos;

	hlist_for_each(pos, SES_SID_HASH_HEAD(sid))
	{
		session = hlist_entry(pos, x_session_t, sid_to);

		if(session->sid == sid)
		{
			return session;
		}
	}

	return NULL;
}

_s32 ses_create_new_sid(_vd)
{
	static _s32 sid = 0; 
	return sid++;
}

_vd inline ses_keep_alive(_ul data)
{
	INFO("Keepalive timeout.\n");
}

_s32 ses_create_new_session(_s32 fd, struct sockaddr_in *addr)
{
	_s32 ret = -1;
	x_session_t *session = (x_session_t*)wys_malloc(sizeof(x_session_t));

	if(!session)
	{
		perror("malloc");
		goto ERR;
	}

	session->fid = fd;
	
	memcpy(&session->addr, addr, sizeof(struct sockaddr_in));


	session->state = SESSION_STAT_RECV_INIT;
	session->rcv_len = 0;
	session->timer.data = (_ul)session;
	session->timer.expires = jiffies + 5*HZ;
	session->timer.function = ses_keep_alive;
	session->pid = poll_add(fd);
	session->sid = ses_create_new_sid();
	
	if(session->pid < 1)
	{
		goto ERR;
	}
	
	if(poll_setfd_noblocking(fd))
	{
		goto ERR;
	}

	SES_ENTER_FID_HASH(session);
	SES_ENTER_SID_HASH(session);
	
	ret = 0;
ERR:
	return ret;
}

_s32 ses_release_session(_s32 fid, _u32 index)
{
	x_session_t* session  = ses_find_session_by_fid(fid);

	assert(session != NULL);

	if((session->state & SESSION_STAT_SEND_FINISH) && (session->state & SESSION_STAT_KEEP_ALIVE))
	{
		// TODO:后续开发,现在lua只会返回close
		INFO("add session to keep alive pool.\n");
	}

	SES_OUT_FID_HASH(session);
	SES_OUT_SID_HASH(session);
	wys_free(session->snd_buf);
	wys_free(session->rcv_buf);
	wys_free(session);
	poll_del(index);

	return 0;
}

_vd ses_session_init(_vd)
{
	_s32 i;

	for(i = 0; i < SES_FID_HASH_MAX; i++)
	{
		INIT_HLIST_HEAD(&ses_fid_hash[i]);
	}

	for(i = 0; i < SES_SID_HASH_MAX; i++)
	{
		INIT_HLIST_HEAD(&ses_sid_hash[i]);
	}
}

_s8* ses_check_eof(_s8* buf)
{
	return strstr((const _s8*)buf, HTTP_EOF);
}

_vd ses_send_data(x_session_t *session)
{
	poll_add_write(session->pid);
	return;
}

_s8* ses_get_date_string(_s8* out_buf, time_t* tms)
{
	time_t	now;
	struct tm *p;
	char *wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	char *mday[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

	if(tms)
		p=gmtime(tms);
	else
	{
		time(&now);
		p=gmtime(&now);
	}

	sprintf(out_buf,"%s, %02d %s %d %02d:%02d:%02d GMT",
		wday[p->tm_wday],p->tm_mday,mday[p->tm_mon],
		(1900+p->tm_year),p->tm_hour, p->tm_min, p->tm_sec);

	return out_buf;
}

_vd ses_create_505_page(_s8* err_page, _s8* err_info)
{
	snprintf(err_page, X_MAX_ERR_PAGE_LEN, 
	"<html>"
	"<body bgcolor=\"white\">"
	"<title>500 Internal Server Error</title>"
	"<style>"
	"body"
	"{"
		"nargin:auto;"
		"font-family:Tahoma, Geneva, sans-serif;"
	"}"
	"</style>"
	"<h1 align=\"center\">500 Internal Server Error</h1>"
	"<hr><p align=\"center\">Error Info:%s</p>"
	"</body>"
	"</html>"
	, err_info);

}

_s32 ses_send_error_info(x_session_t* session)
{
	_s32 ret = -1;
	_s32 len = 0;
	_s8 date[128] = {0};
	_s8 err_page[X_MAX_ERR_PAGE_LEN] = {0};
	ses_create_505_page(err_page, session->error_info);
	session->snd_buf = (_s8*)wys_realloc(session->snd_buf, 1024);

	if(!session->snd_buf)
	{
		ERROR("realloc()");
		goto ERR;
	}

	len += sprintf(session->snd_buf + len, "HTTP/1.1 500 ERROR\r\n");
	len += sprintf(session->snd_buf + len, "Server:%s\r\n",X_SERVER);
	len += sprintf(session->snd_buf + len, "Content-type:text/html\r\n");
	len += sprintf(session->snd_buf + len, "Content-Length:%d\r\n",strlen(err_page));	
	len += sprintf(session->snd_buf + len, "Accept-Ranges: bytes\r\n");
	len += sprintf(session->snd_buf + len, "Date: %s\r\n", ses_get_date_string(date, NULL));
	len += sprintf(session->snd_buf + len, "Connection: close\r\n\r\n");
	len += sprintf(session->snd_buf + len, "%s",err_page);

	session->snd_len += len;
	ses_send_data(session);
	ret = 0;
ERR:
	return ret;
}

_s32 ses_send_http_data(x_session_t* session)
{

	if(session->state & SESSION_STAT_LUA_ERROR)
	{
		return ses_send_error_info(session);
	}
	ses_send_data(session);
	return 0;
}

_s32 ses_http_procotol_handler(x_session_t *session)
{
	_s32 ret = -1;

	// TODO:后续开发多线程支持

	if(lua_access(session))
	{
		goto ERR;
	}
	if(ses_send_http_data(session))
	{
		goto ERR;
	}

	ret = 0;
ERR:
	return ret;
}

_s32 ses_check_send_state(_s32 fid)
{
	_s32 ret = -1;
	x_session_t *session = ses_find_session_by_fid(fid);

	if(session->state & SESSION_STAT_SEND_FINISH)
	{
		goto OUT;
	}

	ret = 0;
OUT:
	return ret;	
}

_s32 ses_recv(_s32 fid)
{
	_s32 ret = -1;
	_s32 len;
	x_session_t *session = ses_find_session_by_fid(fid);
	static _s8 rcv_buf[10240] = {0};
	
	if(!session)
	{
		goto ERR;
	}

REDO:
	bzero(rcv_buf, sizeof(rcv_buf));
	
	len = recv(session->fid, rcv_buf, sizeof(rcv_buf), 0);

	if(len <= 0)
	{
		goto ERR;
	}

	if(session->rcv_buflen < session->rcv_len + len)
	{
		session->rcv_buf = (_s8*)wys_realloc(session->rcv_buf, session->rcv_len + len);
		if(!session->rcv_buf)
		{
			perror("realloc()");
			goto ERR;
		}
		
		session->rcv_buflen = session->rcv_len + len;
	}

	memcpy(session->rcv_buf + session->rcv_len , rcv_buf, len);
	session->rcv_len += len;

	// TODO:根据content-length判断数据是否接收完整,这种判断会有点问题
	if(len == sizeof(rcv_buf))
	{
		len = 0;
		goto REDO;
	}

	if(!ses_check_eof(session->rcv_buf))
	{
	
		session->state &= (~SESSION_STAT_RECV_INIT);
		session->state |= SESSION_STAT_RECV_AGAIN;
		goto OUT;
	}

	if(ses_http_procotol_handler(session))
	{
		goto ERR;
	}

OUT:
	ret = 0;
ERR:
	return ret;
}

_s32 ses_send(_s32 fid)
{
	_s32 ret = -1;
	_s32 len;
	x_session_t *session = ses_find_session_by_fid(fid);

	if(session->snd_len == 0)
	{
		poll_del_write(session->pid);
		goto OUT;
	}

	len = send(session->fid, session->snd_buf, session->snd_len, 0);

	if(len <= 0)
	{
		perror("send");
		session->state |= SESSION_STAT_SEND_ERROR;
		goto ERR;
	}
	else
	{
		if(len < session->snd_len)
		{
			memmove(session->snd_buf, session->snd_buf + len, session->snd_len - len);
			session->snd_len -= len;
			session->state |= SESSION_STAT_SEND_AGAIN;
		}
		else
		{
			wys_free(session->snd_buf);
			session->snd_buf = NULL;
			session->snd_len = 0;
			session->state |= SESSION_STAT_SEND_FINISH;
			poll_del_write(session->pid);
		}
	}

OUT:
	ret = 0;
ERR:	
	return ret;
}
