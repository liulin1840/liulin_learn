/*****************************************************
@ Copyright (c) 2015-2016 GUN. All rights reserved
@ Auther    : miaomiao
@ Department: R&D
@ E-mail	: 545911533@qq.com
******************************************************/
#ifndef _THREAD_H
#define _THREAD_H "thread.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>


//线程任务结构体
typedef struct task
{
	void*(*task)(void *parms);
	void* parms;
	struct task * next;
}task_st;

//线程池结构
typedef struct
{
	pthread_mutex_t lock;
	pthread_cond_t ready;
	task_st* head;
	int shutdown;
	pthread_t *thread_id;
	int max_thread_num;
	int cur_task_list_size;
}thread_pool_st;

//线程池例行执行模块
void* thread_routine(void* parms);
//初始化线程池
int thread_pool_init(thread_pool_st** pool_addr,int max_thread_num);
//向线程池中添加任务
int pool_add_task(thread_pool_st* pool,void*(*task)(void* parms), void *parms);
//销毁线程池
int pool_destroy(thread_pool_st* pool);

#endif

