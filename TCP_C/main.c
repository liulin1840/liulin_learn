/////////////////////////////////////////////////////////////////////
/// @file main.c
/// @brief 主程序,主要负责初始化与TCP连接处理
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "list.h"
#include "timer.h"
#include "db.h"
#include "mem.h"
#include "debug.h"
#include "allocator.h"
#include "auto_data_save.h"
#include "worker.h"
#include "hash.h"

/*********************** MACROS **********************/
#define CLOSESOCK(fd) shutdown(fd,SHUT_RDWR);close(fd)

#define AN_LISTEN_MAX	1024
#define AN_POLL_MAX 	1024
#define AN_HASH_MASK 	0XFF
#define AN_HASH_MAX 	(AN_HASH_MASK + 1)
#define AN_HASH_INDEX(idx) (((_u32)idx) & AN_HASH_MASK)
#define AN_CLOSE_SOCK(fd) shutdown(fd,SHUT_RDWR);close(fd)
#define AN_UDP_BUF_LEN	1024
#define AN_TCP_BUF_LEN	1024
#define AN_SSDB_CONF	"/usr/sbin/waterwall.conf"

#define SEND_ERROR		-1
#define SEND_AGAIN		0
#define SEND_FINISH		1
#define SEND_SHUTDOWN	2

/****************** DATA STRUCTRUE *******************/

/***************** GLOBAL VARIABLES ******************/
extern char WAYOS[];
_s32 an_curfds = 0;///< 当前socket fd数量
_s32 an_udp_fd = -1;///< 监听用udp fd(暂未使用)
_s32 an_tcp_fd = -1;///< 监听用tcp fd
_s32 an_stop = 0;
_u16 an_udp_port = 7777;///< udp监听端口 (暂未使用)
_u16 an_tcp_port = 7676;///< tcp监听端口
struct pollfd an_conn_array[AN_POLL_MAX];///< pollfd数组,含有所有连接fd

//TCP接收/发送缓存
_s32 gl_send_len = 0;		 ///< 当前已处理数据字节数(不含'\0',多线程需更改方式)
_s32 max_send_len = 204800; ///< 发送最大字节数，暂定
_s32 max_buf_len = 204000;	 ///< 最大发送具体内容(其他字节留给key等额外内容)
/********************* FUNCITONS *********************/
void an_exit(int line, int flag)
{
	INFO("========================== EXIT ===========================\n");
	exit(flag);
}
//----------------------------------------------------------------
// 函数名称：an_sigfunc
/// @brief 信号量处理函数,打印捕获值并退出程序
/// 
/// @param _in signo->信号量
/// @return void->无返回值
/// @author xxx
//---------------------------------------------------------------
static void an_sigfunc(int signo)
{
	char* sigtype = NULL;

	switch(signo)
	{
		case SIGTERM:
			sigtype = "SIGTERM";
			break;
		case SIGINT:
			sigtype = "SIGINT";
			break;
		case SIGSEGV: 
			sigtype = "SIGSEGV";
			break;
		case SIGBUS:
			sigtype = "SIGBUS";		
			break;
		default:
			sigtype = "UNKNOW";		
	}
	
	INFO("\ncatch signal:%d type:%s\n",signo,sigtype);
	an_exit(__L__,0);
}

//----------------------------------------------------------------
// 函数名称：an_signals_init
/// @brief 初始化信号量处理函数
/// 
/// @param void->无参
/// @return void->无返回值
/// @author xxx
//---------------------------------------------------------------
void an_signals_init(void)
{
	struct sigaction sa;
	sigaction(SIGPIPE, &sa, 0);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM,an_sigfunc);
	signal(SIGINT,an_sigfunc);
	signal(SIGSEGV,an_sigfunc);
	signal(SIGBUS,an_sigfunc);
}

//----------------------------------------------------------------
// 函数名称：an_poll_init
/// @brief 初始化an_conn_array数组
/// 
/// @param void->无参
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_poll_init(void)
{
	_s32 i;
	
	an_curfds = 1;
//	an_conn_array[0].fd = an_udp_fd;
	an_conn_array[0].fd = an_tcp_fd;
	an_conn_array[0].events = POLLIN;

	for(i = 1; i < AN_POLL_MAX; i++)
	{
		an_conn_array[i].fd = -1;
		an_conn_array[i].events = POLLIN;	
	}
	
	return;
}

//----------------------------------------------------------------
// 函数名称：an_sock_noblock
/// @brief 设置socket fd为非阻塞
/// 
/// @param _in fd->socket fd
/// @return -1->failed,0->sucess
/// @author xxx
//---------------------------------------------------------------
_s32 inline an_sock_noblock(int fd)
{
	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0)|O_NONBLOCK))
        return -1;
    return 0;
}

//----------------------------------------------------------------
// 函数名称：an_tcp_sock_init
/// @brief 初始化tcp监听
/// 
/// @param void->无参数
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
int an_tcp_sock_init(void)
{
	int ret = -1;
	int lisnum = AN_LISTEN_MAX;
	int reuse_on = 1;
	int trycnt = 0;
	struct sockaddr_in addr;
	
	if((an_tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		ERROR("socket(AF_INET, SOCK_STREAM, 0)");
		goto ERR;
	}

	if(an_sock_noblock(an_tcp_fd))
	{
		ERROR("an_sock_noblock(m_tcp_fd)");
		goto ERR;
	}

	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(an_tcp_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	setsockopt(an_tcp_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_on, sizeof(reuse_on));

	if((bind(an_tcp_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr))) == -1)
	{
		for(trycnt = 0; trycnt < 10; trycnt++)
		{
			if((bind(an_tcp_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr))) != -1)
			{
				goto NEXT;
			}
		}

		ERROR("bind()");
		goto ERR;
	}
NEXT:
	if(listen(an_tcp_fd, lisnum) == -1)
	{
		ERROR("listen()");
		goto ERR;
	}

	ret = 0;
	INFO("init TCP listen sock OK.port:%u.\n", an_tcp_port);
ERR:
	return ret;
}

//----------------------------------------------------------------
// 函数名称：an_udp_sock_init
/// @brief 初始化udp监听(暂未使用)
/// 
/// @param void->无参数
/// @return -1->failed,0->sucess
/// @author xxx
//---------------------------------------------------------------
_s32 an_udp_sock_init(void)
{
	_s32 ret = -1;
	_s32 reuse_on = 1;
	_s32 trycnt = 0;
	struct sockaddr_in addr;
	struct linger so_linger;

	if((an_udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		ERROR("socket(AF_INET, SOCK_DGRAM, 0)");
		goto ERR;
	}

	if(an_sock_noblock(an_udp_fd))
	{
		ERROR("an_sock_noblock(an_udp_svrfd)");
		goto ERR;
	}

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(an_udp_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	setsockopt(an_udp_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_on, sizeof(reuse_on));

	so_linger.l_onoff = 1;
	so_linger.l_linger = 5;
	setsockopt(an_udp_fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));

	if((bind(an_udp_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr))) == -1)
	{
		for(trycnt = 0; trycnt < 10; trycnt++)
		{
			if((bind(an_udp_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr))) != -1)
			{
				goto NEXT;
			}
		}

		ERROR("bind()");
		goto ERR;
	}
NEXT:	
	ret = 0;
	INFO("init UDP sock OK.port:%u.\n", an_udp_port);
ERR:
	return ret;
}

//----------------------------------------------------------------
// 函数名称：an_udp_recv
/// @brief udp recv处理函数(暂未使用)
/// 
/// @param void->无参数
/// @return void->无返回值
/// @author xxx
//---------------------------------------------------------------
void an_udp_recv(void)
{
	_s8* udp_buf = NULL;
	struct sockaddr_in from_addr;
	_s32 from_len = sizeof(from_addr);
	_s32 recv_len = 0;
	
	udp_buf = (_s8*)wys_malloc(AN_UDP_BUF_LEN);

	if(!udp_buf)
	{
		ERROR("malloc()");
		goto ERR;
	}

	// TODO:还要考虑超过AN_UDP_BUF_LEN大小的数据的接受问题
	recv_len = recvfrom(an_udp_fd, udp_buf, AN_UDP_BUF_LEN 
	, 0, (struct sockaddr*)&from_addr, (socklen_t*)&from_len);

	if(recv_len <= 0)
	{
		goto OUT;
	}

//	al_allocate_task(udp_buf, 0);

ERR:
	return;
OUT:
	wys_free(udp_buf);
	return;
}

//----------------------------------------------------------------
// 函数名称：an_udp_send
/// @brief udp send处理函数(暂未使用)
/// 
/// @param _in dest->目的地址
/// @param _in buf->发送内容
/// @param _in buf_len->buf长度
/// @return -1->failed,0->sucess
/// @author xxx
//---------------------------------------------------------------
_s32 an_udp_send(struct sockaddr_in * dest, void * buf, _s32 buf_len)
{
	int len= 0;
	int fd = 0;
	int reuse_on = 1;
	if(!buf || buf_len < 1)
		return -1;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if(fd < 0)
	{
		INFO("socket failed!\n");
		len = -1;
		goto ERR;
	}
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse_on, sizeof(reuse_on));
		
	len = sendto(fd, buf,buf_len,0, (struct sockaddr*)dest, sizeof(struct sockaddr_in));

	if(len != buf_len)
	{
		INFO("send not complete!\n");
		len = -1;
		goto ERR;
	}
	
ERR:
	if(fd > 0)
	{
		close(fd);
		shutdown(fd,SHUT_RDWR);
	}
	return len;		
}

//----------------------------------------------------------------
// 函数名称：an_sock_poll_del
/// @brief 从an_conn_array数组中清理单个pollfd
/// 
/// @param _in fd->需要清理的fd
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_sock_poll_del(_s32 fd)
{
	_s32 i;
	_s32 ret = -1;
	
	for(i = 0; i < AN_POLL_MAX; i++)
	{
		if(fd == an_conn_array[i].fd)
		{
			an_conn_array[i].fd = -1;
			an_conn_array[i].events = POLLIN;
			ret = 0;
			if(i == an_curfds-1)
			{
				an_curfds--;
			}
			break;
		}
	}

	return ret;
}

void m_sock_del_write(_s32 index)
{	
	an_conn_array[index].events = (an_conn_array[index].events & (~POLLOUT));
	return;	

}
void m_sock_del_read(_s32 index)
{	
	an_conn_array[index].events = (an_conn_array[index].events & (~POLLIN));
	return;	

}

void m_sock_add_write(_s32 index)
{
	an_conn_array[index].events = (an_conn_array[index].events | POLLOUT);
	return;	
}

void m_sock_add_read(_s32 index)
{
	an_conn_array[index].events = (an_conn_array[index].events | POLLIN);
	return;	
}

//----------------------------------------------------------------
// 函数名称：an_sock_poll_del
/// @brief 从an_conn_array数组中断开单个socket连接
/// 
/// @param _in index->需要断开的连接在an_conn_array数组中的下标
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_sock_destory(_s32 index)
{
	CLOSESOCK(an_conn_array[index].fd);
	an_socket_t_del(index);
	an_sock_poll_del(an_conn_array[index].fd);
}

//----------------------------------------------------------------
// 函数名称：an_split_rcv_buf_to_json
/// @brief tcp切包
/// 
/// @param _in sock_t->需要切分的an_socket_t结构体
/// @return -1->failed,0->sucess
/// @note 分包并交给allocator处理 || 一个json包头标识为\n,之后才是完整json包
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_split_rcv_buf_to_json(an_socket_t* sock_t)
{
	_s32 ret = -1;

	_s8* json_data = NULL;
	_s32 json_len = 0;

	_s8* start_p = sock_t->rcv_buf;
	_s8* end_p = sock_t->rcv_buf;
	_s32 cut_len = 0;

	_s32 start_flag = 0;
	_s32 packet_flag = 0;

	if(!start_p)
		goto OUT;

	while(*start_p && cut_len < sock_t->rcv_len)
	{
		if(*start_p != '\n')
		{
			start_p++;
			cut_len++;
		}
		else
			break;
	}

	if(cut_len > 0)
	{
		memmove(sock_t->rcv_buf, sock_t->rcv_buf + cut_len, sock_t->rcv_len - cut_len);
		sock_t->rcv_len -= cut_len;
	}

	INFO("sock_t->rcv_buf:%s\n",sock_t->rcv_buf);

	/*
	TODO (黄历云): 下面的代码试图处理一个包中有多个命令的情况，但是实际上好像处理不了
	当packet_flag大于1时退出了,请仔细检查一下
	*/
REDO:
	start_p = sock_t->rcv_buf;
	end_p = sock_t->rcv_buf;
	packet_flag = 0;

	while(json_len < sock_t->rcv_len)
	{
		if(*end_p == '{')
		{
			start_flag++;
		}
		else if(*end_p == '}')
		{
			start_flag--;
		}
		else if(*end_p == '\n')
		{
			packet_flag++;
			end_p++;
			start_p = end_p;
			if(packet_flag > 1)
				goto OUT;
			continue;
		}

		json_len++;
		end_p++;

		if(start_flag == 0 && packet_flag == 1)
		{
			break;
		}
	}

	if(json_len > 0 && start_flag == 0)
	{
		json_data = wys_malloc(json_len+1);
		if(!json_data)
		{
			goto OUT;
		}
		snprintf(json_data,json_len+1,"%s",start_p);
		//memmove+1为'\n'占的标识位
		memmove(sock_t->rcv_buf, sock_t->rcv_buf + json_len + 1, sock_t->rcv_len - json_len + 1);
		INFO("rcv_len = %d\n",sock_t->rcv_len);
		INFO("json_len = %d\n",json_len);
		sock_t->rcv_len -= (json_len + 1);
		al_allocate_task(json_data, json_len, sock_t->index);
	}

	if(sock_t->rcv_len > 0 && start_flag == 0)
		goto REDO;

	ret = 0;
OUT:
	return ret;
}

//----------------------------------------------------------------
// 函数名称：an_tcp_recv
/// @brief tcp recv处理函数
/// 
/// @param _in index->需要recv的连接在an_conn_array数组中的下标
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_tcp_recv(_s32 index)
{
 	_s32 ret = -1;
	_s32 r_len = 0;
	_s32 fd = an_conn_array[index].fd;
	static _s8 r_buf[AN_TCP_BUF_LEN] = {0};
	
	an_socket_t* sock_t = an_socket_t_find_by_index(index);
	if(!sock_t)
	{
		goto OUT;
	}
	
	memset(r_buf, 0x00, sizeof(r_buf));
	r_len = recv(fd, r_buf, AN_TCP_BUF_LEN, 0);
	if(r_len <= 0)
	{
		goto OUT;
	}

	if(sock_t->rcv_buflen < r_len + sock_t->rcv_len)
	{
		sock_t->rcv_buf = wys_realloc(sock_t->rcv_buf, r_len + sock_t->rcv_len);

		if(!sock_t->rcv_buf)
		{
			ERROR("realloc()");
			goto OUT;
		}
		sock_t->rcv_buflen = r_len + sock_t->rcv_len;
	}

	memcpy(sock_t->rcv_buf + sock_t->rcv_len, r_buf, r_len);
	sock_t->rcv_len += r_len;

	ret = an_split_rcv_buf_to_json(sock_t);
	if(ret)
		goto OUT;

	ret = 0;

OUT:
	return ret;
}

//----------------------------------------------------------------
// 函数名称：an_ignore_errno
/// @brief 忽略部分errno
/// 
/// @param _in eno->错误值
/// @return -1->不忽略,0->忽略
/// @author xxx
//---------------------------------------------------------------
_s32 an_ignore_errno(_s32 eno)
{
	_s32 ret = -1;

	switch(eno)
	{
		case EAGAIN:
		case EINTR:
			ret = 0;
			break;
		default:
			break;
	}

	return ret;
}

//----------------------------------------------------------------
// 函数名称：an_tcp_send
/// @brief tcp send处理函数
/// 
/// @param _in index->需send的连接在an_conn_array中的下标
/// @return -1->发送失败,0->未发送完,1->发送完成,2->连接断开
/// @note 除未发送完的情况,其他返回值均主动断开连接
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_tcp_send(_s32 index)
{
	_s32 send_len = 0;
	_s32 send_stat;
	int fd = an_conn_array[index].fd;
	an_socket_t* sock_t = NULL;
	sock_t = an_socket_t_find_by_index(index);
	if(!sock_t)
	{
		send_stat = SEND_SHUTDOWN;
		goto OUT;
	}
	
	//此处由于lua服务器问题，切割数据后再发送(2016.11.08已解决,不用切割)
	if(sock_t->send_len)
	{
		/*if(sock_t->send_len >= lua_send_len)
		{
			send_len = send(fd, sock_t->send_buf, lua_send_len, 0);
		}
		else*/
		{
			send_len = send(fd, sock_t->send_buf, sock_t->send_len, 0);
		}
	}

	if (send_len< 0) 
	{
		if (an_ignore_errno(errno)) 
		{			
			send_stat = SEND_ERROR;
			goto OUT;
		}
		m_sock_add_write(index);
		send_stat = SEND_AGAIN;
	}
	else if(send_len == 0)
	{
		send_stat = SEND_SHUTDOWN;
		goto OUT;
	}
	else
	{
		// TODO:根据WEB服务器处理方式此处可能需要修改
		if (send_len < sock_t->send_len)
		{
			m_sock_del_write(index);
			send_stat = SEND_AGAIN;
		}
		/*else if(sock_t->send_len >= lua_send_len)
		{
			memmove(sock_t->send_buf, sock_t->send_buf+lua_send_len, sock_t->send_len - lua_send_len);
			sock_t->send_len -= lua_send_len;
			send_stat = SEND_AGAIN;
		}*/
		else
		{
			m_sock_del_write(index);
			send_stat = SEND_FINISH;
			wys_free(sock_t->send_buf);
			sock_t->send_buf = NULL;
			sock_t->send_len = 0;
		}
	}	

OUT:
	return send_stat;	
}

//----------------------------------------------------------------
// 函数名称：an_tcp_events_do
/// @brief tcp事件处理函数
/// 
/// @param _in index->需处理的连接在an_conn_array中的下标
/// @return -1->error,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_tcp_events_do(_s32 index)
{
	_s32 ret = -1;
	_u32 send_stat = 0;

	if(an_conn_array[index].revents & POLLERR)
	{
		an_sock_destory(index);
		goto ERR;
	}
	else
	{
		if(an_conn_array[index].revents & POLLOUT)
		{
			send_stat = an_tcp_send(index);
			//发完就断
			if(send_stat == SEND_FINISH)
			{
				m_sock_del_write(index);
				an_sock_destory(index);
				goto OUT;
			}
			else if(send_stat == SEND_AGAIN)
			{
				m_sock_add_write(index);
				goto OUT;
			}
			else
			{
				an_sock_destory(index);
				goto OUT;
			}
		}

		if(an_conn_array[index].revents & POLLIN)
		{
			if(an_tcp_recv(index))
			{
				an_sock_destory(index);
				goto OUT;
			}
		}
	}
	
OUT:
	ret = 0;
ERR:
	return ret;
}

//----------------------------------------------------------------
// 函数名称：an_sock_poll_add
/// @brief 新增连接处理函数
/// 
/// @param _in fd->新增连接的socket fd
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_sock_poll_add(_s32 fd)
{
	_s32 i;
	an_socket_t* sock_t = NULL;
	_s32 index = 0;

	for(i = 1; i < AN_POLL_MAX; i++)
	{
		if(an_conn_array[i].fd < 0)
		{
			an_conn_array[i].fd = fd;
			an_conn_array[i].revents = POLLIN;
			break;
		}
	}

	if(i == AN_POLL_MAX)
	{
		DEBUG("sock poll is full.\n");
		return -1;
	}

	if(i == an_curfds)
	{
		an_curfds++;
	}

	sock_t = an_socket_t_create();
	if(sock_t)
	{
		index = an_socket_t_get_hash(i);
		hlist_add_head(&sock_t->hash, &an_sock_hash[index]);
		sock_t->send_buflen = max_send_len;
		sock_t->index = i;
	}
	else
	{
		DEBUG("socket_t create error\n");
		return -1;
	}
	
	return 0;
	
}

//----------------------------------------------------------------
// 函数名称：an_poll
/// @brief poll处理函数
/// 
/// @param _in fd->新增连接的socket fd
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_poll(_s32 interval)
{
	_s32 nready, newfd; 
	_u32 len, i;
	struct sockaddr_in peer_addr;
	
	nready = poll(an_conn_array, an_curfds, interval);

	if(nready < 1)
	{
		return 0;
	}

	if(an_conn_array[0].revents & POLLIN)
	{
		if(1)
		{
			len = sizeof(struct sockaddr_in);
			newfd = accept(an_tcp_fd, (struct sockaddr*)&peer_addr, &len);
			if(newfd < 0)
			{
				INFO("accept err\n");
			}
			else
			{
				INFO("accept ok\n");
				if(an_sock_poll_add(newfd))
				{
					goto OUT;
				}
			}
		}
	}

	for(i = 1; i < an_curfds; i++)
	{
		if(an_conn_array[i].fd == -1)
			continue;
		if(an_conn_array[i].revents & (POLLIN | POLLOUT | POLLERR))
		{			
			if(an_tcp_events_do(i))
			{
				goto ERR;
			}
		}
	}
	
OUT:
	return 0;
ERR:
	an_exit(__L__, 0);
	return -1;
}

//----------------------------------------------------------------
// 函数名称：an_timer_init
/// @brief 初始化timer
/// 
/// @param void->无参数
/// @return void->无返回值
/// @author xxx
//---------------------------------------------------------------
void an_timer_init(void)
{
	jiffies_init();
	init_timers_cpu();
	srand(time(0));

	return;
}

//----------------------------------------------------------------
// 函数名称：an_database_init
/// @brief 初始化数据库地址
/// 
/// @param void->无参数
/// @return -1->failed,0->sucess
/// @author xxx
//---------------------------------------------------------------
_s32 an_database_init(void)
{

	_s32 ret = -1;
	_s32 tmp_len;
	_s8 *buf = NULL;
	_s8 *tmp, *tmp_end;
	_s8 ip[32] = {0};
	_s8 port[8] = {0};
	FILE* fd;	
	
	//读取配置文件
	if(access(AN_SSDB_CONF, F_OK))
	{
		INFO("config file [%s] doesn't exists!\n",AN_SSDB_CONF);
	}
	
	fd = fopen(AN_SSDB_CONF, "rb");
	if(!fd)
	{
		ERROR("fopen()");
		goto ERR;
	}

	fseek(fd,0,SEEK_END);
	tmp_len = ftell(fd);
	rewind(fd);

	if(tmp_len <= 0)
	{
		DEBUG("conf file length err!\n");
		goto ERR;
	}

	buf = (_s8*)wys_malloc(tmp_len);
	if(!buf)
	{
		ERROR("malloc()");
		goto ERR;
	}
	
	fread(buf, 1, tmp_len, fd);

	tmp = strstr(buf,"[ssdb-conf]\n");
	if(!(tmp = strstr(buf,"[ssdb-conf]\n")))
	{
		INFO("Can not read ssdb-conf from %s\n",AN_SSDB_CONF);
		goto ERR;
	}

	if(!(tmp = strstr(buf + 12, "ssdb_ipaddr")) || !(tmp = strchr(tmp,':')) 
	|| !(tmp++ ,tmp_end = strchr(tmp,'\n')))
	{
		INFO("Can not read ssdb_ipaddr from %s\n",AN_SSDB_CONF);
		goto ERR;
	}

	strncpy(ip, tmp, tmp_end - tmp);

	if(!(tmp = strstr(tmp_end,"ssdb_port")) || !(tmp = strchr(tmp,':'))
	|| !(tmp++, tmp_end = strchr(tmp,'\n')))
	{
		INFO("Can not read ssdb_ipaddr from %s\n",AN_SSDB_CONF);
		goto ERR;
	}

	strncpy(port, tmp, tmp_end - tmp);
	INFO("Datebase addr:%s:%s\n",ip,port);
	
	dbinit(ip,(_u16)atoi(port));

	ret = 0;
ERR:
	if(fd)fclose(fd);
	fd = NULL;
	wys_free(buf);
	return ret;

}

void an_mem_init(void)
{
	return wys_mem_init();
}

//----------------------------------------------------------------
// 函数名称：an_init
/// @brief 主程序初始化
/// 
/// @param void->无参数
/// @return -1->failed,0->sucess
/// @author xxx
//---------------------------------------------------------------
_s32 an_init(void)
{
	INFO("%s\n",WAYOS);
	
	an_signals_init();

	an_mem_init();

/*	if(an_udp_sock_init())
	{
		DEBUG("upd server listlen sock init failed!\n");
		goto OUT;
	}*/

	an_hash_and_list_init();

	if(an_tcp_sock_init())
	{
		DEBUG("server listen sock init failed!\n");
		goto OUT;
	}

	an_poll_init();

	an_timer_init();

	if(an_data_save_init())
	{
		DEBUG("data_save init failed!\n");
		goto OUT;
	}

	if(an_database_init())
	{
		DEBUG("Database init failed!\n");
		goto OUT;
	}

	if(wk_init())
	{
		DEBUG("woker init failed!\n");
		goto OUT;
	}

	INFO("=================== Init OK. Run Now! =====================\n");

	return 0;
OUT:
	return -1;
}

//----------------------------------------------------------------
// 函数名称：main
/// @brief 主函数
/// 
/// @param void->无参数
/// @return 0->退出
/// @author xxx
//---------------------------------------------------------------
_s32 main(_s32 argc, char** argv)
{	

	if(an_init())return 0;

	while(!an_stop)
	{
		if(an_poll(50))an_stop = 1;
		run_timers();
	}
	an_exit(__L__,0);
	return 0;
}

