/////////////////////////////////////////////////////////////////////
/// @file hash.c
/// @brief hash操作及相关链表定义
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "mem.h"
#include "list.h"
#include "worker.h"
#include "ww_action.h"
#include "public.h"

struct hlist_head an_forbid_hash[HASH_SIZE] ;
struct list_head  an_forbid_list =LIST_HEAD_INIT(an_forbid_list);
struct list_head  an_forbid_group_list[GROUP_ID_MAX];

struct list_head  an_group_list = LIST_HEAD_INIT(an_group_list);	///< 组基本信息链表
struct hlist_head an_group_hash[HASH_SIZE];							///< 组基本信息哈希表
_s32 an_group_num = 0;             									///< 用户组数量 没算GID==0

struct hlist_head encrypt_hash[HASH_SIZE];
struct list_head  encrypt_all_list = LIST_HEAD_INIT(encrypt_all_list);
struct list_head  encrypt_group_list[GROUP_ID_MAX];

struct hlist_head an_sock_hash[HASH_SIZE];						///< sock哈希表,以在an_conn_array中的下标作为哈希值
struct hlist_head an_web_cache_hash[HASH_SIZE];				///< web缓存哈希表,以cookie'token'值作为哈希值

struct hlist_head ww_user_hash[HASH_SIZE];						///< 全部用户的行为统计哈希表(关闭窗口/关闭URL)
struct list_head ww_user_list = LIST_HEAD_INIT(ww_user_list);	///< 全部用户的行为统计链表(关闭窗口/关闭URL)
struct list_head ww_user_group_list[GROUP_ID_MAX];				///< 所有用户组的行为统计链表(关闭窗口/关闭URL)

//临时用，获取数据不用它
struct list_head auto_save_user_group_list = LIST_HEAD_INIT(auto_save_user_group_list); ///< 用户组的行为统计链表,每天保存时临时生成一个
_u32 auto_save_user_group_user_cnt[GROUP_ID_MAX] = {0};									///< 用户组的行为统计人数,每天保存时临时生成一个

struct hlist_head an_detail_user_hash[HASH_SIZE];							///< 全部用户行为记录哈希表
struct list_head an_detail_user_list = LIST_HEAD_INIT(an_detail_user_list);	///< 全部用户行为记录链表
struct list_head an_detail_user_group_list[GROUP_ID_MAX];					///< 用户组行为记录链表

struct list_head an_qq_user_list = LIST_HEAD_INIT(an_qq_user_list);			///< 全部用户聊天记录链表
struct list_head an_qq_user_group_list[GROUP_ID_MAX];						///< 用户组聊天记录链表
struct hlist_head an_qq_user_hash[HASH_SIZE];								///< 全部用户聊天记录哈希表

struct list_head an_work_rule_list = LIST_HEAD_INIT(an_work_rule_list);		///< 全部办公行为链表
struct list_head an_work_group_list[GROUP_ID_MAX];							///< 用户组办公行为链表
struct hlist_head an_work_rule_hash[HASH_SIZE];								///< 全部办公行为哈希表
_s32 an_work_rule_num = 0;													///< 办公行为规则数量

/**************排序start******************/

//----------------------------------------------------------------
// 函数名称 list_commen_compare_fun
/// @brief list快排用公共比较函数
/// 
/// @param _in a->第一个比较节点地址
/// @param _in b->第二个比较节点地址
/// @param _in compare_type->比较类型
/// @return a>b 正数,a<b 负数,a=b 0
/// @author miaomiao
//---------------------------------------------------------------
_s32 list_commen_compare_fun(struct list_head* a, struct list_head* b, _s32 compare_type)
{
	_s32 return_val = 0;

	switch(compare_type)
	{
	case LIST_QQ_WK_MSG_SORT:
		list_commen_compare(a, b, an_qq_wk_msg_t, time, msg_list, &return_val);
		break;

	case LIST_DETAIL_WK_ACT_SORT:
		list_commen_compare(a, b, an_detail_wk_act_t, time, list, &return_val);
		break;

	case LIST_GROUP_BASE_INFO_SORT:
		list_commen_compare(a, b, ww_group_base_info_t, gid, list, &return_val);
		break;
	}

	return return_val;
}

//----------------------------------------------------------------
// 函数名称 list_swap
/// @brief 交换链表节点(节点必须初始化)
/// 
/// @param _in a->第一个节点地址
/// @param _in b->第二个节点地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void inline list_swap(struct list_head *a,  struct list_head *b)
{
	struct  list_head* pos = NULL;

	if(a != b)
	{
		if(b != b->next)
		{
			pos = b->next;
		}
		
		if(a == b->next)
		{
			list_del_init(a);
			list_move_tail(a,b);
			return;
		}
		
		if(a == b->prev)
		{
			list_del_init(a);
			list_move_tail(a,b->next);
			return;
		}

		list_del_init(b);
		list_move_tail(b,a->next);

		if(pos != NULL)
		{
			list_del_init(a);
			list_move_tail(a,pos);
		}
		else
		{
			list_del_init(a);
		}
	}
}

//----------------------------------------------------------------
// 函数名称 list_get_partion
/// @brief 链表快排具体实现
/// 
/// @param _in begin->开始节点地址
/// @param _in end->结束节点地址
/// @param _in compare_fun_type->比较函数类型(不用就传0)
/// @param _in compare_fun->比较函数(自定义比较函数) @see list_commen_compare_fun hash.c
/// @return 返回中间值的节点地址
/// @author miaomiao
//---------------------------------------------------------------
struct list_head* list_get_partion(struct list_head* head, struct list_head** begin, struct list_head** end, _s32 compare_fun_type
					, _s32 (*compare_fun)(struct list_head* a, struct list_head* b, _s32 compare_fun_type))
{
	struct list_head* p = *begin;
	struct list_head* q = p->next;
	struct list_head* partion_temp = NULL;

	while(q->prev != *end && q != head)
	{
		if((*compare_fun)(q,*begin,compare_fun_type) < 0)
		{
			p = p->next;
			if(p != q)
			{
        		struct list_head* q_next = q->next;
				struct list_head* p_prev = p->prev;
				
				list_swap(p,q);
				
				p = p_prev->next;
				q = q_next;
				
				continue;
			}
		}
		q = q->next;
	}
	
	if(p != *begin)
	{
    	list_swap(p, *begin);
	}

	partion_temp = *begin;
	*end = q->prev;
	*begin = p;
	p = partion_temp;
	
	return p;
}

//----------------------------------------------------------------
// 函数名称 list_quick_sort
/// @brief 链表快速排序函数
/// 
/// @param _in begin->开始节点地址
/// @param _in end->结束节点地址
/// @param _in compare_fun_type->比较函数类型(不用就传0)
/// @param _in compare_fun->比较函数(自定义比较函数) @see list_commen_compare_fun hash.c
/// @return 返回中间值的节点地址
/// @author miaomiao
//---------------------------------------------------------------
void list_quick_sort(struct list_head* head, struct list_head* begin, struct list_head* end, _s32 compare_fun_type
					, _s32 (*compare_fun)(struct list_head* a, struct list_head* b,_s32 compare_fun_type))
{
    if(begin != end)
    {
		struct list_head* partion = list_get_partion(head,&begin,&end,compare_fun_type,compare_fun);
		if(begin != partion)
		{
			list_quick_sort(head,begin,partion,compare_fun_type,(void*)compare_fun);
		}
		if(partion->next != end && partion->next != head)
		{
			list_quick_sort(head,partion->next,end,compare_fun_type,(void*)compare_fun);
		}
	}
}

/**************排序end******************/

_s32 forbid_act_hash_value(_s8* name)
{
	_s32 hash_seed = 131;
	_s32 hash_r = 0;
	while (*name++)
	{
		hash_r = hash_r*hash_seed + *name;
	}

	return hash_r & HASH_MASK;

}

ww_forbidact_t* forbid_act_find_by_wnd_name(_s8*name, _s32 gid)
{
	struct list_head* pos = NULL;
	ww_forbidact_t* forbid_t = NULL;
	list_for_each(pos, &an_forbid_group_list[gid])
	{
		forbid_t = list_entry(pos, ww_forbidact_t, group_list);
		if (strstr(name, forbid_t->name))
		{
			return forbid_t;  //成功找到
		}
	}
	if(gid != 0)
	{
		list_for_each(pos, &an_forbid_group_list[0])
		{
			forbid_t = list_entry(pos, ww_forbidact_t, group_list);
			if (strstr(name, forbid_t->name))
			{
				return forbid_t;  //成功找到
			}
		}
	}
	return NULL; //没有找到
}

ww_forbidact_t* forbid_act_find_by_process(_s8*name, _s32 gid)
{
	struct list_head* pos = NULL;
	ww_forbidact_t* forbid_t = NULL;
	list_for_each(pos, &an_forbid_group_list[gid])
	{
		forbid_t = list_entry(pos, ww_forbidact_t, group_list);
		if (!strcasecmp(name, forbid_t->name))
		{
			return forbid_t;  //成功找到
		}
	}
	if(gid != 0)
	{
		list_for_each(pos, &an_forbid_group_list[0])
		{
			forbid_t = list_entry(pos, ww_forbidact_t, group_list);
			if (!strcasecmp(name, forbid_t->name))
			{
				return forbid_t;  //成功找到
			}
		}
	}
	return NULL; //没有找到
}

ww_forbidact_t* forbid_act_hash_find(_s8*name, _s32 gid)
{
	_s32 hash_num = forbid_act_hash_value(name);
	struct hlist_node* hash_cmp = NULL;
	ww_forbidact_t* forbid_t = NULL;
	hlist_for_each(hash_cmp, &an_forbid_hash[hash_num])
	{
		forbid_t = hlist_entry(hash_cmp, ww_forbidact_t, h_nd);
		if (!strcmp(forbid_t->name, name))
		{
			if (forbid_t->gid == 0 || forbid_t->gid == gid)            //在HASH表中  单一用户组的规则里 不存适用于全部用户组的
			{
				return forbid_t;  //成功找到
			}
		}
	}
	return NULL; //没有找到
}

_s32 forbid_act_hash_add(_s8* name,_s32 size,_s32 gid,_u8 act_type,_u8 is_forbid, _u8 flag)
{
	_s32 hash_num = forbid_act_hash_value(name);
	struct hlist_node* hash_cmp = NULL;
	ww_forbidact_t* forbid_t = NULL;
	hlist_for_each(hash_cmp,&an_forbid_hash[hash_num])
	{
		forbid_t = hlist_entry(hash_cmp, ww_forbidact_t, h_nd);
		if (!strcmp(forbid_t->name, name))
		{
			if (gid != 0)
			{
				if (forbid_t->gid == 0 || forbid_t->gid == gid)            //已存在于全部用户组或相同用户组的重名规则 添加失败
				{                                                        //所以不用判断 act_type
					if (flag == 1)
						return -1; //已存在，添加失败
					if (flag == 0)            //用来判断是初始化时调用还是页面添加时调用 初始化时全部和单一会重复
						return 0;
				}
			}
			else if (gid == 0)
			{
				if (forbid_t->gid == gid)                               //如果添加的全部用户组规则与单一用户组规则重复    删除单一用户组规则
				{
					return -1;
				}
				else if (forbid_t->gid != 0)
				{
					forbid_act_hash_del(forbid_t->name, forbid_t->gid);
				
				}
			}
		}
	}
	
	struct hlist_node* hash_ins = (struct hlist_node*)wys_malloc(sizeof(ww_forbidact_t));
	if (!hash_ins)
	{
		return -1; //添加失败
	}
	forbid_t =(ww_forbidact_t*) hash_ins;
	hlist_add_head(hash_ins, &an_forbid_hash[hash_num]);                                                   
	list_add_tail(&(forbid_t->all_list), &an_forbid_list);                            
	list_add_tail(&(forbid_t->group_list), &an_forbid_group_list[gid]);                   // //在HASH表中  单一用户组的规则里 不存适用于全部用户组的 所以不用判断GID是否为0
	if (act_type == 0 || act_type == 3)   //域名关键字/搜索关键字
	{
		if (act_type == 0)
			forbid_t->kind = 1;
		if (act_type == 3)
			forbid_t->kind = 2;
		forbid_t->type = is_forbid + 1;
		forbid_t->gid = gid;
		forbid_t->is_forbid = is_forbid;
		forbid_t->act_type = act_type;
		memcpy(forbid_t->name,name, sizeof(forbid_t->name));
	}
	if (act_type == 2)                    //进程运行
	{
		forbid_t->enable = 1;
		forbid_t->option = is_forbid?3:0;
		forbid_t->act_type = act_type;
		forbid_t->is_forbid = is_forbid;
		forbid_t->gid = gid;
		memcpy(forbid_t->name,name,sizeof(forbid_t->name));
	}
	if (act_type == 1)                       //窗口
	{
		forbid_t->kind = 2;
		forbid_t->type = is_forbid + 1;
		forbid_t->gid = gid;
		forbid_t->is_forbid = is_forbid;
		forbid_t->act_type = act_type;
		memcpy(forbid_t->name,name, sizeof(forbid_t->name));
	}

	return 0;  //添加成功
}

//删除所有规则
void forbid_act_hash_del_all()
{
	struct list_head* list_delete = an_forbid_list.next;
	ww_forbidact_t* forbid_del = NULL;
	while (list_delete!=&an_forbid_list)
	{
		forbid_del = list_entry(list_delete, ww_forbidact_t, all_list);
		list_delete = list_delete->next;
		list_del(&(forbid_del->all_list));
		list_del(&(forbid_del->group_list));
		hlist_del(&(forbid_del->h_nd));
		wys_free(forbid_del);
	}

}

//删除某组的所有规则
void forbid_act_hash_del_group(_s32 gid)
{
	ww_forbidact_t* forbid_del = NULL;
	struct list_head* list_delete = an_forbid_group_list[gid].next;

	while (list_delete!=&an_forbid_group_list[gid])
	{
		forbid_del = list_entry(list_delete, ww_forbidact_t, group_list);
		list_delete = list_delete->next;
		list_del(&(forbid_del->all_list));
		list_del(&(forbid_del->group_list));
		hlist_del(&(forbid_del->h_nd));
		wys_free(forbid_del);
	}
}

//删除单条规则
_s32 forbid_act_hash_del(_s8*name,_s32 gid)
{
	_s32 hash_num = forbid_act_hash_value(name);
	ww_forbidact_t*forbid_del = NULL;
	struct hlist_node* hash_dl = NULL;
	hlist_for_each(hash_dl,&an_forbid_hash[hash_num])
	{
		forbid_del = hlist_entry(hash_dl, ww_forbidact_t, h_nd);
		if (forbid_del->gid == gid)
		{
			if (!strcmp(forbid_del->name, name))
			{
				hlist_del((&forbid_del->h_nd));
				list_del(&(forbid_del->all_list));
				list_del(&(forbid_del->group_list));
				wys_free(forbid_del);
				return 0;//删除成功
			}
		}
	}
	
	return -1;//删除失败
}

//----------------------------------------------------------------
// 函数名称 ww_group_base_info_st_get_hash
/// @brief 根据gid获取hash值(针对an_group_hash)
/// 
/// @param _in gid->用户组ID
/// @return gid的hash值
/// @author miaomiao
//---------------------------------------------------------------
_u32 ww_group_base_info_st_get_hash(_s32 gid)
{
	union {
		_u32 l;
		_u8 c[4];
	} id;
	_u32 index = 0;
	id.l = gid;

	index = id.c[0]+id.c[1]+id.c[2]+id.c[3];

    return index & HASH_MASK;
}

//----------------------------------------------------------------
// 函数名称 ww_group_base_info_st_create
/// @brief 创建ww_group_base_info_t并初始化
/// 
/// @param void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
ww_group_base_info_t* ww_group_base_info_st_create(void)
{
	ww_group_base_info_t *temp = NULL;
	
	temp = (ww_group_base_info_t *)wys_malloc(sizeof(*temp));
	if(temp)
	{
		INIT_LIST_HEAD(&temp->list);
		INIT_HLIST_NODE(&temp->hash);
	}
	
	return temp;
}

//----------------------------------------------------------------
// 函数名称 ww_group_base_info_st_find_by_id
/// @brief 根据gid,在an_group_hash中查找ww_group_base_info_t是否存在
/// 
/// @param _in gid->用户组ID
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
ww_group_base_info_t * ww_group_base_info_st_find_by_id(_s32 gid)
{
	struct hlist_head *head = NULL;
	struct hlist_node *pos = NULL;
	ww_group_base_info_t *temp = NULL;
	_u32 index = 0;

	if(gid > GROUP_ID_MAX)
		return NULL;

	index = ww_group_base_info_st_get_hash(gid);
	head = &an_group_hash[index];

	hlist_for_each(pos, head)
	{
		temp = hlist_entry(pos, ww_group_base_info_t, hash);

		if(temp->gid == gid)
			return temp;
	}

	return NULL;
}

//----------------------------------------------------------------
// 函数名称 ww_group_base_info_st_del
/// @brief 删除ww_group_base_info_t
/// 
/// @param _in gid->用户组ID
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
void ww_group_base_info_st_del(_s32 gid)
{
	ww_group_base_info_t *temp = NULL;
	temp = ww_group_base_info_st_find_by_id(gid);

	if(temp)
	{
		hlist_del(&temp->hash);
		list_del(&temp->list);
		wys_free(temp);
		an_group_num--;
	}
}

//----------------------------------------------------------------
// 函数名称 ww_group_base_info_st_del_all
/// @brief 清空an_group_list
/// 
/// @param void->无参
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void ww_group_base_info_st_del_all(void)
{
	struct list_head *pos, *safe = NULL;
	ww_group_base_info_t *temp = NULL;

	list_for_each_safe(pos, safe, &an_group_list)
	{
		temp = list_entry(pos, ww_group_base_info_t, list);

		if(temp)
		{
			hlist_del(&temp->hash);
			list_del(&temp->list);
			wys_free(temp);
		}
	}
}

//----------------------------------------------------------------
// 函数名称 an_socket_t_get_hash
/// @brief 通过index获取哈希值(针对an_sock_hash)
/// 
/// @param _in index->an_conn_array中的下标
/// @return index的hash值
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_socket_t_get_hash(_s32 index)
{
	union {
		_u32 l;
		_u8 c[4];
	} id;
	_u32 i = 0;
	id.l = index;

	i = id.c[0]+id.c[1]+id.c[2]+id.c[3];

    return i & HASH_MASK;
}

//----------------------------------------------------------------
// 函数名称 an_socket_t_create
/// @brief 创建an_socket_t并初始化
/// 
/// @param _in void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
an_socket_t* an_socket_t_create(void)
{
	an_socket_t* temp = NULL;
	temp = (an_socket_t*)wys_malloc(sizeof(an_socket_t));
	if(temp)
	{
		INIT_HLIST_NODE(&temp->hash);
	}

	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_socket_t_find_by_index
/// @brief 根据index,在an_sock_hash查找an_socket_t是否存在
/// 
/// @param _in index->an_conn_array中的下标
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
an_socket_t* an_socket_t_find_by_index(_s32 index)
{
	struct hlist_head *head = NULL;
	struct hlist_node *pos = NULL;
	an_socket_t *temp = NULL;
	_u32 i = 0;

	if(index >= AN_POLL_MAX)
		return NULL;

	i = an_socket_t_get_hash(index);
	head = &an_sock_hash[i];

	hlist_for_each(pos, head)
	{
		temp = hlist_entry(pos, an_socket_t, hash);

		if(temp->index == index)
			return temp;
	}

	return NULL;
}

//----------------------------------------------------------------
// 函数名称 an_socket_t_del
/// @brief 删除an_socket_t
/// 
/// @param _in index->an_conn_array中的下标
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_socket_t_del(_s32 index)
{
	an_socket_t *temp = NULL;
	temp = an_socket_t_find_by_index(index);

	if(temp)
	{
		hlist_del(&temp->hash);
		if(temp->rcv_buf)
			wys_free(temp->rcv_buf);
		if(temp->send_buf)
			wys_free(temp->send_buf);
		if(temp->org_send_buf)
			wys_free(temp->org_send_buf);
		wys_free(temp);
	}
}

//----------------------------------------------------------------
// 函数名称 an_web_cache_t_get_hash
/// @brief 通过token获取哈希值(针对an_web_cache_hash)
/// 
/// @param _in token->网页cookie值
/// @return token的hash值
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_web_cache_t_get_hash(_s32 token)
{
	union {
		_u32 l;
		_u8 c[4];
	} id;
	_u32 i = 0;
	id.l = token;

	i = id.c[0]+id.c[1]+id.c[2]+id.c[3];

    return i & HASH_MASK;
}

//----------------------------------------------------------------
// 函数名称 an_web_cache_t_timeout
/// @brief 删除an_web_cache_t
/// 
/// @param _in data->an_web_cache_t自身地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_web_cache_t_timeout(unsigned long data)
{
	an_web_cache_t* web_cache = (an_web_cache_t*)data;

	an_detail_wk_act_t_del_list(&web_cache->detail_act_list);
	an_qq_wk_msg_t_del_list(&web_cache->qq_msg_list);
	hlist_del_init(&web_cache->hash);
	del_timer(&web_cache->timeout);
	
	wys_free(web_cache);
	
	return;
}

//----------------------------------------------------------------
// 函数名称 an_web_cache_t_create
/// @brief 创建an_web_cache_t并初始化
/// 
/// @param _in void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
an_web_cache_t* an_web_cache_t_create(_s32 token)
{
	an_web_cache_t* temp = NULL;
	_s32 index = 0;
	temp = (an_web_cache_t*)wys_malloc(sizeof(an_web_cache_t));
	if(temp)
	{
		temp->token = token;
	
		INIT_HLIST_NODE(&temp->hash);
		INIT_LIST_HEAD(&temp->detail_act_list);
		INIT_LIST_HEAD(&temp->qq_msg_list);
		
		init_timer(&temp->timeout);
		temp->timeout.data = (unsigned long)temp;
		temp->timeout.function = an_web_cache_t_timeout;
		temp->timeout.expires = jiffies + COOKIE_AGEING_TIME * HZ;
		add_timer(&temp->timeout);

		index = an_web_cache_t_get_hash(token);
		hlist_add_head(&temp->hash, &an_web_cache_hash[index]);
	}

	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_web_cache_t_find_by_token
/// @brief 根据token,在an_web_cache_hash查找an_web_cache_t是否存在
/// 
/// @param _in token->网页cookie值
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
an_web_cache_t* an_web_cache_t_find_by_token(_s32 token)
{
	struct hlist_head *head = NULL;
	struct hlist_node *pos = NULL;
	an_web_cache_t *temp = NULL;
	_u32 i = 0;

	i = an_web_cache_t_get_hash(token);
	head = &an_web_cache_hash[i];

	hlist_for_each(pos, head)
	{
		temp = hlist_entry(pos, an_web_cache_t, hash);

		if(temp->token == token)
			return temp;
	}

	return NULL;
}

//----------------------------------------------------------------
// 函数名称 an_web_cache_t_find_create_by_token
/// @brief 根据token,在an_web_cache_hash查找或创建an_web_cache_t
/// 
/// @param _in token->网页cookie值
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
an_web_cache_t* an_web_cache_t_find_create_by_token(_s32 token)
{
	an_web_cache_t* temp = an_web_cache_t_find_by_token(token);
	if(NULL == temp)
	{
		temp = an_web_cache_t_create(token);
	}

	return temp;
}

_s32 encrypt_file_hash_value(_s8* name)
{
	_s32 hash_seed = 131;
	_s32 hash_r = 0;
	while (*name++)
	{
		hash_r = hash_r*hash_seed + *name;
	}

	return hash_r & HASH_MASK;

}
_s32 encrypt_file_hash_add(_s8* name, _s32 name_len, _s32 gid, _u8 filetype, _u32 authority, _u16 enable, _u8 flag)
{
	_s32 hash_num = encrypt_file_hash_value(name);
	ww_encryptfile_t* encrypt_add = NULL;
	ww_encryptfile_t* encrypt_cmp = NULL;
	struct hlist_node* pos = NULL;
	hlist_for_each(pos, &encrypt_hash[hash_num])
	{
		encrypt_cmp = hlist_entry(pos, ww_encryptfile_t, h_nd);
		if (!strcmp(name, encrypt_cmp->name))
		{
			if (encrypt_cmp->gid == 0 || encrypt_cmp->gid == gid)
			{
				if (flag==1)
				return -1; //添加失败
				if (flag == 0)
				return 0;
			}
		}
	}

	encrypt_add = (ww_encryptfile_t*)wys_malloc(sizeof(ww_encryptfile_t));
	if (!encrypt_add)
	{
		return -1; //添加失败
	}
	encrypt_add->gid = gid;
	encrypt_add->authority = authority;
	encrypt_add->enable = 1;
	//memcpy(encrypt_add->name, name, name_len);
	snprintf(encrypt_add->name, name_len, "%s", name);
	encrypt_add->filetype = filetype;
	hlist_add_head(&(encrypt_add->h_nd), &encrypt_hash[hash_num]);
	list_add_tail(&(encrypt_add->all_list), &encrypt_all_list);
	list_add_tail(&(encrypt_add->group_list), &encrypt_group_list[gid]);

	return 0; //添加成功
}

_s32 encrypt_file_hash_del(_s8*name, _s32 gid)
{
	ww_encryptfile_t* encrypt_del = NULL;
	_s32 hash_num = encrypt_file_hash_value(name);
	struct hlist_node* pos = NULL;
	hlist_for_each(pos, &encrypt_hash[hash_num])
	{
		encrypt_del = hlist_entry(pos, ww_encryptfile_t, h_nd);
		if (!strcmp(encrypt_del->name, name))
		{
			if (encrypt_del->gid == gid)
			{
				hlist_del(&(encrypt_del->h_nd));
				list_del(&(encrypt_del->group_list));
				list_del(&(encrypt_del->all_list));
				wys_free(encrypt_del);
				return 0; //删除成功
			}
		}
	}
	return -1;//删除失败
}

void encrypt_file_hash_del_all(void)
{
	ww_encryptfile_t* encrypt_del = NULL;
	struct list_head* pos = encrypt_all_list.next;
	while (pos != &encrypt_all_list)
	{
		encrypt_del = list_entry(pos, ww_encryptfile_t, all_list);
		pos = pos->next;
		hlist_del(&(encrypt_del->h_nd));
		list_del(&(encrypt_del->group_list));
		list_del(&(encrypt_del->all_list));
		wys_free(encrypt_del);
	}
}

void encrypt_file_hash_del_group(_s32 gid)
{
	ww_encryptfile_t* encrypt_del = NULL;
	struct list_head* pos = encrypt_group_list[gid].next;
	while (pos != &encrypt_group_list[gid])
	{
		encrypt_del = list_entry(pos, ww_encryptfile_t, group_list);
		pos = pos->next;
		hlist_del(&(encrypt_del->h_nd));
		list_del(&(encrypt_del->group_list));
		list_del(&(encrypt_del->all_list));
		wys_free(encrypt_del);
	}
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_st_get_hash
/// @brief 通过user_id获取哈希值(针对ww_user_hash)
/// 
/// @param _in user_id->用户ID
/// @return user_id的hash值
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_user_action_st_get_hash(_s32 user_id)
{
	union {
		_u32 l;
		_u8 c[4];
	} id;
	_u32 i = 0;
	id.l = user_id;

	i = id.c[0]+id.c[1]+id.c[2]+id.c[3];

    return i & HASH_MASK;
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_st_get_hash
/// @brief 创建user_action_st并初始化
/// 
/// @param void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
struct ww_user_action_st* ww_user_action_st_create(void)
{
	int i;
	struct ww_user_action_st *temp = NULL;
	
	temp = (struct ww_user_action_st *)wys_malloc(sizeof(*temp));
	if(temp)
	{
		INIT_LIST_HEAD(&temp->user_list);
		INIT_LIST_HEAD(&temp->action_list);
		INIT_LIST_HEAD(&temp->group_list);
		INIT_HLIST_NODE(&temp->user_hash);
		for(i = 0; i < HASH_SIZE; i++)
		{
			INIT_HLIST_HEAD(&temp->action_hash_table[i]);
		}
	}
	
	return temp;
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_find_by_id
/// @brief 根据user_id,在ww_user_hash查找user_action_st是否存在
/// 
/// @param _in user_id->用户ID
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
struct ww_user_action_st * ww_user_action_find_by_id(_u32 user_id)
{
	_u32 index = 0;
	struct hlist_node *pos = NULL;
	struct hlist_head *head = NULL;
	struct ww_user_action_st * temp = NULL;

	if(user_id == 0 || user_id == 0xffffffff)
		return 0;

	index = ww_user_action_st_get_hash(user_id);
	head = &ww_user_hash[index];

	hlist_for_each(pos, head)
	{
		temp = hlist_entry(pos,struct ww_user_action_st,user_hash);

		if(temp->user_id == user_id)
			return temp;
	}
	
	return NULL;
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_id_find_create
/// @brief 根据user_id查找或创建user_action_st
/// 
/// @param _in user_id->用户ID
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
struct ww_user_action_st* ww_user_action_id_find_create(_u32 user_id)
{
	struct ww_user_action_st *temp = NULL;

	if(user_id == 0 || user_id == 0xffffffff)
		return NULL;

	temp = ww_user_action_find_by_id(user_id);
	if(temp)
		return temp;
	
	temp = ww_user_action_st_create();

	return temp;
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_find_by_gid
/// @brief 根据group_id,在auto_save_user_group_list查找user_action_st是否存在
/// 
/// @param _in group_id->用户所属组ID
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
struct ww_user_action_st * ww_user_action_find_by_gid(_u32 group_id)
{
	struct list_head *pos;
	
	struct ww_user_action_st * temp = NULL;

	list_for_each(pos, &auto_save_user_group_list)
	{
		temp = list_entry(pos,struct ww_user_action_st,group_list);

		if(temp->group_id == group_id)
			return temp;
	}
	
	return NULL;
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_gid_find_create
/// @brief 根据gid查找或创建user_action_st
/// 
/// @param _in group_id->用户所属组ID
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
struct ww_user_action_st* ww_user_action_gid_find_create(_u32 group_id)
{
	struct ww_user_action_st *temp = NULL;

	temp = ww_user_action_find_by_gid(group_id);
	if(temp)
		return temp;
	
	temp = ww_user_action_st_create();

	return temp;
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_list_del
/// @brief 删除某个ww_action_list_st规则链表
/// 
/// @param _in head->规则链表表头
/// @see ww_action_list_st worker.h
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
void ww_user_action_list_del(struct list_head * head)
{
	struct list_head * pos = NULL;
	struct ww_action_list_st * action = NULL;

	pos = head->next;
	while(pos != head)
	{
		action = list_entry(pos, struct ww_action_list_st, action_list);
		pos = pos->next;
		hlist_del(&action->name_hash);
		list_del(&action->action_list);
		wys_free(action);
		action = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_st_del
/// @brief 删除单个ww_user_action_st节点
/// 
/// @param _in user->节点地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void ww_user_action_st_del(struct ww_user_action_st * user)
{
	struct list_head * pos = NULL;
	struct ww_action_list_st * action = NULL;
	pos = user->action_list.next;
	while(pos != &user->action_list)
	{
		action = list_entry(pos,struct ww_action_list_st,action_list);
		pos = pos->next;
		hlist_del_init(&action->name_hash);
		list_del_init(&action->action_list);
		wys_free(action);
		action = NULL;
	}
	hlist_del_init(&user->user_hash);
	
	list_del_init(&user->user_list);
	list_del_init(&user->action_list);
	list_del_init(&user->group_list);
	wys_free(user);
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_st_del_group
/// @brief 清理某个用户组下所有用户(针对ww_user_group_list,基本没用)
/// 
/// @param _in gid->要清理的组ID
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void ww_user_action_st_del_group(_s32 gid)
{
	struct list_head * pos = NULL;
	struct ww_user_action_st * user = NULL;

	pos = ww_user_group_list[gid].next;
	while(pos != &ww_user_group_list[gid])
	{
		user = list_entry(pos,struct ww_user_action_st,group_list);
		pos = pos->next;
		ww_user_action_st_del(user);
		user = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_st_clear
/// @brief 清空所有用户统计(针对ww_user_list)
/// 
/// @param void->无参
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void ww_user_action_st_clear(void)
{
	struct list_head * pos = NULL;
	struct ww_user_action_st * user = NULL;

	pos = ww_user_list.next;
	while(pos != &ww_user_list)
	{
		user = list_entry(pos,struct ww_user_action_st,user_list);
		pos = pos->next;
		ww_user_action_st_del(user);
		user = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 ww_user_action_st_group_clear
/// @brief 清空临时auto_save_user_group_list
/// 
/// @param void->无参
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void ww_user_action_st_group_clear(void)
{
	struct list_head * pos = NULL;
	struct ww_user_action_st * group = NULL;

	pos = auto_save_user_group_list.next;
	while(pos != &auto_save_user_group_list)
	{
		group = list_entry(pos,struct ww_user_action_st,group_list);
		pos = pos->next;
		ww_user_action_st_del(group);
		group = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 ww_action_name_hash
/// @brief 根据name获取hash值(针对ww_action_list_st)
/// 
/// @param name->行为描述(目前为进程名 或 域名)
/// @return name的hash值
/// @author miaomiao
//---------------------------------------------------------------
_u32 ww_action_name_hash(_s8* name)
{
	//只加前8位
	_s32 i = 0;
	_u32 index = 0;
	for(i = 0; i < 8; i++)
	{
		//加到'\0'为止
		if(name[i])
			index += name[i];
		else
			break;
	}
	
    return index & HASH_MASK;
}

//----------------------------------------------------------------
// 函数名称 ww_action_st_create
/// @brief 创建ww_action_list节点并初始化
/// 
/// @param void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
struct ww_action_list_st* ww_action_st_create(void)
{
	struct ww_action_list_st *temp = NULL;
	
	temp = (struct ww_action_list_st *)wys_malloc(sizeof(*temp));
	if(temp)
	{
		INIT_LIST_HEAD(&temp->action_list);
		INIT_HLIST_NODE(&temp->name_hash);
	}
	
	return temp;
}

//----------------------------------------------------------------
// 函数名称 ww_action_st_find_by_hash_name_type
/// @brief 在指定hash表中查找ww_action_list_st
/// 
/// @param _in hash_list->hash表头
/// @param _in name->行为描述(目前为进程名 或 域名)
/// @param _in type->行为类型USER_ACTION_TYPE
/// @see USER_ACTION_TYPE ww_action.h
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
struct ww_action_list_st * ww_action_st_find_by_hash_name_type(struct hlist_head * hash_list, _s8* name, _u32 type)
{
	struct hlist_head * head = NULL;
	struct hlist_node * pos = NULL;
	_u32 index = 0;
	
	struct ww_action_list_st * temp = NULL;

	index = ww_action_name_hash(name);
	head = hash_list+index;

	pos = head->first;
	if(pos == NULL)
		return 0;

	hlist_for_each(pos,head)
	{
		temp = hlist_entry(pos, struct ww_action_list_st, name_hash);
		if(type == OT_BROWSER_CLOSE_URL)
		{
			if(!strcmp(temp->act_name,name))
			{
				return temp;
			}
		}
		else if(type == OT_WINDOW_CLOSE)
		{
			if(!strcmp(temp->pro_name,name))
			{
				return temp;
			}
		}
	}
	
	return NULL;
}

//----------------------------------------------------------------
// 函数名称 ww_action_st_hash_name_type_find_create
/// @brief 在指定hash表中查找并创建ww_action_list_st
/// 
/// @param _in hash_list->hash表头
/// @param _in pro_name->进程名
/// @param _in act_name->行为描述(域名)
/// @param _in type->行为类型USER_ACTION_TYPE
/// @see USER_ACTION_TYPE ww_action.h
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
struct ww_action_list_st * 
ww_action_st_hash_name_type_find_create(struct hlist_head * hash_list
					, _s8* pro_name, _s8* act_name, _u32 type)
{
	struct ww_action_list_st * temp = NULL;

	if(type != OT_WINDOW_CLOSE && type != OT_BROWSER_CLOSE_URL)
		return NULL;

	if(type == OT_WINDOW_CLOSE)
		temp = ww_action_st_find_by_hash_name_type(hash_list, pro_name, type);
	else if(type == OT_BROWSER_CLOSE_URL)
		temp = ww_action_st_find_by_hash_name_type(hash_list, act_name, type);

	if(temp)
		return temp;

	temp = ww_action_st_create();
	
	return temp;
}

//----------------------------------------------------------------
// 函数名称 ww_action_st_find_by_user_name_type
/// @brief 根据ww_user_action_st,action_name查找该action是否存在。
/// 
/// @param _in user->行为统计单个用户节点,主要利用其作为hash表头的成员
/// @param _in name->行为描述(目前为进程名 或 域名)
/// @param _in type->行为类型 USER_ACTION_TYPE
/// @param _in category->日志类型 LOG_CATEGORY
/// @note 此处category仅做是否违规行为区别,待后续需求再细分
/// @see USER_ACTION_TYPE ww_action.h
/// @see LOG_CATEGORY ww_action.h
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
struct ww_action_list_st * ww_action_st_find_by_user_name_type(struct ww_user_action_st * user
															, _s8* name, _u32 type, _u32 category)
{
	struct hlist_head * head = NULL;
	struct hlist_node * pos = NULL;
	_u32 index = 0;
	
	struct ww_action_list_st * temp = NULL;

	if(user == NULL)
		return 0;

	index = ww_action_name_hash(name);
	head = &(user->action_hash_table[index]);

	pos = head->first;
	if(pos == NULL)
		return 0;

	hlist_for_each(pos,head)
	{
		temp = hlist_entry(pos, struct ww_action_list_st, name_hash);
		if(type == OT_BROWSER_CLOSE_URL)
		{
			//先匹配name，再匹配类型,相等或者都非0则使用同节点
			if(!strcmp(temp->act_name,name) && ((temp->category == category) || (temp->category && category)))
			{
				return temp;
			}
		}
		else if(type == OT_WINDOW_CLOSE)
		{
			if(!strcmp(temp->pro_name,name) && ((temp->category == category) || (temp->category && category)))
			{
				return temp;
			}
		}
	}
	
	return NULL;
}

//----------------------------------------------------------------
// 函数名称 ww_action_st_user_name_type_find_create
/// @brief 查找或创建ww_action_list_st
/// 
/// @param _in user->行为统计单个用户节点,主要利用其作为hash表头的成员
/// @param _in pro_name->进程名
/// @param _in act_name->行为描述(域名)
/// @param _in type->行为类型 USER_ACTION_TYPE
/// @param _in category->日志类型 LOG_CATEGORY
/// @see USER_ACTION_TYPE ww_action.h
/// @see LOG_CATEGORY ww_action.h
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
struct ww_action_list_st * 
ww_action_st_user_name_type_find_create(struct ww_user_action_st * user
					, _s8* pro_name, _s8* act_name, _u32 type, _u32 category)
{
	struct ww_action_list_st * temp = NULL;

	if(!user)
		return NULL;

	if(type != OT_WINDOW_CLOSE && type != OT_BROWSER_CLOSE_URL)
		return NULL;

	if(type == OT_WINDOW_CLOSE)
		temp = ww_action_st_find_by_user_name_type(user, pro_name, type, category);
	else if(type == OT_BROWSER_CLOSE_URL)
		temp = ww_action_st_find_by_user_name_type(user, act_name, type, category);

	if(temp)
		return temp;

	temp = ww_action_st_create();
	
	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_detail_user_t_get_hash
/// @brief 通过uid获取哈希值(针对an_detail_user_hash)
/// 
/// @param _in uid->用户ID
/// @return uid的hash值
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_detail_user_t_get_hash(_u32 uid)
{
	union {
		_u32 l;
		_u8 c[4];
	} id;
	_u32 i = 0;
	id.l = uid;

	i = id.c[0]+id.c[1]+id.c[2]+id.c[3];

    return i & HASH_MASK;
}

//----------------------------------------------------------------
// 函数名称 an_detail_user_t_create
/// @brief 创建an_detail_user_t节点并初始化
/// 
/// @param void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
an_detail_user_t* an_detail_user_t_create(void)
{
	an_detail_user_t* temp = NULL;
	temp = (an_detail_user_t*)wys_malloc(sizeof(an_detail_user_t));
	if(temp)
	{
		INIT_HLIST_NODE(&temp->hash);
		INIT_LIST_HEAD(&temp->user_list);
		INIT_LIST_HEAD(&temp->group_list);
		INIT_LIST_HEAD(&temp->action_list);
	}

	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_detail_user_t_find_by_uid
/// @brief 根据uid,在an_detail_user_hash中查找an_detail_user_t是否存在
/// 
/// @param _in uid->用户ID
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
an_detail_user_t* an_detail_user_t_find_by_uid(_u32 uid)
{
	struct hlist_head *head = NULL;
	struct hlist_node *pos = NULL;
	an_detail_user_t *temp = NULL;
	_u32 i = 0;

	if(uid == 0 || uid == 0xffffffff)
		return NULL;

	i = an_detail_user_t_get_hash(uid);
	head = &an_detail_user_hash[i];

	hlist_for_each(pos, head)
	{
		temp = hlist_entry(pos, an_detail_user_t, hash);

		if(temp->uid == uid)
			return temp;
	}

	return NULL;
}

//----------------------------------------------------------------
// 函数名称 an_detail_user_t_find_create_by_uid
/// @brief 根据uid,查找或创建an_detail_user_t
/// 
/// @param _in uid->用户ID
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
an_detail_user_t* an_detail_user_t_find_create_by_uid(_u32 uid)
{
	an_detail_user_t *temp = NULL;
	temp = an_detail_user_t_find_by_uid(uid);

	if(temp)
		goto out;

	temp = an_detail_user_t_create();

out:
	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_detail_user_t_del
/// @brief 删除an_detail_user_t节点
/// 
/// @param _in temp->要删除的节点地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_detail_user_t_del(an_detail_user_t * temp)
{
	struct list_head * pos = NULL;
	an_detail_act_t* action = NULL;

	if(temp)
	{
		pos = temp->action_list.next;
		while(pos != &temp->action_list)
		{
			action = list_entry(pos, an_detail_act_t, list);
			pos = pos->next;
			list_del(&action->list);
			wys_free(action);
			action = NULL;
		}
		hlist_del(&temp->hash);
		list_del(&temp->user_list);
		list_del(&temp->group_list);
		list_del(&temp->action_list);
		wys_free(temp);
	}
}

//----------------------------------------------------------------
// 函数名称 an_detail_user_t_del_group
/// @brief 清理某组下所有用户(针对an_detail_user_group_list,基本没用)
/// 
/// @param _in gid->要清理的组ID
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_detail_user_t_del_group(_s32 gid)
{
	struct list_head * pos = NULL;
	an_detail_user_t * user = NULL;

	pos = an_detail_user_group_list[gid].next;
	while(pos != &an_detail_user_group_list[gid])
	{
		user = list_entry(pos, an_detail_user_t, group_list);
		pos = pos->next;
		an_detail_user_t_del(user);
		user = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 an_detail_user_t_clear
/// @brief 清空an_detail_user_list
/// 
/// @param _in gid->要清理的组ID
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_detail_user_t_clear(void)
{
	struct list_head * pos = NULL;
	an_detail_user_t * user = NULL;

	pos = an_detail_user_list.next;
	while(pos != &an_detail_user_list)
	{
		user = list_entry(pos, an_detail_user_t, user_list);
		pos = pos->next;
		an_detail_user_t_del(user);
		user = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 an_detail_act_t_del
/// @brief 删除单个an_detail_act_t
/// 
/// @param _in action->要清理的内存地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_detail_act_t_del(an_detail_act_t* action)
{
	if(action)
	{
		list_del_init(&action->list);
		wys_free(action);
	}
}

//----------------------------------------------------------------
// 函数名称 an_detail_wk_act_t_create
/// @brief 创建an_detail_wk_act_t节点并初始化
/// 
/// @param void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
an_detail_wk_act_t* an_detail_wk_act_t_create(void)
{
	an_detail_wk_act_t* temp = NULL;

	temp = (an_detail_wk_act_t*)wys_malloc(sizeof(an_detail_wk_act_t));
	if(temp)
	{
		INIT_LIST_HEAD(&temp->list);
	}

	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_detail_wk_act_t_del
/// @brief 删除单个an_detail_wk_act_t节点
/// 
/// @param _in temp->要清理的内存地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_detail_wk_act_t_del(an_detail_wk_act_t* temp)
{
	if(temp)
	{
		list_del(&temp->list);
		wys_free(temp);
	}
}

//----------------------------------------------------------------
// 函数名称 an_detail_wk_act_t_del_list
/// @brief 清理指定链表(针对an_detail_wk_act_t)
/// 
/// @param _in head->要清理的链表表头地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_detail_wk_act_t_del_list(struct list_head* head)
{
	struct list_head * pos = NULL;
	an_detail_wk_act_t * detail_act = NULL;

	pos = head->next;
	while(pos != head)
	{
		detail_act = list_entry(pos, an_detail_wk_act_t, list);
		pos = pos->next;
		an_detail_wk_act_t_del(detail_act);
		detail_act = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 an_detail_act_t_create
/// @brief 创建an_detail_act_t节点并初始化
/// 
/// @param void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
an_detail_act_t* an_detail_act_t_create(void)
{
	an_detail_act_t* temp = NULL;

	temp = (an_detail_act_t*)wys_malloc(sizeof(an_detail_act_t));
	if(temp)
	{
		INIT_LIST_HEAD(&temp->list);
	}

	return temp;
}

// TODO:优化效率
//----------------------------------------------------------------
// 函数名称 ww_get_category_by_ww_action_list_st
/// @brief 获取页面关注的行为统计类型(AN_CATEGORY)
/// 
/// @param _in act->要查询的行为信息
/// @param _in gid->该行为所属组ID
/// @return AN_CATEGORY中的类型
/// @see AN_CATEGORY ww_action.h
/// @note 此处是根据struct ww_action_list_st解析
/// @author miaomiao
//---------------------------------------------------------------
_u32 ww_get_category_by_ww_action_list_st(struct ww_action_list_st * act, _u32 gid)
{
	_s8** ptr = NULL;
	void* work_check = NULL;

	switch(act->category)
	{
	case LC_WARNING:
	case LC_FORBID:
		return AN_CALC_ATTEMPT;
	};

	//此处analyzer不做辅助违规判断，违规行为全交客户端判断
	#if 0
	if(forbid_act_find_by_process(act->pro_name,gid))
		return AN_CALC_ATTEMPT;
	else if(forbid_act_find_by_wnd_name(act->act_name,gid))
		return AN_CALC_ATTEMPT;
	#endif

	work_check = (void*)an_work_rule_t_find_by_type_name_gid_all(0,act->pro_name,gid);
	if(!work_check)
		work_check = (void*)an_work_rule_t_find_by_type_name_gid_all(1,act->act_name,gid);

	if(work_check)
		return AN_CALC_WORK;

	switch(act->type)
	{
	case OT_BROWSER_OPEN_URL:
	case OT_BROWSER_CLOSE_URL:
		return AN_CALC_URL;
	};

	ptr = ww_chat_list;
	while(**ptr)
	{
		if(strcasecmp(*ptr,act->pro_name)==0)
		{
			return AN_CALC_CHAT;
		}
		ptr++;
	}

	ptr = ww_move_list;
	while(**ptr)
	{
		if(strcasecmp(*ptr,act->pro_name)==0)
		{
			return AN_CALC_VIDEO;
		}
		ptr++;
	}

	return AN_CALC_OTHER;
}

//----------------------------------------------------------------
// 函数名称 an_detail_act_t_create_to_list
/// @brief 创建an_detail_act_t并初始化(添加至用户行为记录链表)
/// 
/// 此接口用于将数据库中的用户日志记录转化为an_detail_act_t,
///	然后添加到相关用户行为记录链表中
/// @param _inout list->被添加的链表表头
/// @param _in user_action->数据库中的一条用户日志记录
/// @param _in gid->用户所属组ID
/// @return NULL->failed,addr->sucess
/// @see auto_save_detail_act ww_action.h
/// @author miaomiao
//---------------------------------------------------------------
an_detail_act_t* an_detail_act_t_create_to_list(struct list_head* list, action_st * user_action, _u32 gid)
{
	an_detail_act_t* temp = NULL;
	_u32 t = ntohl(user_action->type);
	wnd_swtich_log_st* wnd_swtich = (wnd_swtich_log_st*)user_action->data;
	browser_action_log_st* browser_action = (browser_action_log_st*)user_action->data;
	void* work_check = NULL;


	temp = an_detail_act_t_create();
	if(!temp)
		goto out;


	list_add_tail(&temp->list, list);
	temp->time = GMT_REVERS_MASK-ntohll(user_action->start_time);
	temp->type = t;
	//temp->is_ciphertext = ntohs(user_action->is_ciphertext);
	snprintf(temp->process, sizeof(temp->process), "%s", user_action->process);

//目前关心的行为类型
/*	OT_BROWSER_OPEN_URL
	OT_WINDOW_OPEN
	OT_PROCESS_CREATE
	OT_FILE_CREATE
	OT_FILE_DELETE
	OT_PROCESS_ATTEMPT_CREATE
	OT_WINDOW_ATTEMPT_OPEN 
	OT_BROWSER_ATTEMPT_OPEN_URL */

	switch(t)
	{
		case OT_PROCESS_ATTEMPT_CREATE:
		case OT_WINDOW_ATTEMPT_OPEN:
		case OT_BROWSER_ATTEMPT_OPEN_URL:
		case OT_FILE_ATTEMPT_CREATE_PROCESS://违规进程
			temp->category = AN_DETAIL_ATTEMPT;
			break;
		case OT_BROWSER_OPEN_URL:
			temp->category = AN_DETAIL_URL_OPEN;
			break;
		case OT_WINDOW_OPEN:
			temp->category = AN_DETAIL_WINDOW_OPEN;
			break;
		case OT_PROCESS_CREATE:
			temp->category = AN_DETAIL_PROCESS_RUN;
			break;
		case OT_FILE_CREATE:
			temp->category = AN_DETAIL_FILE_CREATE;
			break;
		case OT_FILE_DELETE:
			temp->category = AN_DETAIL_FILE_DEL;
			break;
	};

	//窗口相关
	if(t >= OT_WINDOW_OPEN && t < OT_FILE_CREATE)
	{
		snprintf(temp->actname,sizeof(temp->actname),"%s",wnd_swtich->data);
	}
	//浏览器相关
	else if(t >= OT_BROWSER_OPEN_URL && t <= OT_BROWSER_ATTEMPT_OPEN_URL)//OT_BROWSER_ATTEMPT_OPEN_URL按照常规格式解
	{
		snprintf(temp->actname,sizeof(temp->actname),"%s",browser_action->data);
	}
	else if(t == OT_FILE_ATTEMPT_CREATE_PROCESS)
	{
		memset(temp->actname,0,sizeof(temp->actname));
	}
	//其他操作
	else if(user_action->data_len)
	{
		snprintf(temp->actname,sizeof(temp->actname),"%s",user_action->data);
	}

	switch(ntohl(user_action->category))
	{
	case LC_WARNING:
	case LC_FORBID:
		temp->category = AN_DETAIL_ATTEMPT;
		break;
	};

	//暂时违规行为优先级高于办公行为
	if(temp->category != AN_DETAIL_ATTEMPT)
	{
		work_check = (void*)an_work_rule_t_find_by_type_name_gid_all(0,temp->process,gid);
		if(!work_check)
			work_check = (void*)an_work_rule_t_find_by_type_name_gid_all(1,temp->actname,gid);

		if(work_check)
			temp->category = AN_DETAIL_WORK;
	}

out:
	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_qq_user_t_get_hash
/// @brief 通过uid获取哈希值,(针对an_qq_user_hash)
/// 
/// @param _in uid->用户ID
/// @return uid的hash值
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_qq_user_t_get_hash(_u32 uid)
{
	union {
		_u32 l;
		_u8 c[4];
	} id;
	_u32 i = 0;
	id.l = uid;

	i = id.c[0]+id.c[1]+id.c[2]+id.c[3];

    return i & HASH_MASK;
}

//----------------------------------------------------------------
// 函数名称 an_qq_user_t_create
/// @brief 创建an_qq_user_t并初始化
/// 
/// @param void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
an_qq_user_t* an_qq_user_t_create(void)
{
	an_qq_user_t* temp = NULL;
	_s32 i = 0;
	temp = (an_qq_user_t*)wys_malloc(sizeof(an_qq_user_t));
	if(temp)
	{
		for(i = 0; i < HASH_SIZE; i++)
		{
			INIT_HLIST_HEAD(&temp->qq_hash[i]);
		}
		INIT_LIST_HEAD(&temp->user_list);
		INIT_LIST_HEAD(&temp->group_list);
		INIT_LIST_HEAD(&temp->qq_list);
	}

	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_qq_user_t_find_by_uid
/// @brief 根据uid,在an_qq_user_hash中查找an_qq_user_t是否存在
/// 
/// @param _in uid->用户ID
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
an_qq_user_t* an_qq_user_t_find_by_uid(_u32 uid)
{
	struct hlist_head *head = NULL;
	struct hlist_node *pos = NULL;
	an_qq_user_t *temp = NULL;
	_u32 i = 0;

	if(uid == 0 || uid == 0xffffffff)
		return NULL;

	i = an_qq_user_t_get_hash(uid);
	head = &an_qq_user_hash[i];

	hlist_for_each(pos, head)
	{
		temp = hlist_entry(pos, an_qq_user_t, user_hash);

		if(temp->uid == uid)
			return temp;
	}

	return NULL;
}

//----------------------------------------------------------------
// 函数名称 an_qq_user_t_find_create_by_uid
/// @brief 查找或创建an_qq_user_t
/// 
/// @param _in uid->用户ID
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
an_qq_user_t* an_qq_user_t_find_create_by_uid(_u32 uid)
{
	an_qq_user_t *temp = NULL;
	temp = an_qq_user_t_find_by_uid(uid);

	if(temp)
		goto out;

	temp = an_qq_user_t_create();

out:
	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_qq_user_t_del
/// @brief 删除an_qq_user_t节点
/// 
/// @param _in temp->要清理的内存地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_qq_user_t_del(an_qq_user_t *temp)
{
	struct list_head * obj_pos = NULL;
	struct list_head * msg_pos = NULL;
	an_qq_obj_t* obj = NULL;
	an_qq_msg_t* msg = NULL;

	if(temp)
	{
		obj_pos = temp->qq_list.next;
		while(obj_pos != &temp->qq_list)
		{
			obj = list_entry(obj_pos, an_qq_obj_t, qq_list);
			obj_pos = obj_pos->next;
			msg_pos = obj->msg_list.next;
			while(msg_pos != &obj->msg_list)
			{
				msg = list_entry(msg_pos, an_qq_msg_t, msg_list);
				msg_pos = msg_pos->next;
				list_del(&msg->msg_list);
				wys_free(msg);
				msg = NULL;
			}

			list_del(&obj->qq_list);
			hlist_del(&obj->hash);
			wys_free(obj);
			obj = NULL;
		}
		hlist_del(&temp->user_hash);
		list_del(&temp->user_list);
		list_del(&temp->group_list);
		list_del(&temp->qq_list);
		wys_free(temp);
	}
}

//----------------------------------------------------------------
// 函数名称 an_qq_user_t_del_group
/// @brief 清理某组下的用户聊天记录(针对an_qq_user_group_list,基本没用)
/// 
/// @param _in gid->要清理的组ID
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_qq_user_t_del_group(_s32 gid)
{
	struct list_head * pos = NULL;
	an_qq_user_t * user = NULL;

	pos = an_qq_user_group_list[gid].next;
	while(pos != &an_qq_user_group_list[gid])
	{
		user = list_entry(pos, an_qq_user_t, group_list);
		pos = pos->next;
		an_qq_user_t_del(user);
		user = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 an_qq_user_t_clear
/// @brief 清理全部用户聊天记录(针对an_qq_user_list)
/// 
/// @param void->无参
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_qq_user_t_clear(void)
{
	struct list_head * pos = NULL;
	an_qq_user_t * user = NULL;

	pos = an_qq_user_list.next;
	while(pos != &an_qq_user_list)
	{
		user = list_entry(pos, an_qq_user_t, user_list);
		pos = pos->next;
		an_qq_user_t_del(user);
		user = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 an_qq_obj_t_get_hash
/// @brief 通过QQ账号获取哈希值(针对an_qq_user_t中qq_hash成员)
/// 
/// @param _in qq_account->QQ账号
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_qq_obj_t_get_hash(_u32 qq_account)
{
	union {
		_u32 l;
		_u8 c[4];
	} id;
	_u32 i = 0;
	id.l = qq_account;

	i = id.c[0]+id.c[1]+id.c[2]+id.c[3];

    return i & HASH_MASK;
}

//----------------------------------------------------------------
// 函数名称 an_qq_obj_t_create
/// @brief 创建an_qq_obj_t节点并初始化
/// 
/// @param void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
an_qq_obj_t* an_qq_obj_t_create(void)
{
	an_qq_obj_t* temp = NULL;
	temp = (an_qq_obj_t*)wys_malloc(sizeof(an_qq_obj_t));
	if(temp)
	{
		INIT_LIST_HEAD(&temp->msg_list);
		INIT_HLIST_NODE(&temp->hash);
		INIT_LIST_HEAD(&temp->qq_list);
	}

	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_qq_obj_t_find_by_qq_account
/// @brief 根据QQ账号,在指定的hash中查找an_qq_obj_t是否存在
/// 
/// @param _in qq_account->QQ账号
/// @param _in hash_head->hash表头地址
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
an_qq_obj_t* an_qq_obj_t_find_by_qq_account(_u32 qq_account, struct hlist_head * hash_head)
{
	struct hlist_head *head = NULL;
	struct hlist_node *pos = NULL;
	an_qq_obj_t *temp = NULL;
	_u32 i = 0;

	i = an_qq_obj_t_get_hash(qq_account);
	head = hash_head+i;

	hlist_for_each(pos, head)
	{
		temp = hlist_entry(pos, an_qq_obj_t, hash);

		if(temp->qq_account == qq_account)
			return temp;
	}

	return NULL;
}

//----------------------------------------------------------------
// 函数名称 an_qq_obj_t_find_create_by_qq_account
/// @brief 根据QQ账号,在指定的hash中查找或创建an_qq_obj_t
/// 
/// @param _in qq_account->QQ账号
/// @param _in hash_head->hash表头地址
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
an_qq_obj_t* an_qq_obj_t_find_create_by_qq_account(_u32 qq_account, struct hlist_head * hash_head)
{
	an_qq_obj_t *temp = NULL;
	temp = an_qq_obj_t_find_by_qq_account(qq_account, hash_head);

	if(temp)
		goto out;

	temp = an_qq_obj_t_create();

out:
	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_qq_msg_t_create
/// @brief 创建an_qq_msg_t节点并初始化
/// 
/// @param _in msg_len->QQ消息长度
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
an_qq_msg_t* an_qq_msg_t_create(_u32 msg_len)
{
	an_qq_msg_t* temp = NULL;
	temp = (an_qq_msg_t*)wys_malloc(sizeof(an_qq_msg_t)+msg_len);
	if(temp)
	{
		INIT_LIST_HEAD(&temp->msg_list);
		temp->msg_len = msg_len;
	}

	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_qq_msg_t_del
/// @brief 删除an_qq_msg_t
/// 
/// @param _in temp->要清理的内存地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_qq_msg_t_del(an_qq_msg_t* temp)
{
	if(temp)
	{
		list_del(&temp->msg_list);
		wys_free(temp);
	}
}

//----------------------------------------------------------------
// 函数名称 an_qq_wk_msg_t_get_hash
/// @brief 根据QQ账号获取hash值
/// 
/// 此处获取的hash不针对具体hash表,
/// 仅针对an_qq_wk_msg_t结构体本身,
/// 用于页面查询数据时生成的一些临时hash表
/// @param _in qq_account->QQ账号
/// @return QQ账号对应的hash值
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_qq_wk_msg_t_get_hash(_u32 qq_account)
{
	union {
		_u32 l;
		_u8 c[4];
	} id;
	_u32 i = 0;
	id.l = qq_account;

	i = id.c[0]+id.c[1]+id.c[2]+id.c[3];

    return i & HASH_MASK;
}

//----------------------------------------------------------------
// 函数名称 an_qq_wk_msg_t_create
/// @brief 创建an_qq_wk_msg_t并初始化
/// 
/// @param _in msg_len->QQ消息长度
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
an_qq_wk_msg_t* an_qq_wk_msg_t_create(_u32 msg_len)
{
	an_qq_wk_msg_t* temp = NULL;
	temp = (an_qq_wk_msg_t*)wys_malloc(sizeof(an_qq_wk_msg_t)+msg_len);
	if(temp)
	{
		INIT_LIST_HEAD(&temp->msg_list);
		INIT_HLIST_NODE(&temp->hash);
		temp->msg_len = msg_len;
	}

	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_qq_wk_msg_t_find_by_qq_account
/// @brief 根据QQ账号和用户ID在指定hash表中查找an_qq_wk_msg_t节点
/// 
/// @param _in qq_account->QQ账号
/// @param _in uid->用户ID
/// @param _in hash_head->要hash表头地址
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
an_qq_wk_msg_t* an_qq_wk_msg_t_find_by_qq_account(_u32 qq_account, _u32 uid, struct hlist_head * hash_head)
{
	struct hlist_head *head = NULL;
	struct hlist_node *pos = NULL;
	an_qq_wk_msg_t *temp = NULL;
	_u32 i = 0;

	i = an_qq_wk_msg_t_get_hash(qq_account);
	head = hash_head+i;

	hlist_for_each(pos, head)
	{
		temp = hlist_entry(pos, an_qq_wk_msg_t, hash);

		if(temp->obj_account == qq_account && temp->uid == uid)
			return temp;
	}

	return NULL;
}

//----------------------------------------------------------------
// 函数名称 an_qq_wk_msg_t_del
/// @brief 删除an_qq_wk_msg_t节点
/// 
/// @param _in temp->要清理的内存地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_qq_wk_msg_t_del(an_qq_wk_msg_t* temp)
{
	if(temp)
	{
		list_del(&temp->msg_list);
		if(temp->hash.next != NULL)
			hlist_del(&temp->hash);
		wys_free(temp);
	}
}

//----------------------------------------------------------------
// 函数名称 an_qq_wk_msg_t_del_list
/// @brief 清理具体链表(针对an_qq_wk_msg_t)
/// 
/// @param _in head->要清理的链表头地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_qq_wk_msg_t_del_list(struct list_head* head)
{
	struct list_head * pos = NULL;
	an_qq_wk_msg_t * qq_msg = NULL;

	pos = head->next;
	while(pos != head)
	{
		qq_msg = list_entry(pos, an_qq_wk_msg_t, msg_list);
		pos = pos->next;
		an_qq_wk_msg_t_del(qq_msg);
		qq_msg = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 an_work_rule_t_get_hash
/// @brief 通过name获取哈希值(针对an_work_rule_hash)
/// 
/// @param _in name->办公行为描述
/// @return name的hash值
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_work_rule_t_get_hash(_s8* name)
{
	_s32 i = 0;
	_u32 index = 0;
	//只加前8位
	for(i = 0; i < 8; i++)
	{
		//加到'\0'为止
		if(name[i])
			index += name[i];
		else
			break;
	}
	
    return index & HASH_MASK;
}

//----------------------------------------------------------------
// 函数名称 an_work_rule_t_get_hash
/// @brief 创建an_work_rule_t并初始化
/// 
/// @param void->无参
/// @return NULL->failed,addr->sucess
/// @author miaomiao
//---------------------------------------------------------------
an_work_rule_t* an_work_rule_t_create(void)
{
	an_work_rule_t* temp = NULL;
	temp = (an_work_rule_t*)wys_malloc(sizeof(an_work_rule_t));
	if(temp)
	{
		INIT_LIST_HEAD(&temp->rule_list);
		INIT_HLIST_NODE(&temp->hash);
		INIT_LIST_HEAD(&temp->group_list);
	}

	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_work_rule_t_find_by_type_name_gid
/// @brief 根据type,name,gid查找an_work_rule_t是否存在,全匹配
/// 
/// @param _in type->办公行为类型,0:使用软件,1:访问网页
/// @param _in name->办公行为描述(进程名 或 域名)
/// @param _in gid->规则所属组id
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
an_work_rule_t* an_work_rule_t_find_by_type_name_gid(_u8 type, _s8* name, _u32 gid)
{
	struct hlist_head *head = NULL;
	struct hlist_node *pos = NULL;
	an_work_rule_t *temp = NULL;
	_u32 i = 0;

	i = an_work_rule_t_get_hash(name);
	head = &an_work_rule_hash[i];

	hlist_for_each(pos, head)
	{
		temp = hlist_entry(pos, an_work_rule_t, hash);

		if(type == 0 && type == temp->worktype && gid == temp->gid && strncmp(temp->process,name,sizeof(temp->process)) == 0)
			return temp;
		else if(type == 1 && type == temp->worktype && gid == temp->gid && strncmp(temp->workname,name,sizeof(temp->workname)) == 0)
			return temp;
	}

	return NULL;
}

//----------------------------------------------------------------
// 函数名称 an_work_rule_t_find_by_type_name_gid_all
/// @brief 根据type,name,gid(含全部员工)查找an_work_rule_t是否存在,全匹配
/// 
/// 与an_work_rule_t_find_by_type_name_gid不同
/// 该函数在搜索时,规则所属gid(组ID)为0也算匹配上
/// @param _in type->办公行为类型,0:使用软件,1:访问网页
/// @param _in name->办公行为描述(进程名 或 域名)
/// @param _in gid->规则所属组id
/// @return NULL->failed,addr->found
/// @author miaomiao
//---------------------------------------------------------------
an_work_rule_t* an_work_rule_t_find_by_type_name_gid_all(_u8 type, _s8* name, _u32 gid)
{
	struct hlist_head *head = NULL;
	struct hlist_node *pos = NULL;
	an_work_rule_t *temp = NULL;
	_u32 i = 0;

	i = an_work_rule_t_get_hash(name);
	head = &an_work_rule_hash[i];

	hlist_for_each(pos, head)
	{
		temp = hlist_entry(pos, an_work_rule_t, hash);

		if(type == 0 && type == temp->worktype && (temp->gid == 0 || gid == temp->gid) && strncmp(temp->process,name,sizeof(temp->process)) == 0)
			return temp;
		else if(type == 1 && type == temp->worktype && (temp->gid == 0 || gid == temp->gid) && strncmp(temp->workname,name,sizeof(temp->workname)) == 0)
			return temp;
	}

	return NULL;
}

//----------------------------------------------------------------
// 函数名称 an_work_rule_t_find_create_by_name_gid
/// @brief 根据type,name,gid查找或创建an_work_rule_t
/// 
/// @param _in type->办公行为类型,0:使用软件,1:访问网页
/// @param _in name->办公行为描述(进程名 或 域名)
/// @param _in gid->规则所属组id
/// @return NULL->failed,addr->found/create
/// @author miaomiao
//---------------------------------------------------------------
an_work_rule_t* an_work_rule_t_find_create_by_name_gid(_u8 type, _s8* name, _u32 gid)
{
	an_work_rule_t *temp = NULL;
	temp = an_work_rule_t_find_by_type_name_gid(type,name,gid);

	if(temp)
		goto out;

	temp = an_work_rule_t_create();

out:
	return temp;
}

//----------------------------------------------------------------
// 函数名称 an_work_rule_t_find_create_by_name_gid
/// @brief 删除an_work_rule_t(单个规则)
/// 
/// @param _in temp->要清理的内存地址
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_work_rule_t_del(an_work_rule_t* temp)
{
	if(temp)
	{
		list_del(&temp->rule_list);
		list_del(&temp->group_list);
		hlist_del(&temp->hash);
		wys_free(temp);
	}
}

//----------------------------------------------------------------
// 函数名称 an_work_rule_t_del_group
/// @brief 清空某组的所有办公行为规则(仅针对链表,不处理数据库)
/// 
/// @param _in gid->要清理的组ID
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_work_rule_t_del_group(_s32 gid)
{
	struct list_head * pos = NULL;
	an_work_rule_t * work = NULL;

	pos = an_work_group_list[gid].next;
	while(pos != &an_work_group_list[gid])
	{
		work = list_entry(pos, an_work_rule_t, group_list);
		pos = pos->next;
		an_work_rule_t_del(work);
		work = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 an_work_rule_t_clear
/// @brief 清空所有办公行为规则(仅针对链表,不处理数据库)
/// 
/// @param void->无参
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_work_rule_t_clear(void)
{
	struct list_head * pos = NULL;
	an_work_rule_t * work = NULL;

	pos = an_work_rule_list.next;
	while(pos != &an_work_rule_list)
	{
		work = list_entry(pos, an_work_rule_t, rule_list);
		pos = pos->next;
		an_work_rule_t_del(work);
		work = NULL;
	}
}

//----------------------------------------------------------------
// 函数名称 an_hash_and_list_init
/// @brief 初始化全局hash表/链表
/// 
/// @param void->无参
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_hash_and_list_init(void)
{
	_s32 i = 0;
	
	for (i = 0; i < HASH_SIZE; i++)
	{
		INIT_HLIST_HEAD(&an_forbid_hash[i]);
	}
	
	for (i = 0; i < HASH_SIZE; i++)
	{
		INIT_HLIST_HEAD(&ww_user_hash[i]);
	}
	
	for (i = 0; i < HASH_SIZE; i++)
	{
		INIT_HLIST_HEAD(&an_group_hash[i]);
	}

	for (i = 0; i < HASH_SIZE; i++)
	{
		INIT_HLIST_HEAD(&encrypt_hash[i]);
	}
	
	for (i = 0; i < HASH_SIZE; i++)
	{
		INIT_HLIST_HEAD(&an_sock_hash[i]);
	}

	for (i = 0; i < HASH_SIZE; i++)
	{
		INIT_HLIST_HEAD(&an_web_cache_hash[i]);
	}

	for (i = 0; i < HASH_SIZE; i++)
	{
		INIT_HLIST_HEAD(&an_detail_user_hash[i]);
	}

	for (i = 0; i < HASH_SIZE; i++)
	{
		INIT_HLIST_HEAD(&an_qq_user_hash[i]);
	}

	for (i = 0; i < HASH_SIZE; i++)
	{
		INIT_HLIST_HEAD(&an_work_rule_hash[i]);
	}
	
	for (i = 0; i < GROUP_ID_MAX; i++)
	{
		INIT_LIST_HEAD(&an_forbid_group_list[i]);
	}
	
	for (i = 0; i < GROUP_ID_MAX; i++)
	{
		INIT_LIST_HEAD(&ww_user_group_list[i]);
	}

	for (i = 0; i < GROUP_ID_MAX; i++)
	{
		INIT_LIST_HEAD(&encrypt_group_list[i]);
	}

	for (i = 0; i < GROUP_ID_MAX; i++)
	{
		INIT_LIST_HEAD(&an_detail_user_group_list[i]);
	}

	for (i = 0; i < GROUP_ID_MAX; i++)
	{
		INIT_LIST_HEAD(&an_qq_user_group_list[i]);
	}

	for (i = 0; i < GROUP_ID_MAX; i++)
	{
		INIT_LIST_HEAD(&an_work_group_list[i]);
	}
	
}




