/*****************************************************
@Copyright (c) 2016 WAYOS.Co.,Ltd. All rights reserved
@Auther    : Jeaham Chain
@Department: R&D
@E-mail	   : chenjinhan@wayos.cn
******************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include <x-list.h>
#include <x-memory.h>
#include <x-types.h>
#include <x-debug.h>

#define MEM_HASH_MASK 0X000003FF
#define MEM_HASH_MAX (MEM_HASH_MASK + 1)
#define MEM_HASH_INDEX(addr) ((addr)&MEM_HASH_MASK) 
#define FNAME_MAX_LEN 0X00000020
#define ASSERT(args,fname,line) do{if(!(args)){INFO("=== fname:%s\n=== line:%d\n",(fname), (line));}assert(args);}while(0);

#define MEM_INFO_RD_LOCK 	pthread_rwlock_rdlock(&wys_meminfo_lock)
#define MEM_INFO_WR_LOCK 	pthread_rwlock_wrlock(&wys_meminfo_lock)
#define MEM_INFO_UNLOCK 	pthread_rwlock_unlock(&wys_meminfo_lock)
#define MEM_SIZE_RD_LOCK 	pthread_rwlock_rdlock(&wys_memsize_lock)
#define MEM_SIZE_WR_LOCK 	pthread_rwlock_wrlock(&wys_memsize_lock)
#define MEM_SIZE_UNLOCK 	pthread_rwlock_unlock(&wys_memsize_lock)

pthread_rwlock_t wys_meminfo_lock;
pthread_rwlock_t wys_memsize_lock;

typedef struct mem_info
{
	struct hlist_node hash_to;
	void* addr;
	_u32  size;
	_u32  line;
	_s8   fname[FNAME_MAX_LEN];
}mem_info_t;

struct hlist_head wys_meminfo_hash[MEM_HASH_MAX];
_s32 total_mem_size = 0;

struct hlist_head* _get_hash(_u32 addr)
{
	_u32 hash = MEM_HASH_INDEX(addr);
	return &wys_meminfo_hash[hash];
}

void _enter_hash(mem_info_t* info)
{
	struct hlist_head* head;
	MEM_INFO_WR_LOCK;
	head = _get_hash((_u32)info->addr);
	hlist_add_head(&info->hash_to, head);
	MEM_INFO_UNLOCK;
	return;
}

void _out_hash(mem_info_t* info)
{
	MEM_INFO_WR_LOCK;
	hlist_del_init(&info->hash_to);
	MEM_INFO_UNLOCK;
	return;
}

mem_info_t* _find_hash(void* addr)
{
	mem_info_t* info;
	struct hlist_node* pos;
	struct hlist_head* head;

	MEM_INFO_RD_LOCK;

	head = _get_hash((_u32)addr);

	hlist_for_each(pos, head)
	{
		info = hlist_entry(pos, struct mem_info, hash_to);
		if(info->addr== addr)
		{
			MEM_INFO_UNLOCK;
			return info;
		}
	}
	MEM_INFO_UNLOCK;
	return NULL;
}

void* _malloc(_u32 size, const _s8* fname, _u32 line)
{
	void* addr = NULL;
	mem_info_t* info = NULL;

	addr = malloc((size_t)size);
	if(!addr)
	{
		goto ERR;
	}

	bzero(addr, size);
	
	info = (mem_info_t*)malloc(sizeof(mem_info_t));
	if(!info)
	{
		free(addr);
		addr = NULL;
		goto ERR;
	}

	bzero(info, sizeof(mem_info_t));
	info->addr = addr;
	info->size = size;
	info->line = line;
	strncpy(info->fname, fname, FNAME_MAX_LEN);
	info->fname[FNAME_MAX_LEN-1] = '\0';
	MEM_SIZE_WR_LOCK;
	total_mem_size += size;
	MEM_SIZE_UNLOCK;
	_enter_hash(info);
	
ERR:
	return addr;
}

void* _realloc(void* ptr, _u32 size, const _s8* fname, _u32 line)
{
	mem_info_t* info = NULL;
	mem_info_t* tmp_info = NULL;
	void* addr = NULL;
	
	if(!ptr)
	{
		return _malloc(size, fname, line);
	}
	else
	{
		if(!size)
		{
			info = _find_hash(ptr);
			ASSERT((info != NULL),fname,line);
			_out_hash(info);
			MEM_SIZE_WR_LOCK;
			total_mem_size -= info->size;
			MEM_SIZE_UNLOCK;
			free(info);
			free(ptr);
		}
		else
		{
			if((info = _find_hash(ptr)))
			{
				ASSERT((info->size <= size),fname,line);

				if(info->size == size)
				{
					return info->addr;
				}

				if(!(addr = realloc(ptr, (size_t)size)))
				{
					tmp_info = _find_hash(ptr);
					ASSERT((tmp_info != NULL),fname,line);
					_out_hash(tmp_info);
					MEM_SIZE_WR_LOCK;
					total_mem_size -= tmp_info->size;
					MEM_SIZE_UNLOCK;
					free(tmp_info);
					free(ptr);					
				}
				else
				{
					MEM_SIZE_WR_LOCK;
					total_mem_size += (size-info->size);
					MEM_SIZE_UNLOCK;
					if(addr == info->addr)
					{
						info->size = size;						
					}
					else
					{
						_out_hash(info);
						info->addr = addr;
						info->size = size;
						info->line = line;
						bzero(info->fname, FNAME_MAX_LEN);
						strncpy(info->fname, fname, FNAME_MAX_LEN);
						info->fname[FNAME_MAX_LEN-1] = '\0';
						_enter_hash(info);
					}
				}
			}
		}
	}

	return addr;
}

void _free(void* ptr,const _s8* fname, _u32 line)
{
	if(!ptr)
		return;

	mem_info_t* info = _find_hash(ptr);
	ASSERT((info != NULL),fname,line);
	MEM_SIZE_WR_LOCK;
	total_mem_size -= info->size;
	MEM_SIZE_UNLOCK;
	_out_hash(info);
	free(info);
	free(ptr);
}

void wys_mem_init(void)
{
	_s32 i;

	for(i = 0; i < MEM_HASH_MAX; i++)
	{
		INIT_HLIST_HEAD(&wys_meminfo_hash[i]);
	}

	pthread_rwlock_init(&wys_meminfo_lock, NULL);
	pthread_rwlock_init(&wys_memsize_lock, NULL);
}

void wys_show_mem_info(void)
{
	mem_info_t* info;
	struct hlist_node* pos = NULL;
	struct hlist_head* head = NULL;
	_s32 i;

	INFO("\n+-----------------------------------------------+\n");
	INFO("|                   Mem Info                    |\n");
	INFO("+-----------------------------------------------+\n");
	MEM_SIZE_RD_LOCK;
	INFO("|            Total Size : %-6d                |\n",total_mem_size);
	MEM_SIZE_UNLOCK;
	INFO("+------------+-------+-------+------------------+\n");
	INFO("|   Address  | Size  | Line  |     Function     |\n");
	INFO("+------------+-------+-------+------------------+\n");
	for(i = 0; i < MEM_HASH_MAX; i++)
	{
		if((head = &wys_meminfo_hash[i]))
		{
			hlist_for_each(pos, head)
			{
				info = hlist_entry(pos, struct mem_info, hash_to);
				INFO("| 0x%08x | %-5d | %-5d | %-16.16s |\n"
				, (_u32)info->addr
				, info->size
				, info->line
				, info->fname);
			}
		}
	}
	INFO("+------------+-------+-------+------------------+\n");
}

void wys_hex_dump(_u8 *buf,_s32 len)
{	
	int i,j,k,l = 0;
	char binstr[0X50];
	_s32 addr = (_s32)buf;
	INFO("\n+----------+-------------------------------------------------+------------------+\n");
	INFO("|   addr   |                      hex                        |  avalible char   |\n");
	INFO("+----------+-------------------------------------------------+------------------+\n");

	for(i = 0; i < len; i++)
	{
		if((i % 0X10) == 0)
		{
			l = 0;
			l = sprintf(binstr, "| %08X | %02X ", (addr + i), *(buf + i));
		}
		else if((i % 0X10) == 15)
		{
			l += sprintf(binstr + l, "%02X | ", *(buf + i));
			for(j = i - 0X0F; j <= i; j++)
			{
				l += sprintf(binstr + l, "%c", ('!' < *(buf + j) && *(buf + j) <= '~') ? *(buf + j) : '.');
			}

			INFO("%s |\n",binstr);
			bzero(binstr, sizeof(binstr));
		}
		else
		{
			l += sprintf(binstr + l, "%02X ", *(buf + i));
		}
	}		

	if(0 != (i % 0X10))
	{		
		j = 0X10 - (i % 0X10);

		for(k = 0; k < j; k++)
		{
			l += sprintf(binstr + l, "   ");
		}

		l += sprintf(binstr + l, "| ");
		k = j;
		
		for(j = i - (0X10 - k); j < i; j++)
		{
			l += sprintf(binstr + l, "%c", ('!' < *(buf + j) && *(buf + j) <= '~') ? *(buf + j) : '.');
		}

		for( i = 0; i < k ; i++)
		{
			l += sprintf(binstr + l, " ");
		}

		INFO("%s |\n",binstr);
	}

	INFO("+----------+-------------------------------------------------+------------------+\n");
}



