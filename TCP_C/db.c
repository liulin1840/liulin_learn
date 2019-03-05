/*****************************************************
@Copyright (c) 2015-2016 GUN. All rights reserved
@Auther    : miaomiao
@Department: GUN
@E-mail	   : 545911533@qq.com
******************************************************/
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>

#include "list.h"
#include "db.h"
#include "debug.h"
#include "mem.h"
#include "types.h"


#define DB_BUFLEN 		10240
#define FREE_KVS_0_KEY	0x00000001
#define DB_HASH_MASK	0X00FF
#define DB_HASH_MAX		(DB_HASH_MASK+1)
#define DB_HASH_IDX(fd) ((_u32)(fd)&DB_HASH_MASK)
#define DB_CONN_KEEP_ALIVE	600000 

#define DB_POOL_RD_LOCK	pthread_rwlock_rdlock(&_db_pool_conn_lock)
#define DB_POOL_WR_LOCK	pthread_rwlock_wrlock(&_db_pool_conn_lock)
#define DB_POOL_UNLOCK 	pthread_rwlock_unlock(&_db_pool_conn_lock)
#define DB_FREE_RD_LOCK	pthread_rwlock_rdlock(&_db_free_conn_lock)
#define DB_FREE_WR_LOCK	pthread_rwlock_wrlock(&_db_free_conn_lock)
#define DB_FREE_UNLOCK 	pthread_rwlock_unlock(&_db_free_conn_lock)

typedef struct
{
	struct list_head	link_to_free;
	struct hlist_node	link_to_pool;
	_ul32	keepalive; 	//老化时间戳
	_s32	fd;			//文件描述符
	_u32	stat;		//连接状态
}dbconn_t;

_u32 _dbaddr = 0;
_u16 _dbport = 0;
_ul32 _start_second = 0;
_s32 _db_pool_conn_count = 0;
_s32 _db_free_conn_count = 0;

pthread_rwlock_t _db_pool_conn_lock;
pthread_rwlock_t _db_free_conn_lock;

struct list_head 	_db_free_conn_list;
struct hlist_head 	_db_pool_conn_hash[DB_HASH_MAX];


enum
{
	DB_NONE = 0,
	DB_SET,
	DB_GET,
	DB_SCAN,
	DB_DEL,
	DB_EXISTS,
	DB_MULTI_GET,
	DB_MULTI_SET,
	DB_MULTI_DEL,
	DB_FLUSHDB,
	DB_DBSIZE,
	DB_CLR_BINLOG,
	DB_COMPACT
};

_s8* _dbstr[] = 
{
	[DB_NONE] 		= "none",
	[DB_SET] 		= "set",
	[DB_GET] 		= "get",
	[DB_SCAN] 		= "scan",
	[DB_DEL] 		= "del",
	[DB_EXISTS] 	= "exists",
	[DB_MULTI_GET]	= "multi_get",
	[DB_MULTI_SET] 	= "multi_set",
	[DB_MULTI_DEL] 	= "multi_del",
	[DB_FLUSHDB]	= "flushdb",
	[DB_DBSIZE]		= "dbsize",	
	[DB_CLR_BINLOG]	= "clear_binlog",
	[DB_COMPACT]	= "compact" 

};

_s8* _dbret_str[] = 
{
	[DB_NORET]			= "unknown err",		
	[DB_ERR]			= "database err",				
	[DB_OUT_MEM]		= "no enough mem",			
	[DB_INVALID_PARMS]	= "invalid parms",	
	[DB_CONNECT_ERR]	= "connect database err",		
	[DB_NOT_FOUND]		= "key not found",		
	[DB_SET_OK]			= "set success",			
	[DB_GET_OK]			= "get success",			
	[DB_DEL_OK]			= "del success",			
	[DB_SCAN_OK]		= "scan success",			
	[DB_NO_EXIST]		= "key donnot exists",		
	[DB_EXIST]			= "key exists",			
	[DB_MULTI_GET_OK]	= "multi get success",	
	[DB_MULTI_SET_OK]	= "multi set success",	
	[DB_MULTI_DEL_OK]	= "multi del success",		
	[DB_FLUSHDB_OK]		= "flushdb success",
	[DB_DBSIZE_OK]		= "dbsize success",
	[DB_RNO_MAX]		= "invalid errno"
};


_ul32 _dbjiffies(void)
{
	_ul32 tt;
	_ul32 cc_s;
	static _ul32 jiffies_second = 0;
	struct timeval tv;
	gettimeofday (&tv, NULL);	

	cc_s = (tv.tv_sec - _start_second);

	if(cc_s - jiffies_second > 2)
	{
		jiffies_second+=2;
		_start_second = tv.tv_sec - jiffies_second;
		
	}else
	{
		jiffies_second = cc_s;	
	}

	
	if(tv.tv_usec )
		tt = (tv.tv_usec / 1000) + (jiffies_second)*1000;
	else
		tt = (jiffies_second)*1000;
	
	return tt;	
}


void _dbenter_pool_conn_hash(dbconn_t* conn)
{
	DB_POOL_WR_LOCK;
	struct hlist_head* head = &_db_pool_conn_hash[DB_HASH_IDX(conn->fd)];
	hlist_add_head(&conn->link_to_pool, head);
	_db_pool_conn_count++;
	DB_POOL_UNLOCK;
}

void _dbpush_free_conn_list(dbconn_t* conn)
{
	INIT_LIST_HEAD(&conn->link_to_free);
	DB_FREE_WR_LOCK;
	list_add_tail(&conn->link_to_free, &_db_free_conn_list);
	_db_free_conn_count++;
	DB_FREE_UNLOCK;
}

void _dbout_pool_conn_hash(dbconn_t* conn)
{
	DB_POOL_WR_LOCK;
	hlist_del_init(&conn->link_to_pool);
	_db_pool_conn_count--;
	DB_POOL_UNLOCK;
}

void _db_conn_destory(dbconn_t* conn)
{
	_dbout_pool_conn_hash(conn);
	shutdown(conn->fd,SHUT_RDWR);
	close(conn->fd);
	wys_free(conn);
}

dbconn_t* _db_new_conn(void)
{
	dbconn_t* conn = NULL;
	struct sockaddr_in	addr;
	_s32				fd;
	_s32				rc = 1;
	struct timeval timeout = {1,0};

	if(!(conn = (dbconn_t*)wys_malloc(sizeof(dbconn_t))))
	{
		ERROR("malloc");
		goto ERR;
	}

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		ERROR("socket()");
		goto ERR;
	}

	fcntl(fd, F_SETFD, FD_CLOEXEC);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&rc, sizeof(rc));	
   	setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
   	setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
	
	memset((char *) &addr, 0x0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = _dbport;
	addr.sin_addr.s_addr = _dbaddr;

	rc = connect(fd, (struct sockaddr *)&addr, sizeof(addr));

	if(rc < 0)
	{
		ERROR("connect()");
		goto ERR;
	}

	conn->fd = fd;
	conn->keepalive = _dbjiffies() + DB_CONN_KEEP_ALIVE;
	_dbenter_pool_conn_hash(conn);

	return conn;
ERR:
	wys_free(conn);
	conn = NULL;
	return conn;	
}

dbconn_t* _db_get_conn(void)
{
	dbconn_t* conn = NULL;
	
	DB_FREE_WR_LOCK;
	assert(_db_free_conn_count >= 0 );
	if(_db_free_conn_count == 0)
	{
		DB_FREE_UNLOCK;
		return _db_new_conn();
	}
	else
	{
		assert(!list_empty(&_db_free_conn_list));

		conn = list_entry(_db_free_conn_list.next, dbconn_t, link_to_free);
		list_del_init(&conn->link_to_free);			
		_db_free_conn_count--;

		DB_FREE_UNLOCK;
		return conn;
	}
}

_s32 _db_conn_free(dbconn_t *conn)
{
	_dbpush_free_conn_list(conn);
	return 0;
}

_s32 _dbatoi(_s8* buf)
{
	if(buf == NULL || buf[0] < '0' || buf[0] > '9')
		return -1;
	else
		return atoi((_s8*)buf);
}

//只非负整数有效
_s32 _dbnumlen(_s32 num)
{
	_s32 num_len = 0;
	_s32 tmp = num;

	if(num == 0)
	{
		return 1;
	}

	assert(num > 0);

	while(tmp)
	{
		num_len++;
		tmp /= 10;
	}

	return num_len;
}

_u32 _dbcountkv(_s8* buf, _s32 buf_len)
{
	_s8* pk = NULL;
	_s8* pv = NULL;
	_u32 cnt = 0;

	pk = buf;
	
	while(pk - buf < buf_len - 1)
	{
		pv = (pk + _dbnumlen(_dbatoi(pk)) + _dbatoi(pk));
		pv += 2; 
		pk = (pv + _dbnumlen(_dbatoi(pv)) + _dbatoi(pv));
		pk += 2;
		cnt++;
	}
	
	return cnt;

}

_u16 _dbsplitkv(DB* db, _s8* buf, _s32 buf_len, _u16 init_state)
{
	_s32 idx = 0;
	_u16 ret = init_state;
	_s8* pk = buf;
	_s8* pv = NULL;
	_u32 kvn = _dbcountkv(buf,buf_len);

	db->kvs = wys_malloc(sizeof(db_kv)*kvn);
	if(!(db->kvs))
	{
		if(!kvn)
		{
			goto OUT;
		}
		
		ret = 	DB_OUT_MEM;
		goto OUT;
	}
	
	while(pk - buf < buf_len - 1)
	{
		db->kvs[idx].key_len = _dbatoi(pk);
		db->kvs[idx].key = (pk + _dbnumlen(db->kvs[idx].key_len) + 1);
		db->kvs[idx].key[db->kvs[idx].key_len] = '\0';

		pv = (db->kvs[idx].key + db->kvs[idx].key_len + 1);
		db->kvs[idx].val_len = _dbatoi(pv);
		db->kvs[idx].val = (pv + _dbnumlen(db->kvs[idx].val_len) + 1);
		db->kvs[idx].val[db->kvs[idx].val_len] = '\0';

		pk = (db->kvs[idx].val + db->kvs[idx].val_len + 1);
		
		idx++;
	}
	db->kvn = idx;
OUT:
	return ret;	
}

_s32 _dbconnect(void)
{
	struct sockaddr_in	addr;
	_s32				fd;
	_s32				rc = 1;
	struct timeval timeout = {1,0};

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		ERROR("socket()");
		goto ERR;
	}

	fcntl(fd, F_SETFD, FD_CLOEXEC);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&rc, sizeof(rc));	
   	setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
   	setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
	
	memset((char *) &addr, 0x0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = _dbport;
	addr.sin_addr.s_addr = _dbaddr;

	rc = connect(fd, (struct sockaddr *)&addr, sizeof(addr));

	if(rc < 0)
	{
		ERROR("connect()");
		goto ERR;
	}

	return fd;
ERR:
	return -1;
}

_s32 _dbsend(dbconn_t *conn, _s8* data, _u32 datalen)
{	
	_s32 ret = -1;
	_s32 s_len = 0;
	_s32 len = 0;
	while(s_len != datalen)
	{
		len = send(conn->fd, data + s_len, datalen - s_len, 0);
		if(len <= 0)
		{
			goto ERR;
		}

		s_len += len;
	}

	ret = 0;
ERR:
	return ret;
}

_s32 _dbcheck(_s8* buf, _s32 buflen)
{

	// TODO:这里需要后面优化,暂时能用
	if(buf[buflen-2] == '\n' && buf[buflen-1] == '\n')
		return -1;
	else
		return 0;
}

_s32 _dbrecv(dbconn_t *conn, _s8** buf_ptr, _u32* buflen)
{
	_s32 r_len = 0; 
	_s32 r_buflen = 0;
	_s32 rcv_len = 0;
	_s32 ret = -1;
	_s8* buf =  NULL;

	buf = (_s8*)wys_malloc(DB_BUFLEN);

	if(!buf)
		goto MERR;

	r_buflen = DB_BUFLEN;
	r_len = recv(conn->fd,buf,DB_BUFLEN,0);

	if(r_len <= 0)
	{
		ERROR("recv()");
		goto RERR;
	}

	while(r_len > 0)
	{
		rcv_len += r_len;

		if(_dbcheck(buf,rcv_len))
		{
			break;
		}

		if(rcv_len == r_buflen)
		{
			buf = wys_realloc(buf, r_buflen + DB_BUFLEN);

			if(!buf)
			{
				ERROR("malloc()");
				goto MERR;
			}
				
			r_buflen += DB_BUFLEN;
			r_len = recv(conn->fd, buf + rcv_len, DB_BUFLEN, 0);
		}
		else
		{
			r_len = recv(conn->fd, buf + rcv_len, DB_BUFLEN - r_len, 0);
		}
		
		if(r_len <= 0)
		{
			ERROR("recv()");
			goto RERR;
		}
		
	}
	
	*buflen = rcv_len;
	*buf_ptr = buf;
	ret = 0;
MERR:
	return ret;
RERR:
	//_db_conn_destory(conn);
	return ret;
}

void _dbphrase(_s8* buf, _u32 rcv_len, DB* db, _u16 cmd)
{
	_s32 len = 0;
	_s8* p = (_s8*)buf;
	_s8* val = NULL;
	_s8 msg[128] = {0};
	_u32 idx = 0;
	_u8 flag = 0;

	//解析返回状态
	len = _dbatoi(p);

	if(len <= 0)
	{
		db->rno = DB_ERR;
		goto OUT;
	}
		
	p = memchr(buf,'\n',rcv_len);
	p++;

	memcpy(msg, p, (len >= sizeof(msg) ? sizeof(msg) - 1 : len));

	switch(len)
	{
		case 2:
			if(!(strncmp(msg,"ok",2) == 0))
			{
				db->rno = DB_NORET;
				flag = 1;
			}
			break;
		case 9:
			if(strncmp(msg,"not_found",9) == 0)
			{
				db->rno = DB_NOT_FOUND;
				flag = 1;
			}
			else
			{
				db->rno = DB_NORET;
				flag = 1;
			}
			break;
		default:
			db->rno = DB_NORET;			
	}

	if(flag)
	{
		switch(cmd)
		{
			//get的时候只返回value数据，要将事先预存的key释放掉
			case DB_GET:
			{
				wys_free(db->kvs[0].key);
				db->src.sts &= (~FREE_KVS_0_KEY);
				break;
			}
		}
		goto OUT;
	}

	p += len + 1;

	//解析返回数据
	switch(cmd)
	{
		case DB_GET:
		{
			db->rno = DB_GET_OK;
			len = _dbatoi(p);
			p = memchr(p,'\n',rcv_len - (p - buf));
			val = ++p;
			*(p+len) = '\0';
			
			db->kvs[idx].val = val;
			db->kvs[idx].val_len = len;
			db->kvn++;
			break;
		}
		case DB_MULTI_GET:
		{
			db->rno = _dbsplitkv(db, p, rcv_len - (p - buf), DB_MULTI_GET_OK);
			break;
		}
		case DB_DEL:
		{
			db->rno = DB_DEL_OK;
			break;
		}
		case DB_SET:
		{
			db->rno = DB_SET_OK;
			len = _dbatoi(p);
			p = memchr(p,'\n',rcv_len - (p-buf));
			p++;

			if(len != 1)
			{
				db->rno = DB_ERR;
			}			
			break;
		}
		case DB_MULTI_SET:
		{
			db->rno = DB_MULTI_SET_OK;
			p = memchr(p,'\n',rcv_len - (p-buf));
			p++;

			if(db->kvn != _dbatoi(p))
			{
				db->rno = DB_ERR;
				db->kvn = 0;
				goto OUT;
			}

			db->kvn = 0;
			break;
		}
		case DB_MULTI_DEL:
		{
			db->rno = DB_MULTI_DEL_OK;
			p = memchr(p,'\n',rcv_len - (p-buf));
			p++;

			if(db->kvn != _dbatoi(p))
			{
				db->rno = DB_ERR;
				db->kvn = 0;
				goto OUT;
			}

			db->kvn = 0;
			break;
		}
		case DB_EXISTS:
		{
			if(_dbatoi(p) == 1 && _dbatoi(p+2) == 1)
				db->rno = DB_EXIST;
			else
				db->rno = DB_NO_EXIST;

			break;
		}
		case DB_SCAN:
		{
			db->rno = _dbsplitkv(db, p, rcv_len - (p - buf) ,DB_SCAN_OK);
			break;
		}
		case DB_FLUSHDB:
		{
			db->rno = DB_FLUSHDB_OK;
			break;
		}
		case DB_DBSIZE:
		{
			db->rno = DB_DBSIZE_OK;
			len = _dbatoi(p);
			p = memchr(p,'\n',rcv_len - (p - buf));
			val = ++p;
			*(p+len) = '\0';
			
			db->kvs[idx].val = val;
			db->kvs[idx].val_len = len;
			db->kvn++;
			break;
		}
	}	
OUT:
	return;
}

void _dbdo(_s8* data, _u32 datalen, DB* db, _u16 cmd)
{
	dbconn_t *conn = NULL;
	_u32 rcv_len = 0;
	_s8 flag = 0;
	_s8 trycnt = 0;

REDO:
	if(flag)
	{
		flag = !flag;
		_db_conn_destory(conn);
	}

	if(trycnt++ > 3)
	{
		goto ERR;
	}
	
	conn = _db_get_conn();

	if(!conn)
	{
		db->rno = DB_CONNECT_ERR;
		goto ERR;
	}
	// TODO:check conn
	if(_dbsend(conn,data,datalen))
	{
		flag = !flag;
		db->rno = DB_CONNECT_ERR;
		goto REDO;
	}

	if(_dbrecv(conn,&db->src.buf,&rcv_len))
	{	
		flag = !flag;
		db->rno = DB_CONNECT_ERR;
		goto REDO;
	}
	_db_conn_free(conn);
	
	_dbphrase(db->src.buf, rcv_len, db, cmd);	

ERR:
	return;
}

void _dbcmd(_u16 cmd, _s8* data, _s32 datalen, DB* db)
{
	_s32 cmdlen;
	_s32 _datalen;
	_s8* cmd_str = _dbstr[cmd];
	_s8* _data = (_s8*)wys_malloc(strlen(cmd_str) + datalen + 64);

	if(!_data)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}
	
	cmdlen = sprintf(_data,"%d\n%s\n",strlen(cmd_str),cmd_str);
	_datalen = cmdlen+datalen;

	if(data && datalen)
	{
		memcpy(_data + cmdlen, data, datalen);
	}
	memset(_data + _datalen, '\n', 1);	

	_dbdo(_data,_datalen+1,db, cmd);
ERR:
	wys_free(_data);
	return;
}

_sin DB* _dbcreate(void)
{	  
	return (DB*)wys_malloc(sizeof(DB));
}

DB* dbget(_s8* key, _s32 key_len)
{
	DB* db = NULL;
	db = _dbcreate();
	_s8* data = NULL;
	_u32 data_len = 0;
	
	if(!db)
	{
		goto ERR;
	}

	if(!key || !key_len)
	{
		db->rno = DB_INVALID_PARMS;
		goto ERR;
	}

	db->kvs = (db_kv*)wys_malloc(sizeof(db_kv));

	if(!db->kvs)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	db->kvs[0].key = (_s8*)wys_malloc(key_len+1);
	db->kvs[0].key_len = key_len;

	if(!db->kvs[0].key)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	memcpy(db->kvs[0].key,key,key_len);
	db->src.sts |= FREE_KVS_0_KEY;
	
	data = (_s8*)wys_malloc(key_len+16);

	if(!data)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	data_len = sprintf(data, "%d\n%s\n",key_len, key);

	_dbcmd(DB_GET,data, data_len, db);
ERR:
	wys_free(data);
	return db;
}

DB* dbset(_s8* key, _s32 key_len, _s8* val, _s32 val_len)
{
	DB*	db = NULL;
	_s8* data = NULL;
	_s32 datalen;

	db = _dbcreate();

	if(!db)
	{
		goto ERR;
	}
	if(!key || !val)
	{
		db->rno = DB_INVALID_PARMS;
		goto ERR;
	}
	
	if(!strlen(key) || val_len <= 0)
	{
		db->rno = DB_INVALID_PARMS;
		goto ERR;
	}

	data = (_s8*)wys_malloc(key_len+val_len+64);

	if(!data)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	datalen = snprintf(data,key_len+val_len+64,"%d\n%s\n%d\n",key_len,key,val_len);

	memcpy(data + datalen, val, val_len);
	memcpy(data + datalen + val_len,"\n",1);
	
	datalen = datalen + val_len +1;
	_dbcmd(DB_SET,data,datalen,db);	
ERR:
	wys_free(data);
	return db;
}

DB* dbscan(_s8* start, _s32 start_len, _s8* end, _s32 end_len, _s32 max)
{
	_s32 max_len = 0;;
	DB* db = NULL;
	_s8* data = NULL;
	_s8 data_len = 0;
	db = _dbcreate();

	if(!db)
	{
		goto ERR;
	}

	if(!start || !end || max < 1)
	{
		db->rno = DB_INVALID_PARMS;
		goto ERR;
	}

	max_len = _dbnumlen(max);
	
	data = (_s8*)wys_malloc( start_len + end_len + max_len + 64);

	if(!data)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	data_len = sprintf(data,"%d\n%s\n%d\n%s\n%d\n%d\n",start_len,start,end_len,end,max_len,max);

	_dbcmd(DB_SCAN,data,data_len,db);

ERR:
    wys_free(data);
	return db;
}

_s32 dbinit(_s8* addr, _u16 port)
{
	_s32 i;
	struct timeval tv;
	gettimeofday (&tv, NULL);	
	
	_dbaddr = inet_addr(addr);
	_dbport = htons(port);	
	_start_second = tv.tv_sec;

	for(i = 0; i < DB_HASH_MAX; i++)
	{
		INIT_HLIST_HEAD(&_db_pool_conn_hash[i]);
	}

	INIT_LIST_HEAD(&_db_free_conn_list);
	
	return 0;
}

void dbfree(DB* db)
{

	if(!db)
	{
		goto OUT;
	}
	else
	{
		if(db->src.sts&FREE_KVS_0_KEY)
		{
			wys_free(db->kvs[0].key);
		}
		wys_free(db->src.buf);
		wys_free(db->kvs);
		wys_free(db);
	}
OUT:
	return;
}

DB* dbdel(_s8* key, _s32 key_len)
{
	DB* db = NULL;
	db = _dbcreate();
	_s8* data = NULL;
	_u32 data_len = 0;
	
	if(!db)
	{
		goto ERR;
	}

	if(!key || !key_len)
	{
		db->rno = DB_INVALID_PARMS;
		goto ERR;
	}

	data = (_s8*)wys_malloc( key_len+16);

	if(!data)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	data_len = sprintf(data, "%d\n%s\n",key_len, key);

	_dbcmd(DB_DEL,data, data_len, db);
ERR:
	wys_free(data);
	return db;	
}

const _s8* dberr(_u16 dbrno)
{
	if(dbrno >= DB_RNO_MAX)
	{
		return _dbret_str[0];
	}
	else
	{
		return _dbret_str[dbrno];
	}
}

DB* dbexists(_s8* key, _s32 key_len)
{
	DB* db = NULL;
	db = _dbcreate();
	_s8* data = NULL;
	_u32 data_len = 0;
	
	if(!db)
	{
		goto ERR;
	}

	if(!key || !key_len)
	{
		db->rno = DB_INVALID_PARMS;
		goto ERR;
	}

	data = (_s8*)wys_malloc( key_len+16);

	if(!data)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	data_len = sprintf(data, "%d\n%s\n",key_len, key);

	_dbcmd(DB_EXISTS,data, data_len, db);
ERR:
	wys_free(data);
	return db;	
}

DB* dbmulti_set(db_kv* kvs, _s32 kvn)
{
	_s32 total_len = 0, i;
	_u32 data_len = 0;
	_s8* data = NULL;
	DB* db = NULL;


	db = _dbcreate();

	if(!db)
	{
		goto ERR;
	}

	if(!kvs || kvn < 1)
	{
		db->rno = DB_INVALID_PARMS;
		goto ERR;
	}
	
	for(i = 0; i < kvn; i++)
	{
		total_len += (kvs[i].key_len + kvs[i].val_len + _dbnumlen(kvs[i].key_len) + _dbnumlen(kvs[i].val_len) + 4);
	}


	total_len += 64;

	data = (_s8*)wys_malloc( total_len);

	if(!data)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	for(i = 0; i < kvn; i++)
	{
		data_len += sprintf(data + data_len, "%d\n%s\n%d\n", kvs[i].key_len, kvs[i].key, kvs[i].val_len);
		memcpy(data + data_len, kvs[i].val, kvs[i].val_len);
		data_len += kvs[i].val_len;
		memset(data + data_len , '\n', 1);
		data_len += 1;
	}
	//后续验证的时候需要
	db->kvn = kvn;
	_dbcmd(DB_MULTI_SET,data, data_len, db);
	
ERR:
	wys_free(data);
	return db;
	
}


DB* dbmulti_get(db_key* keys, _s32 key_num)
{
	_s32 i;
	_s32 total_len = 0;
	_u32 data_len = 0;
	_s8* data = NULL;
	DB* db = NULL;

	db = _dbcreate();

	if(!db)
	{
		goto ERR;
	}

	if(!keys || key_num < 1)
	{
		db->rno = DB_INVALID_PARMS;
		goto ERR;
	}
	
	for(i = 0; i < key_num; i++)
	{
		total_len += (keys[i].key_len + _dbnumlen(keys[i].key_len) + 2);
	}


	total_len += 64;

	data = (_s8*)wys_malloc( total_len);

	if(!data)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	for(i = 0; i < key_num; i++)
	{
		data_len += sprintf(data + data_len, "%d\n%s\n", keys[i].key_len, keys[i].key);
	}

	_dbcmd(DB_MULTI_GET,data, data_len, db);
	
ERR:
	wys_free(data);
	return db;
}



DB* dbmulti_del(db_key* keys, _s32 key_num)
{
	_s32 i;
	_s32 total_len = 0;
	_u32 data_len = 0;
	_s8* data = NULL;
	DB* db = NULL;


	db = _dbcreate();

	if(!db)
	{
		goto ERR;
	}

	if(!keys || key_num < 1)
	{
		db->rno = DB_INVALID_PARMS;
		goto ERR;
	}
	
	for(i = 0; i < key_num; i++)
	{
		total_len += (keys[i].key_len + _dbnumlen(keys[i].key_len) + 2);
	}


	total_len += 64;

	data = (_s8*)wys_malloc(total_len);

	if(!data)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	for(i = 0; i < key_num; i++)
	{
		data_len += sprintf(data + data_len, "%d\n%s\n", keys[i].key_len, keys[i].key);
	}

	//数据返回时做对比用,解析数据时会清零
	db->kvn = key_num;
	_dbcmd(DB_MULTI_DEL,data, data_len, db);
	
ERR:
	wys_free(data);
	return db;
}

DB* dbflush(void)
{
	DB* db = NULL;
	db = _dbcreate();
	DB* db_log = NULL;
	DB* db_cpt = NULL;
	_s8* data = NULL;
	_u32 data_len = 0;
	
	if(!db)
	{
		goto ERR;
	}

	db_log = _dbcreate();
	if(!db_log)
	{
		goto ERR;
	}

	db_cpt = _dbcreate();
	if(!db_cpt)
	{
		goto ERR;
	}
	
	_dbcmd(DB_FLUSHDB, data, data_len, db);
	_dbcmd(DB_CLR_BINLOG, data, data_len, db_log);
	_dbcmd(DB_COMPACT, data, data_len, db_cpt);

	dbfree(db_log);
	dbfree(db_cpt);
ERR:
	return db;
}

DB* dbsize()
{
	DB* db = NULL;
	db = _dbcreate();
	DB* db_cpt = NULL;
	_s8* data = NULL;
	_u32 data_len = 0;
	
	if(!db)
	{
		goto ERR;
	}

	db->kvs = (db_kv*)wys_malloc(sizeof(db_kv));

	if(!db->kvs)
	{
		db->rno = DB_OUT_MEM;
		goto ERR;
	}

	db_cpt = _dbcreate();

	_dbcmd(DB_COMPACT, data, data_len, db_cpt);
	_dbcmd(DB_DBSIZE,data, data_len, db);

	
ERR:
	dbfree(db_cpt);
	wys_free(data);
	return db;
}

