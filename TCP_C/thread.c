/*****************************************************
@ Copyright (c) 2015-2016 WAYOS.Co.Ltd. All rights reserved
@ Auther    : miaomiao
@ Department: R&D
@ E-mail	: 545911533@qq.com
******************************************************/
#include "thread.h"
#include "debug.h"
#include "mem.h"
//初始化线程池
int thread_pool_init(thread_pool_st** pool_addr,int max_thread_num)
{

	int ret = -1;
	thread_pool_st* pool = NULL;
	int i = 0;

	pool = (thread_pool_st*)wys_malloc(sizeof(thread_pool_st));
	
	if(!pool)
	{
		DEBUG("malloc() %d failed!\n",sizeof(thread_pool_st));
		goto END;
	}

	*pool_addr = pool;
	pthread_mutex_init(&(pool->lock),NULL);
	pthread_cond_init(&(pool->ready),NULL);
	
	pool->head = NULL;
	pool->shutdown = 0;
	
	pool->thread_id = (pthread_t *)wys_malloc(max_thread_num*sizeof(pthread_t));
	if(!pool->thread_id)
	{
		DEBUG("malloc() %d failed!\n",max_thread_num*sizeof(pthread_t));
		goto END;
	}
	
	pool->max_thread_num = max_thread_num;
	pool->cur_task_list_size = 0;

	//创建线程池
	for(i = 0; i < max_thread_num; i++)
	{
		pthread_create(&(pool->thread_id[i]),NULL,thread_routine,(void*)pool);
	}
	INFO("create thread pool OK.thread count:%d\n",max_thread_num);
	ret = 0;
END:
	return ret;
}

//向线程池中添加任务
int pool_add_task(thread_pool_st* pool,void*(*task)(void* parms), void *parms)
{
	assert(pool != NULL);
	//构造一个新任务
	task_st*  newtask = (task_st*)wys_malloc(sizeof(task_st));
	assert(newtask != NULL);
	newtask->task = task;
	newtask->parms = parms;
	newtask->next = NULL;

	pthread_mutex_lock(&(pool->lock));

	task_st* task_queue = pool->head;
	if(task_queue != NULL)
	{
		while(task_queue->next != NULL)
			task_queue = task_queue->next;
		task_queue->next = newtask;
	}
	else
	{
		pool->head = newtask;
	}
	
	assert(pool->head != NULL);
	
	pool->cur_task_list_size++;

	pthread_mutex_unlock(&(pool->lock));

	//唤醒线程
	pthread_cond_signal(&(pool->ready));

	return 0;
}

int pool_destroy(thread_pool_st* pool)
{
	int i;
	//防止两次调用
	if(pool->shutdown)
	{
		return 0;
	}

	pool->shutdown = 1;
	
	//唤醒所有等待线程
	pthread_cond_broadcast(&(pool->ready));
	
	//回收所有线程
	INFO("destory thread pool, addr:%p ...\n",pool);
	for(i = 0; i < pool->max_thread_num; i++)
	{
		pthread_join(pool->thread_id[i],NULL);
	}
	INFO("clear all thread OK.\n");
	wys_free(pool->thread_id);
	pool->thread_id = NULL;
	
	//销毁任务队列
	task_st *head = NULL;
	while(pool->head != NULL)
	{
		head = pool->head;
		pool->head = pool->head->next;
		wys_free(head);
	}

	head = NULL;
	
	//销毁互斥锁和条件锁
	pthread_mutex_destroy(&(pool->lock));
	pthread_cond_destroy(&(pool->ready));
	
	//释放线程池
	wys_free(pool);
	return 0;
}

void* thread_routine(void* arg)
{
	thread_pool_st* pool = (thread_pool_st*)arg;
	
	assert(pool != NULL);
	
	while(1)
	{
			
		pthread_mutex_lock(&(pool->lock));

		//如果任务队列为空且线程池没有被销毁,则线程睡眠
		while(pool->cur_task_list_size == 0 && !pool->shutdown)
		{
			pthread_cond_wait(&(pool->ready),&(pool->lock));
		}
		
		//线程池销毁
		if(pool->shutdown)
		{
			pthread_mutex_unlock(&(pool->lock));
			pthread_exit(NULL);
		}
		
		assert(pool->cur_task_list_size != 0);
		assert(pool->head != NULL);

		//取任务
		pool->cur_task_list_size--;
		task_st *task = pool->head;
		pool->head = task->next;
	
		pthread_mutex_unlock(&(pool->lock));
	
		//执行
		(*(task->task))(task->parms);
		wys_free(task);
		task = NULL;
	}
	//useless
	pthread_exit(NULL);
}


