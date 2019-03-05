/////////////////////////////////////////////////////////////////////
/// @file auto_data_save.c
/// @brief 将analyzer缓存在内存的数据整理并定时存入数据库
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#include "hash.h"
#include "debug.h"
#include "mem.h"
#include "thread.h"
#include "prefix.h"
#include "db.h"
#include "list.h"
#include "worker.h"
#include "cmds.h"
#include "ww_action.h"
#include "public.h"
#include "auto_data_save.h"

//----------------------------------------------------------------
// 函数名称 set_systime
/// @brief 设置系统时间
/// 
/// @param _in sec->要设置时间的GMT秒数
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void set_systime(_u32 sec)
{ 
    struct timeval tv;
    tv.tv_sec  = sec;
    tv.tv_usec = 0;// sorry, I'm not a time geek :P	  
    settimeofday(&tv, NULL);
}

//----------------------------------------------------------------
// 函数名称 ww_del_user_db_data_by_uid_type_gmt_prefix
/// @brief 根据用户ID,类型,时间,指定头清理数据库中数据
/// 
/// 一般用于清理PRE_USER_DETAIL_ACTION开头的数据
/// 在数据库中key必须以 头-uid-type-GMT 的形式保存
/// @param _in gmt_u64->要保留的最老的数据所在的GMT时间,注意逆序
/// @param _in uid->要清理的数据所属的uid
/// @param _in prefix->key头 @see PRE_USER_DETAIL_ACTION prefix.h
/// @param _in type->类型 @see USER_ACTION_TYPE ww_action.h
/// @return -1->error,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_del_user_db_data_by_uid_type_gmt_prefix(_u64 gmt_u64, _u32 uid, _s8* prefix, _u32 type)
{
	DB* db_ret = NULL;
	DB* db_del_temp = NULL;
	_s32 key_len;
	_s8 keybuf[64];
	_s32 i;
	_s8 scanstart[64] = {};
	_s8 scanend[64] = {};
	_s32 start_len = 0;
	_s32 end_len = 0;

	_s8* p_gmt = NULL;

	do{
		dbfree(db_ret);
		start_len = snprintf(scanstart,sizeof(scanstart),"%s%d-%d-%llu",prefix,uid,type,gmt_u64?gmt_u64+1:0);
		end_len = snprintf(scanend,sizeof(scanend),"%s%d-%d.",prefix,uid,type);
		db_ret = dbscan(scanstart,start_len,scanend,end_len,DB_KVN_MAX);
		if(!db_ret || db_ret->rno != DB_SCAN_OK)
		{
			return -1;
		}

		if(db_ret->kvn)
		{
			p_gmt = (db_ret->kvs[db_ret->kvn-1].key + strlen(prefix) + USERID_LEN);
			gmt_u64 = atoull(p_gmt);
		}

		for(i = 0; i < db_ret->kvn; i++)
		{
			key_len = snprintf(keybuf,sizeof(keybuf),"%s",db_ret->kvs[i].key);
			db_del_temp = dbdel(keybuf,key_len);
			dbfree(db_del_temp);
			db_del_temp = NULL;
		}
	}while(db_ret && db_ret->kvn > 0);

	dbfree(db_ret);
	dbfree(db_del_temp);

	return 0;
}

//----------------------------------------------------------------
// 函数名称 ww_del_user_db_data_by_gmt_uid_prefix
/// @brief 根据用户ID,时间,指定头清理数据库中数据
/// 
/// 部分数动态数据用此接口清理
/// 在数据库中key必须以 头-uid-GMT 的形式保存
/// @param _in gmt_u64->要保留的最老的数据所在的GMT时间,注意逆序
/// @param _in uid->要清理的数据所属的uid
/// @param _in prefix->key头
/// @return -1->error,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_del_user_db_data_by_gmt_uid_prefix(_u64 gmt_u64, _u32 uid, _s8* prefix)// gmt必须为逆序 id必须为uid
{
	DB* db_ret = NULL;
	DB* db_del_temp = NULL;
	_s32 key_len;
	_s8 keybuf[64];
	_s32 i;
	_s8 scanstart[64] = {};
	_s8 scanend[64] = {};
	_s32 start_len = 0;
	_s32 end_len = 0;

	_s8* p_gmt = NULL;

	do{
		dbfree(db_ret);
		start_len = snprintf(scanstart,sizeof(scanstart),"%s%d-%llu",prefix,uid,gmt_u64?gmt_u64+1:0);
		end_len = snprintf(scanend,sizeof(scanend),"%s%d.",prefix,uid);
		db_ret = dbscan(scanstart,start_len,scanend,end_len,DB_KVN_MAX);
		if(!db_ret || db_ret->rno != DB_SCAN_OK)
		{
			return -1;
		}

		if(db_ret->kvn)
		{
			p_gmt = (db_ret->kvs[db_ret->kvn-1].key + strlen(prefix) + USERID_LEN);
			gmt_u64 = atoull(p_gmt);
		}

		for(i = 0; i < db_ret->kvn; i++)
		{
			key_len = snprintf(keybuf,sizeof(keybuf),"%s",db_ret->kvs[i].key);
			db_del_temp = dbdel(keybuf,key_len);
			dbfree(db_del_temp);
			db_del_temp = NULL;
		}
	}while(db_ret && db_ret->kvn > 0);

	dbfree(db_ret);
	dbfree(db_del_temp);

	return 0;
}

//----------------------------------------------------------------
// 函数名称 ww_del_user_qq_msg_data_by_gmt_uid_prefix
/// @brief 根据用户ID,时间,指定头,QQ账号清理数据库中数据
/// 
/// 此接口仅用于清理PRE_USER_QQ_CALC_MSG
/// 在数据库中key必须以 头-uid-qq_id-GMT 的形式保存
/// @param _in gmt_u64->要保留的最老的数据所在的GMT时间,注意逆序
/// @param _in uid->要清理的数据所属的uid
/// @param _in prefix->key头 @see PRE_USER_QQ_CALC_MSG prefix.h
/// @param _in qq_id->QQ账号
/// @return -1->error,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_del_user_qq_msg_data_by_gmt_uid_prefix(_u64 gmt_u64, _u32 uid, _s8* prefix, _u32 qq_id)// gmt必须为逆序 id必须为uid
{
	DB* db_ret = NULL;
	DB* db_del_temp = NULL;
	_s32 key_len;
	_s8 keybuf[64];
	_s32 i;
	_s8 scanstart[64] = {};
	_s8 scanend[64] = {};
	_s32 start_len = 0;
	_s32 end_len = 0;

	_s8* p_gmt = NULL;

	do{
		dbfree(db_ret);
		start_len = snprintf(scanstart,sizeof(scanstart),"%s%d-%d-%llu",prefix,uid,qq_id,gmt_u64?gmt_u64+1:0);
		end_len = snprintf(scanend,sizeof(scanend),"%s%d-%d.",prefix,uid,qq_id);
		db_ret = dbscan(scanstart,start_len,scanend,end_len,DB_KVN_MAX);
		if(!db_ret || db_ret->rno != DB_SCAN_OK)
		{
			return -1;
		}

		if(db_ret->kvn)
		{
			p_gmt = (db_ret->kvs[db_ret->kvn-1].key + strlen(prefix) + USERID_LEN);
			gmt_u64 = atoull(p_gmt);
		}

		for(i = 0; i < db_ret->kvn; i++)
		{
			key_len = snprintf(keybuf,sizeof(keybuf),"%s",db_ret->kvs[i].key);
			db_del_temp = dbdel(keybuf,key_len);
			dbfree(db_del_temp);
			db_del_temp = NULL;
		}
	}while(db_ret && db_ret->kvn > 0);

	dbfree(db_ret);
	dbfree(db_del_temp);
	
	return 0;
}

//----------------------------------------------------------------
// 函数名称 ww_del_user_qq_data_by_gmt_uid_prefix
/// @brief 根据用户ID,时间,指定头清理数据库中聊天统计数据
/// 
/// 此接口仅用于清理PRE_USER_QQ_CALC_MSGTIME
/// 在数据库中key必须以 头-uid-GMT 的形式保存
/// @param _in gmt_u64->要保留的最老的数据所在的GMT时间,注意逆序
/// @param _in uid->要清理的数据所属的uid
/// @param _in prefix->key头 @see PRE_USER_QQ_CALC_MSGTIME prefix.h
/// @return -1->error,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_del_user_qq_data_by_gmt_uid_prefix(_u64 gmt_u64, _u32 uid, _s8* prefix)// gmt必须为逆序 id必须为uid
{
	DB* db_time_check = NULL;
	DB* db_del_temp = NULL;
	_s32 key_len;
	_s8 keybuf[64];
	_s32 i;
	_s8 scanstart[64] = {};
	_s8 scanend[64] = {};
	_s8 scanflag[64] = {};
	_s32 start_len = 0;
	_s32 end_len = 0;

	_s32 ret = 0;

	an_qq_calc_msgtime_t * qqs = NULL;

	memset(scanflag,0,sizeof(scanflag));
	do{
		dbfree(db_time_check);
		if(strlen(scanflag))
			start_len = snprintf(scanstart,sizeof(scanstart),"%s",scanflag);
		else
			start_len = snprintf(scanstart,sizeof(scanstart),PRE_USER_QQ_CALC_MSGTIME"%d-",uid);
		end_len = snprintf(scanend,sizeof(scanend),PRE_USER_QQ_CALC_MSGTIME"%d.",uid);
		db_time_check = dbscan(scanstart, start_len, scanend, end_len, DB_KVN_MAX);
		if(!db_time_check || db_time_check->rno != DB_SCAN_OK)
		{
			ret = -1;
			goto out;
		}
		for(i = 0; i < db_time_check->kvn; i++)
		{
			qqs = (an_qq_calc_msgtime_t*)db_time_check->kvs[i].val;
			if(i == db_time_check->kvn - 1)
			{
				snprintf(scanflag,sizeof(scanflag),PRE_USER_QQ_CALC_MSGTIME"%d-%d",uid,qqs->account+1);
			}

			ww_del_user_qq_msg_data_by_gmt_uid_prefix(gmt_u64, uid, PRE_USER_QQ_CALC_MSG, qqs->account);

			if(gmt_u64 == 0)
			{
				key_len = snprintf(keybuf,sizeof(keybuf),"%s",db_time_check->kvs[i].key);
				db_del_temp = dbdel(keybuf,key_len);
				dbfree(db_del_temp);
				db_del_temp = NULL;
			}
		}
	}while(db_time_check && db_time_check->kvn);

out:
	dbfree(db_del_temp);
	dbfree(db_time_check);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 ww_del_group_db_data_by_gmt_gid_prefix
/// @brief 根据组ID,时间,指定头清理数据库中聊天统计数据
/// 
/// 此接口仅用于清理PRE_USERGROUP_CALC_ACTION
/// 在数据库中key必须以 头-gid-GMT 的形式保存
/// @param _in gmt_u64->要保留的最老的数据所在的GMT时间,注意逆序
/// @param _in gid->要清理的数据所属的gid
/// @param _in prefix->key头 @see PRE_USERGROUP_CALC_ACTION prefix.h
/// @return -1->error,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_del_group_db_data_by_gmt_gid_prefix(_u64 gmt_u64, _u32 gid, _s8* prefix)// gmt必须为逆序 id必须为gid
{
	DB* db_ret = NULL;
	DB* db_del_temp = NULL;
	_s32 key_len;
	_s8 keybuf[64];
	_s32 i;
	_s8 scanstart[64] = {};
	_s8 scanend[64] = {};
	_s32 start_len = 0;
	_s32 end_len = 0;

	_s8* p_gmt = NULL;

	do{
		dbfree(db_ret);
		start_len = snprintf(scanstart,sizeof(scanstart),"%s%d-%llu",prefix,gid,gmt_u64?gmt_u64+1:0);
		end_len = snprintf(scanend,sizeof(scanend),"%s%d.",prefix,gid);
		db_ret = dbscan(scanstart,start_len,scanend,end_len,DB_KVN_MAX);
		if(!db_ret || db_ret->rno != DB_SCAN_OK)
		{
			return -1;
		}

		if(db_ret->kvn)
		{
			p_gmt = (db_ret->kvs[db_ret->kvn-1].key + strlen(prefix));
			while(*p_gmt != '-')p_gmt++;
			p_gmt++;
			gmt_u64 = atoull(p_gmt);
		}

		for(i = 0; i < db_ret->kvn; i++)
		{
			key_len = snprintf(keybuf,sizeof(keybuf),"%s",db_ret->kvs[i].key);
			db_del_temp = dbdel(keybuf,key_len);
			dbfree(db_del_temp);
			db_del_temp = NULL;
		}
	}while(db_ret && db_ret->kvn > 0);

	dbfree(db_ret);
	dbfree(db_del_temp);

	return 0;
}

//----------------------------------------------------------------
// 函数名称 ww_save_user_calc_act_to_db
/// @brief 保存用户行为统计数据至数据库
/// 
/// @param _in user->存储了所有要保存的数据
/// @return -1->failed,0->sucess
/// @note 需重新set相关组ID的最新同步时间
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_save_user_calc_act_to_db(struct ww_user_action_st * user)
{
	enum{
		act_id
		,time_id
		,group_id
		,allgroup_id
		,max_id
	};

	_s32 ret = 0;
	DB* db_ret = NULL;
	_s8 key_buf[max_id][64] = {};
	_s32 key_len = 0;

	struct ww_action_list_st * action = NULL;
	struct list_head* act_pos = NULL;
	an_user_calc_acts_t* save_datas = NULL;
	an_user_calc_act_t* save_data = NULL;
	
	_s32 act_num = 1024;//没意义，可扩充
	_s32 act_count = 0;

	_u64 group_time = 0;
	_u64 all_group_time = 0;

	_s32 base_save_len = sizeof(an_user_calc_acts_t) + act_num*sizeof(an_user_calc_act_t);

	db_kv* kvs = NULL;

	key_len=snprintf(key_buf[0],sizeof(key_buf[0]),PRE_USERGROUP_CALC_ACTTIME"%d",user->group_id);
	db_ret = dbget(key_buf[0], key_len);
	if(!db_ret || (db_ret->rno != DB_GET_OK && db_ret->rno != DB_NOT_FOUND))
	{
		ret = -1;
		goto OUT;
	}
	if(db_ret->kvn)
		group_time = BTOU64(db_ret->kvs[0].val);

	dbfree(db_ret);
	db_ret = NULL;

	key_len=snprintf(key_buf[0],sizeof(key_buf[0]),PRE_USERGROUP_CALC_ACTTIME"%d",0);
	db_ret = dbget(key_buf[0], key_len);
	if(!db_ret || (db_ret->rno != DB_GET_OK && db_ret->rno != DB_NOT_FOUND))
	{
		ret = -1;
		goto OUT;
	}
	if(db_ret->kvn)
		all_group_time = BTOU64(db_ret->kvs[0].val);

	dbfree(db_ret);
	db_ret = NULL;

	save_datas = (an_user_calc_acts_t*)wys_malloc(base_save_len);
	if(!save_datas)
	{
		ret = -1;
		goto OUT;
	}

	kvs = (db_kv*)wys_malloc(sizeof(db_kv)*max_id);
	if(!kvs)
	{
		ret = -1;
		goto OUT;
	}
	

	save_data = (an_user_calc_act_t*)save_datas->data;
	act_count = 0;
	//user = list_entry(user_pos, struct ww_user_action_st, user_list);
	
	list_for_each(act_pos, &user->action_list)
	{
		act_count++;
		action = list_entry(act_pos, struct ww_action_list_st, action_list);

		if(act_count > act_num)
		{
			act_num += act_num;
			save_datas = wys_realloc(save_datas, sizeof(an_user_calc_acts_t) + act_num*sizeof(an_user_calc_act_t));
			if(save_datas == NULL)
			{
				ret = -1;
				goto OUT;
			}
		}

		save_data->time = action->time;
		save_data->type = action->type;
		snprintf(save_data->pro_name,sizeof(action->pro_name),"%s",action->pro_name);
		snprintf(save_data->act_name,sizeof(action->act_name),"%s",action->act_name);
		save_data->key_click = action->key_click;
		save_data->mou_click = action->mou_click;
		save_data->category = ww_get_category_by_ww_action_list_st(action,user->group_id);
		save_data++;
	}
	save_datas->num = act_count;
	save_datas->gid = user->group_id;

	CHECK(act_count);

	key_len = snprintf(key_buf[act_id],sizeof(key_buf[act_id]),PRE_USER_CALC_ACTION"%d-%llu"
						,user->user_id,user->day_time);
	kvs[act_id].key = key_buf[act_id];
	kvs[act_id].key_len = key_len;
	kvs[act_id].val = (_s8*)save_datas;
	kvs[act_id].val_len = sizeof(an_user_calc_acts_t) + act_count*sizeof(an_user_calc_act_t);

	key_len = snprintf(key_buf[time_id], sizeof(key_buf[time_id]), PRE_USER_CALC_ACTTIME"%d", user->user_id);
	kvs[time_id].key = key_buf[time_id];
	kvs[time_id].key_len = key_len;
	kvs[time_id].val = (_s8*)&user->day_time;
	kvs[time_id].val_len = sizeof(user->day_time);
	INFO("uid = %d,time = %llu\n",user->user_id,user->day_time);

	if(group_time <= user->day_time)
		group_time = (user->day_time + 24*3600);
	key_len = snprintf(key_buf[group_id], sizeof(key_buf[group_id]), PRE_USERGROUP_CALC_ACTTIME"%d", user->group_id);
	kvs[group_id].key = key_buf[group_id];
	kvs[group_id].key_len = key_len;
	kvs[group_id].val = (_s8*)&group_time;
	kvs[group_id].val_len = sizeof(user->day_time);

	if(all_group_time <= user->day_time)
		all_group_time = (user->day_time + 24*3600);
	key_len = snprintf(key_buf[allgroup_id], sizeof(key_buf[allgroup_id]), PRE_USERGROUP_CALC_ACTTIME"%d", 0);
	kvs[allgroup_id].key = key_buf[allgroup_id];
	kvs[allgroup_id].key_len = key_len;
	kvs[allgroup_id].val = (_s8*)&all_group_time;
	kvs[allgroup_id].val_len = sizeof(user->day_time);

	db_ret = dbmulti_set(kvs, max_id);
	if(!db_ret || db_ret->rno != DB_MULTI_SET_OK)
	{
		ret = -1;
		goto OUT;
	}

	dbfree(db_ret);
	db_ret = NULL;

OUT:
	dbfree(db_ret);
	wys_free(save_datas);
	wys_free(kvs);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 ww_save_group_calc_act_to_db
/// @brief 保存用户组行为统计数据至数据库
///
/// 该函数是发现了存储的历史数据有误
/// 或者昨天甚至更早的数据没有保存时才会被调用
/// 重新存储时,用户组的用户数量是按照调用该接口时的用户数量计算的
/// 所以整个用户组当天的平均数据可能不准确
/// @param _in group_id->要保存的组ID
/// @param _in action_list->存储了所有要保存的行为数据
/// @param _in histroy_u64->保存的数据属于哪一天(GMT逆序)
/// @param _in user_cnt->当天该用户组有多少用户数(不准确)
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_save_group_calc_act_to_db(_s32 group_id, struct list_head* action_list, _u64 histroy_u64, _u32 user_cnt)
{
	enum{
		act_id
		,time_id
		,max_id
	};

	_s32 ret = 0;
	DB* db_ret = NULL;
	_s8 key_buf[max_id][64] = {};
	_s32 key_len = 0;

	struct ww_action_list_st * action = NULL;
	struct list_head* act_pos = NULL;
	an_user_calc_acts_t* save_datas = NULL;
	an_user_calc_act_t* save_data = NULL;

	_s32 act_num = 1024;//没意义，可扩充
	_s32 act_count = 0;

	_s32 base_save_len = sizeof(an_user_calc_acts_t) + act_num*sizeof(an_user_calc_act_t);

	db_kv* kvs = NULL;

	save_datas = (an_user_calc_acts_t*)wys_malloc(base_save_len);
	if(!save_datas)
	{
		ret = -1;
		goto OUT;
	}

	kvs = (db_kv*)wys_malloc(sizeof(db_kv)*max_id);
	if(!kvs)
	{
		ret = -1;
		goto OUT;
	}

	save_data = (an_user_calc_act_t*)save_datas->data;
	act_count = 0;
	list_for_each(act_pos, action_list)
	{
		act_count++;
		action = list_entry(act_pos, struct ww_action_list_st, action_list);

		if(act_count > act_num)
		{
			act_num += act_num;
			save_datas = wys_realloc(save_datas, sizeof(an_user_calc_acts_t) + act_num*sizeof(an_user_calc_act_t));
			if(save_datas == NULL)
			{
				ret = -1;
				goto OUT;
			}
		}

		save_data->time = action->time;
		save_data->type = action->type;
		snprintf(save_data->pro_name,sizeof(action->pro_name),"%s",action->pro_name);
		snprintf(save_data->act_name,sizeof(action->act_name),"%s",action->act_name);
		save_data->key_click = action->key_click;
		save_data->mou_click = action->mou_click;
		save_data->category = ww_get_category_by_ww_action_list_st(action,group_id);
		save_data++;
	}
	save_datas->num = act_count;
	save_datas->gid = group_id;
	save_datas->user_cnt = user_cnt;

	key_len = snprintf(key_buf[act_id],sizeof(key_buf[act_id]),PRE_USERGROUP_CALC_ACTION"%d-%llu"
						,group_id, histroy_u64);
	kvs[act_id].key = key_buf[act_id];
	kvs[act_id].key_len = key_len;
	kvs[act_id].val = (_s8*)save_datas;
	kvs[act_id].val_len = sizeof(an_user_calc_acts_t) + act_count*sizeof(an_user_calc_act_t);

	key_len = snprintf(key_buf[time_id], sizeof(key_buf[time_id]), PRE_USERGROUP_CALC_ACTTIME"%d", group_id);
	kvs[time_id].key = key_buf[time_id];
	kvs[time_id].key_len = key_len;
	kvs[time_id].val = (_s8*)&histroy_u64;
	kvs[time_id].val_len = sizeof(histroy_u64);

	db_ret = dbmulti_set(kvs, max_id);
	if(!db_ret || db_ret->rno != DB_MULTI_SET_OK)
	{
		ret = -1;
		goto OUT;
	}

	dbfree(db_ret);
	db_ret = NULL;


OUT:
	dbfree(db_ret);
	wys_free(save_datas);
	wys_free(kvs);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 resave_usergroup_calc_action
/// @brief 重新同步用户组某天的行为记录
///
/// @param _in group_id->要保存的组ID
/// @param _in histroy_u64->重新同步的数据属于哪一天(GMT逆序)
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 resave_usergroup_calc_action(_s32 group_id, _u64 histroy_u64)
{
	struct list_head action_list = LIST_HEAD_INIT(action_list);
	struct hlist_head action_hash[HASH_SIZE];

	struct ww_action_list_st *action = NULL;

	_s32 i,j,ret = 0;
	DB* db_users = NULL;
	DB* db_ret = NULL;
	DB* db_group = NULL;
	_s32 uid = 0;
	_u32 gid = 0;

	_s8 key_buf[64] = {};
	_s32 key_len = 0;
	_s32 index = 0;
	_u32 user_cnt = 0;

	an_user_calc_acts_t* save_datas = NULL;
	an_user_calc_act_t* save_data = NULL;

	for(i = 0; i < HASH_SIZE; i++)
	{
		INIT_HLIST_HEAD(&action_hash[i]);
	}

	db_users = dbscan(PRE_USER_ID, strlen(PRE_USER_ID), PRE_USER_ID_SE, strlen(PRE_USER_ID_SE),DB_KVN_MAX);
	if(!db_users || db_users->rno != DB_SCAN_OK)
	{
		ret = -1;
		//b_len = sprintf(buf,"{\"err\":\"database err\"}");
		goto out;
	}
	
	for(i = 0; i < db_users->kvn; i++)
	{
		dbfree(db_ret);
		uid = BTOS32(db_users->kvs[i].val);
		key_len = snprintf(key_buf,sizeof(key_buf),PRE_USER_CALC_ACTION"%d-%llu",uid,histroy_u64);
		db_ret = dbget(key_buf,key_len);
		if(!db_ret || (db_ret->rno != DB_GET_OK && db_ret->rno != DB_NOT_FOUND))
		{
			ret = -1;
			goto out;
		}

		if(db_ret->rno == DB_NOT_FOUND || db_ret->kvn == 0)
		{
			key_len = snprintf(key_buf,sizeof(key_buf),PRE_USER_GROUP"%d",uid);
			db_group = dbget(key_buf,key_len);
			if(!db_group || db_group->rno != DB_GET_OK)
			{
				ret = -1;
				goto out;
			}
			gid = BTOS32(db_group->kvs[0].val);
			if(group_id != 0 && gid == group_id)
				user_cnt++;
			dbfree(db_group);
			db_group = NULL;

			continue;
		}

		save_datas = (an_user_calc_acts_t*)db_ret->kvs[0].val;
		if(group_id != 0 && save_datas->gid != group_id)
			continue;

		save_data = (an_user_calc_act_t*)save_datas->data;
	
		user_cnt++;
		INFO("uid = %d\n",uid);
		INFO("resave num = %d\n",save_datas->num);

		for(j = 0; j < save_datas->num; j++)
		{
			action = ww_action_st_hash_name_type_find_create(action_hash, save_data->pro_name, save_data->act_name, save_data->type);
			if(!action)
			{
				ret = -1;
				goto out;
			}

			if(action->type == 0)
			{
				snprintf(action->pro_name, sizeof(action->pro_name), "%s", save_data->pro_name);
				snprintf(action->act_name, sizeof(action->act_name), "%s", save_data->act_name);
				action->type = save_data->type;
				action->time = 0;
				action->category = (save_data->category == AN_CALC_ATTEMPT)?1:0;
				list_add_tail(&action->action_list,&action_list);
				if(action->type == OT_BROWSER_CLOSE_URL)
					index = ww_action_name_hash(action->act_name);
				else if(action->type == OT_WINDOW_CLOSE)
					index = ww_action_name_hash(action->pro_name);
				hlist_add_head(&action->name_hash,&action_hash[index]);
			}
			action->time += save_data->time;
			action->key_click += save_data->key_click;
			action->mou_click += save_data->mou_click;

			save_data++;
		}
	}

	if(ww_save_group_calc_act_to_db(group_id,&action_list,histroy_u64,user_cnt))
	{
		ret = -1;
		goto out;
	}

out:
	dbfree(db_users);
	dbfree(db_ret);
	dbfree(db_group);
	ww_user_action_list_del(&action_list);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 ww_usergroup_calc_action_save_check
/// @brief 检查用户组是否需要重新同步某天数据
///
/// @param _in today_time->当天00:00:00的GMT时间(逆序)
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_usergroup_calc_action_save_check(_u64 today_time) // time 为逆序
{
	struct list_head *pos = NULL;
	ww_group_base_info_t *temp = NULL;
	_s32 ret = 0;

	_s8 key_buf[64];
	_s32 key_len = 0;
	DB* db_ret = NULL;

	_u64 histroy_u64 = 0;

	list_for_each(pos, &an_group_list)
	{
		temp = list_entry(pos, ww_group_base_info_t, list);
		
		key_len = snprintf(key_buf,sizeof(key_buf),PRE_USERGROUP_CALC_ACTTIME"%d",temp->gid);
		db_ret = dbget(key_buf, key_len);
		if(!db_ret || (db_ret->rno != DB_GET_OK && db_ret->rno != DB_NOT_FOUND))
		{
			ret = -1;
			goto out;
		}
		
		if(db_ret->rno == DB_GET_OK && db_ret->kvn)
			histroy_u64 = BTOU64(db_ret->kvs[0].val);

		if(histroy_u64 <= today_time)
			histroy_u64 = (today_time + 2*24*3600);
		
		if(histroy_u64 > today_time)
		{
			while(histroy_u64 - today_time > 24*3600)
			{
				CHECK(22);
				histroy_u64 -= 24*3600;
				if(resave_usergroup_calc_action(temp->gid, histroy_u64))
				{
					ret = -1;
					goto out;
				}
			}
		}
		dbfree(db_ret);
		db_ret = NULL;
	}

	//此处为check全部用户组的
	key_len = snprintf(key_buf,sizeof(key_buf),PRE_USERGROUP_CALC_ACTTIME"%d",0);
	db_ret = dbget(key_buf, key_len);
	if(!db_ret || (db_ret->rno != DB_GET_OK && db_ret->rno != DB_NOT_FOUND))
	{
		ret = -1;
		goto out;
	}
	
	if(db_ret->rno == DB_GET_OK && db_ret->kvn)
		histroy_u64 = BTOU64(db_ret->kvs[0].val);
	
	if(histroy_u64 <= today_time)
		histroy_u64 = (today_time + 2*24*3600);
	
	if(histroy_u64 > today_time)
	{
		while(histroy_u64 - today_time > 24*3600)
		{
			CHECK(22);
			histroy_u64 -= 24*3600;
			if(resave_usergroup_calc_action(0, histroy_u64))
			{
				ret = -1;
				goto out;
			}
		}
	}
	dbfree(db_ret);
	db_ret = NULL;

out:
	dbfree(db_ret);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 resave_user_calc_action
/// @brief 重新同步某用户某天的行为记录
///
/// @param _in user_id->用户ID
/// @param _in group_id->用户当天所属组ID
/// @param _in histroy_u64->重新同步的数据属于哪一天(GMT逆序)
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 resave_user_calc_action(_u32 user_id, _u32 group_id, _u64 histroy_u64)
{
	_s32 ret = 0;
	DB* db_users = NULL;
	DB* db_ret = NULL;
	
	_s32 j;
	_u32 index = 0;
	
	struct ww_user_action_st * user = NULL;
	struct ww_action_list_st * action = NULL;

	_s8 scanstart[64] = {};
	_s8 scanend[64] = {};
	_s32 start_len = 0;
	_s32 end_len = 0;

	_s8* p_gmt = NULL;
	_u64 gmt_u64 = 0;
	_u64 end_gmt_u64 = 0;
	_u32 uid_hash_index = 0;

	_s8 temp_key[64] = {};
	_s32 temp_len = 0;
	DB* db_temp = NULL;
	_s8 temp_val[2048];
	_u64 tm = GMT_REVERS_MASK-histroy_u64;//正序历史时间

	action_st *user_action = NULL;
	browser_action_log_st *browser_action = NULL;
	wnd_swtich_log_st *wnd_swtich = NULL;
	
	user = ww_user_action_id_find_create(user_id);
	if(!user)
	{
		ret = -1;
		goto out;
	}
	if(user->user_id == 0)
	{
		user->user_id = user_id;
		uid_hash_index = ww_user_action_st_get_hash(user_id);
		hlist_add_head(&user->user_hash,&ww_user_hash[uid_hash_index]);
		list_add_tail(&user->user_list,&ww_user_list);
		list_add_tail(&user->group_list,&ww_user_group_list[group_id]);
	}
	user->group_id = group_id;
	user->start_time = histroy_u64 - 24*3600 + 1;
	user->day_time = histroy_u64;
	end_gmt_u64 = user->day_time;
	do{
		start_len = sprintf(scanstart,PRE_USER_ACTION"%d-%llu",user_id,gmt_u64?gmt_u64+1:user->start_time);
		end_len = sprintf(scanend,PRE_USER_ACTION"%d-%llu",user_id,end_gmt_u64?end_gmt_u64+1:histroy_u64);
		dbfree(db_ret);
		db_ret = dbscan(scanstart,start_len,scanend,end_len,DB_KVN_MAX);
		INFO("------start = %s\n------end = %s\n",scanstart,scanend);
		if(!db_ret)
		{
			ret = -1;
			//b_len = sprintf(buf,"{\"err\":\"database err\"}");
			goto out;
		}

		if(db_ret->rno != DB_SCAN_OK)
		{
			ret = -1;
			//b_len = sprintf(buf,"{\"err\":\"%s\"}",dberr(db_ret->rno));
			goto out;
		}

		if(db_ret->kvn)
		{
			p_gmt = (db_ret->kvs[db_ret->kvn-1].key + strlen(PRE_USER_ACTION) + USERID_LEN);
			gmt_u64 = atoull(p_gmt);
			//if(user->start_time == 0 || user->start_time > start_gmt_u64)
			//	user->start_time = start_gmt_u64;
		}
		
		for(j = 0; j < db_ret->kvn; j++)
		{
			user_action = (action_st *)(db_ret->kvs[j].val);
			if(ntohl(user_action->type) != OT_BROWSER_CLOSE_URL && ntohl(user_action->type) != OT_WINDOW_CLOSE)
				continue;
			
			if(user_action->data_len)
			{
				//窗口相关
				if(ntohl(user_action->type) == OT_WINDOW_CLOSE)
				{
					wnd_swtich = (wnd_swtich_log_st*)user_action->data;
					
					action = ww_action_st_user_name_type_find_create(user
									,user_action->process,NULL,OT_WINDOW_CLOSE,ntohl(user_action->category));
					if(!action)
					{
						ret = -1;
						goto out;
					}
					if(action && action->type == 0)
					{
						strncpy(action->act_name,wnd_swtich->data,sizeof(action->act_name));
						strncpy(action->pro_name,user_action->process,sizeof(action->pro_name));
						action->type = OT_WINDOW_CLOSE;
						action->time = 0;
						//scan出的第一个为准，即当天最后一个
						action->category = ntohl(user_action->category);
						list_add_tail(&action->action_list,&user->action_list);
						index = ww_action_name_hash(action->pro_name);
						hlist_add_head(&action->name_hash,&user->action_hash_table[index]);
					}
					if(ntohll(user_action->end_time) > ntohll(user_action->start_time))
					{
						//如果此行为记录时间跨了一天(可记录未关电脑的情况)
						//进行特殊处理
						if(ntohll(user_action->start_time) < tm)
						{
							action->time += (ntohll(user_action->end_time)-tm);
							//如果昨天的数据已保存,那么将此跨天的数据整合进昨天的数据中
							if(user->day_time == GMT_REVERS_MASK-tm)
							{
								if(resave_out_time_calc_act_to_user(user_action,user_id,group_id))
								{
									ret = -1;
									goto out;
								}
								if(resave_out_time_calc_act_to_group(user_action,group_id))
								{
									ret = -1;
									goto out;
								}
								if(resave_out_time_calc_act_to_group(user_action,0))
								{
									ret = -1;
									goto out;
								}
								//存完后修改原始数据
								temp_len = snprintf(temp_key, 64, "%s", db_ret->kvs[j].key);
								user_action->start_time = htonll(tm);
								memcpy(temp_val,db_ret->kvs[j].val,db_ret->kvs[j].val_len>sizeof(temp_val)?sizeof(temp_val):db_ret->kvs[j].val_len);
								db_temp = dbset(temp_key, temp_len, temp_val, db_ret->kvs[j].val_len>sizeof(temp_val)?sizeof(temp_val):db_ret->kvs[j].val_len);
								if(!db_temp || db_temp->rno != DB_SET_OK)
								{
									ret = -1;
									goto out;
								}
								dbfree(db_temp);
								db_temp = NULL;
							}
						}
						else
							action->time += (ntohll(user_action->end_time) - ntohll(user_action->start_time));
					}
					action->key_click += ntohl(user_action->key_click);
					action->mou_click += ntohl(user_action->mou_click);
					
				}
				//浏览器相关
				else if(ntohl(user_action->type) == OT_BROWSER_CLOSE_URL)
				{
					browser_action = (browser_action_log_st*)user_action->data;

					action = ww_action_st_user_name_type_find_create(user
									,NULL,browser_action->data,OT_BROWSER_CLOSE_URL,ntohl(user_action->category));
					if(!action)
					{
						ret = -1;
						goto out;
					}
					if(action && action->type == 0)
					{
						strncpy(action->act_name,browser_action->data,sizeof(action->act_name));
						strncpy(action->pro_name,user_action->process,sizeof(action->pro_name));
						action->type = OT_BROWSER_CLOSE_URL;
						action->time = 0;
						list_add_tail(&action->action_list,&user->action_list);
						index = ww_action_name_hash(action->act_name);
						hlist_add_head(&action->name_hash,&user->action_hash_table[index]);
					}
					if(ntohll(user_action->end_time) > ntohll(user_action->start_time))
					{
						//如果此行为记录时间跨了一天(可记录未关电脑的情况)
						//进行特殊处理
						if(ntohll(user_action->start_time) < tm)
						{
							action->time += (ntohll(user_action->end_time)-tm);
							//如果昨天的数据已保存,那么将此跨天的数据整合进昨天的数据中
							if(user->day_time == GMT_REVERS_MASK-tm)
							{
								if(resave_out_time_calc_act_to_user(user_action,user_id,group_id))
								{
									ret = -1;
									goto out;
								}
								if(resave_out_time_calc_act_to_group(user_action,group_id))
								{
									ret = -1;
									goto out;
								}
								if(resave_out_time_calc_act_to_group(user_action,0))
								{
									ret = -1;
									goto out;
								}
								//存完后修改原始数据
								temp_len = snprintf(temp_key, 64, "%s", db_ret->kvs[j].key);
								user_action->start_time = htonll(tm);
								memcpy(temp_val,db_ret->kvs[j].val,db_ret->kvs[j].val_len>sizeof(temp_val)?sizeof(temp_val):db_ret->kvs[j].val_len);
								db_temp = dbset(temp_key, temp_len, temp_val, db_ret->kvs[j].val_len>sizeof(temp_val)?sizeof(temp_val):db_ret->kvs[j].val_len);
								if(!db_temp || db_temp->rno != DB_SET_OK)
								{
									ret = -1;
									goto out;
								}
								dbfree(db_temp);
								db_temp = NULL;
							}
						}
						else
							action->time += (ntohll(user_action->end_time) - ntohll(user_action->start_time));
					}
					action->key_click += ntohl(user_action->key_click);
					action->mou_click += ntohl(user_action->mou_click);
				}
			}
		}
	}while(db_ret && db_ret->rno == DB_SCAN_OK && db_ret->kvn);
	if(!db_ret && db_ret->rno != DB_SCAN_OK)
	{
		ret = -1;
		goto out;
	}
	else
	{
		if(gmt_u64 < user->end_time)
			user->end_time = gmt_u64;
	}

	//保存至数据库中
	if(ww_save_user_calc_act_to_db(user))
	{
		ret = -1;
		goto out;
	}
	
out:
	dbfree(db_users);
	dbfree(db_ret);
	ww_user_action_st_del(user);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 ww_user_calc_action_save_check
/// @brief 检查全部用户行为统计记录情况,异常则重新记录
///
/// @param void->无参
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_user_calc_action_save_check(void)
{
	_s32 ret = 0;
	DB* db_users = NULL;
	DB* db_ret = NULL;

	DB* db_acttime = NULL;
	_s8 key_acttime[64] = {};
	_s32 key_actlen = 0;
	_u64 histroy_u64 = 0;
	
	_s32 i;
	_u32 user_id = 0;
	_s32 group_id = 0;

	_u64 today_time = time(NULL);	//(逆序) 当天00:00:00
	_u32 year,mon,day,hour,min,sec;
	an_gmt_to_time(today_time, &year, &mon, &day, &hour, &min, &sec);
	today_time -= (hour*3600 + min*60 + sec);
	today_time = GMT_REVERS_MASK-today_time;
	
	db_users = dbscan(PRE_USER_GROUP, strlen(PRE_USER_GROUP), PRE_USER_GROUP_SE, strlen(PRE_USER_GROUP_SE),DB_KVN_MAX);
	if(!db_users)
	{
		ret = -1;
		//b_len = sprintf(buf,"{\"err\":\"database err\"}");
		goto out;
	}
	
	if(db_users->rno != DB_SCAN_OK)
	{
		ret = -1;
		//b_len = sprintf(buf,"{\"err\":\"%s\"}",dberr(db_users->rno));
		goto out;
	}
	
	for(i = 0; i < db_users->kvn; i++)
	{
		user_id = (_u32)atoi(db_users->kvs[i].key + strlen(PRE_USER_GROUP));
		group_id = BTOS32(db_users->kvs[i].val);
		
		key_actlen = snprintf(key_acttime,sizeof(key_acttime),PRE_USER_CALC_ACTTIME"%d",user_id);
		db_acttime = dbget(key_acttime, key_actlen);
		if(!db_acttime || (db_acttime->rno != DB_GET_OK && db_acttime->rno != DB_NOT_FOUND))
		{
			ret = -1;
			goto out;
		}

		if(db_acttime->rno == DB_GET_OK && db_acttime->kvn)
		{
			histroy_u64 = BTOU64(db_acttime->kvs[0].val);
			INFO("uid = %u, history_u64 = %llu\n",user_id, histroy_u64);
		}
		else
			histroy_u64 = (today_time + 2*24*3600);

		if(histroy_u64 > (today_time + 24*3600))
		{
			histroy_u64 = (today_time + 2*24*3600);//2016-10-31增加，仅重新处理两天前数据,再之前的数据直接无视
			
			while(histroy_u64 - today_time > 24*3600)
			{
				histroy_u64 -= 24*3600;
				//CHECK(1);
				if(resave_user_calc_action(user_id, group_id, histroy_u64))
				{
					ret = -1;
					goto out;
				}
			}
		}
		// TODO:下方操作有一定危险性，考虑其他解决方式
		//增加如果历史记录比当天时间还晚,(如今天10月10,记录已记录了10月12的情况)
		//将历史记录回滚，抛弃已记录的"未来数据"
		else if(histroy_u64 < today_time)
		{
			histroy_u64 = (today_time + 2*24*3600);
			
			while(histroy_u64 - today_time > 24*3600)
			{
				histroy_u64 -= 24*3600;
				//CHECK(1);
				if(resave_user_calc_action(user_id, group_id, histroy_u64))
				{
					ret = -1;
					goto out;
				}
			}
		}
		
		dbfree(db_acttime);
		db_acttime = NULL;
	}

	//全用户组检测
	if(ww_usergroup_calc_action_save_check(today_time))
	{
		ret = -1;
		goto out;
	}
	

out:
	dbfree(db_users);
	dbfree(db_ret);
	dbfree(db_acttime);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 ww_auto_save_syn_to_group
/// @brief 将用户行为统计数据同步至其用户组
///
/// @param _inout group->用户组数据
/// @param _in action->行为统计数据
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_auto_save_syn_to_group(struct ww_user_action_st * group, struct ww_action_list_st * action)
{
	struct ww_action_list_st * g_action = NULL;
	_s32 index = 0;

	g_action = ww_action_st_user_name_type_find_create(group
						, action->pro_name, action->act_name, action->type, action->category);
	if(!g_action)
	{
		return -1;
	}
	if(g_action->type == 0)
	{
		strncpy(g_action->act_name,action->act_name,sizeof(g_action->act_name));
		strncpy(g_action->pro_name,action->pro_name,sizeof(g_action->pro_name));
		g_action->type = action->type;
		g_action->time = 0;
		g_action->category = action->category;
		list_add_tail(&g_action->action_list,&group->action_list);
		if(g_action->type == OT_BROWSER_CLOSE_URL)
			index = ww_action_name_hash(g_action->act_name);
		else if(g_action->type == OT_WINDOW_CLOSE)
			index = ww_action_name_hash(g_action->pro_name);
		hlist_add_head(&g_action->name_hash,&group->action_hash_table[index]);
	}
	g_action->time += action->time;
	g_action->key_click += action->key_click;
	g_action->mou_click += action->mou_click;

	return 0;
}

//----------------------------------------------------------------
// 函数名称 ww_save_all_user_calc_act_to_db
/// @brief 常规存储全部用户行为统计数据
///
/// @param void->无参
/// @return -1->error,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_save_all_user_calc_act_to_db(void)
{
	enum{
		act_id
		,time_id
		,max_id
	};

	_s32 ret = 0;
	DB* db_ret = NULL;
	_s8 key_buf[max_id][64] = {};
	_s32 key_len = 0;

	struct ww_user_action_st * user = NULL;
	struct ww_action_list_st * action = NULL;
	struct list_head* user_pos = NULL;
	struct list_head* act_pos = NULL;
	an_user_calc_acts_t* save_datas = NULL;
	an_user_calc_act_t* save_data = NULL;

	struct ww_user_action_st * group = NULL;
	struct ww_user_action_st * group_all = NULL;

	_s32 act_num = 1024;//没意义，可扩充
	_s32 act_count = 0;

	_s32 base_save_len = sizeof(an_user_calc_acts_t) + act_num*sizeof(an_user_calc_act_t);

	db_kv* kvs = NULL;
	_u32 i = 0;

	for(i = 0; i < GROUP_ID_MAX; i++)
	{
		auto_save_user_group_user_cnt[i] = 0;
	}

	save_datas = (an_user_calc_acts_t*)wys_malloc(base_save_len);
	if(!save_datas)
	{
		ret = -1;
		goto OUT;
	}

	kvs = (db_kv*)wys_malloc(sizeof(db_kv)*max_id);
	if(!kvs)
	{
		ret = -1;
		goto OUT;
	}

	group_all = ww_user_action_gid_find_create(0);
	if(!group_all)
	{
		ret = -1;
		goto OUT;
	}
	group_all->group_id = 0;
	list_add_tail(&group_all->group_list, &auto_save_user_group_list);
	
	list_for_each(user_pos, &ww_user_list)
	{
		save_data = (an_user_calc_act_t*)save_datas->data;
		act_count = 0;
		user = list_entry(user_pos, struct ww_user_action_st, user_list);
		group = ww_user_action_gid_find_create(user->group_id);
		if(!group)
		{
			ret = -1;
			goto OUT;
		}
		if(group->group_id == 0)
		{
			group->group_id = user->group_id;
			group->day_time = user->day_time;
			//全部用户,同步最老的那个
			if(group_all->day_time < user->day_time)
				group_all->day_time = user->day_time;
			list_add_tail(&group->group_list, &auto_save_user_group_list);
		}
		auto_save_user_group_user_cnt[user->group_id]++;
		auto_save_user_group_user_cnt[0]++;
		
		list_for_each(act_pos, &user->action_list)
		{
			act_count++;
			action = list_entry(act_pos, struct ww_action_list_st, action_list);

			//同步至用户组
			ww_auto_save_syn_to_group(group,action);
			ww_auto_save_syn_to_group(group_all,action);

			if(act_count > act_num)
			{
				act_num += act_num;
				save_datas = wys_realloc(save_datas, sizeof(an_user_calc_acts_t) + act_num*sizeof(an_user_calc_act_t));
				if(save_datas == NULL)
				{
					ret = -1;
					goto OUT;
				}
				save_data = (an_user_calc_act_t*)save_datas->data;
				save_data += (act_count-1);
			}

			save_data->time = action->time;
			save_data->type = action->type;
			snprintf(save_data->pro_name,sizeof(action->pro_name),"%s",action->pro_name);
			snprintf(save_data->act_name,sizeof(action->act_name),"%s",action->act_name);
			save_data->key_click = action->key_click;
			save_data->mou_click = action->mou_click;
			save_data->category = ww_get_category_by_ww_action_list_st(action,user->group_id);
			save_data++;
		}
		save_datas->num = act_count;
		save_datas->gid = user->group_id;

		key_len = snprintf(key_buf[act_id],sizeof(key_buf[act_id]),PRE_USER_CALC_ACTION"%d-%llu"
							,user->user_id,user->day_time);
		kvs[act_id].key = key_buf[act_id];
		kvs[act_id].key_len = key_len;
		kvs[act_id].val = (_s8*)save_datas;
		kvs[act_id].val_len = sizeof(an_user_calc_acts_t) + act_count*sizeof(an_user_calc_act_t);

		key_len = snprintf(key_buf[time_id], sizeof(key_buf[time_id]), PRE_USER_CALC_ACTTIME"%d", user->user_id);
		kvs[time_id].key = key_buf[time_id];
		kvs[time_id].key_len = key_len;
		kvs[time_id].val = (_s8*)&user->day_time;
		kvs[time_id].val_len = sizeof(user->day_time);

		db_ret = dbmulti_set(kvs, max_id);
		if(!db_ret || db_ret->rno != DB_MULTI_SET_OK)
		{
			ret = -1;
			goto OUT;
		}

		dbfree(db_ret);
		db_ret = NULL;
	}

OUT:
	dbfree(db_ret);
	wys_free(save_datas);
	wys_free(kvs);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 ww_save_all_group_calc_act_to_db
/// @brief 常规存储全部用户组行为统计数据
///
/// @param void->无参
/// @return -1->error,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_save_all_group_calc_act_to_db(void)
{
	enum{
		act_id
		,time_id
		,max_id
	};

	_s32 ret = 0;
	DB* db_ret = NULL;
	_s8 key_buf[max_id][64] = {};
	_s32 key_len = 0;

	struct ww_user_action_st * group = NULL;
	struct ww_action_list_st * action = NULL;
	struct list_head* group_pos = NULL;
	struct list_head* act_pos = NULL;
	an_user_calc_acts_t* save_datas = NULL;
	an_user_calc_act_t* save_data = NULL;

	_s32 act_num = 1024;//没意义，可扩充
	_s32 act_count = 0;

	_s32 base_save_len = sizeof(an_user_calc_acts_t) + act_num*sizeof(an_user_calc_act_t);

	db_kv* kvs = NULL;

	save_datas = (an_user_calc_acts_t*)wys_malloc(base_save_len);
	if(!save_datas)
	{
		ret = -1;
		goto OUT;
	}

	kvs = (db_kv*)wys_malloc(sizeof(db_kv)*max_id);
	if(!kvs)
	{
		ret = -1;
		goto OUT;
	}
	
	list_for_each(group_pos, &auto_save_user_group_list)
	{
		save_data = (an_user_calc_act_t*)save_datas->data;
		act_count = 0;
		group = list_entry(group_pos, struct ww_user_action_st, group_list);
		list_for_each(act_pos, &group->action_list)
		{
			act_count++;
			action = list_entry(act_pos, struct ww_action_list_st, action_list);

			if(act_count > act_num)
			{
				act_num += act_num;
				save_datas = wys_realloc(save_datas, sizeof(an_user_calc_acts_t) + act_num*sizeof(an_user_calc_act_t));
				if(save_datas == NULL)
				{
					ret = -1;
					goto OUT;
				}
			}

			save_data->time = action->time;
			save_data->type = action->type;
			snprintf(save_data->pro_name,sizeof(action->pro_name),"%s",action->pro_name);
			snprintf(save_data->act_name,sizeof(action->act_name),"%s",action->act_name);
			save_data->key_click = action->key_click;
			save_data->mou_click = action->mou_click;
			save_data->category = ww_get_category_by_ww_action_list_st(action,group->group_id);
			save_data++;
		}
		save_datas->num = act_count;
		save_datas->gid = group->group_id;
		save_datas->user_cnt = auto_save_user_group_user_cnt[group->group_id];

		key_len = snprintf(key_buf[act_id],sizeof(key_buf[act_id]),PRE_USERGROUP_CALC_ACTION"%d-%llu"
							,group->group_id, group->day_time);
		kvs[act_id].key = key_buf[act_id];
		kvs[act_id].key_len = key_len;
		kvs[act_id].val = (_s8*)save_datas;
		kvs[act_id].val_len = sizeof(an_user_calc_acts_t) + act_count*sizeof(an_user_calc_act_t);

		key_len = snprintf(key_buf[time_id], sizeof(key_buf[time_id]), PRE_USERGROUP_CALC_ACTTIME"%d", group->group_id);
		kvs[time_id].key = key_buf[time_id];
		kvs[time_id].key_len = key_len;
		kvs[time_id].val = (_s8*)&group->day_time;
		kvs[time_id].val_len = sizeof(group->day_time);

		db_ret = dbmulti_set(kvs, max_id);
		if(!db_ret || db_ret->rno != DB_MULTI_SET_OK)
		{
			ret = -1;
			goto OUT;
		}

		dbfree(db_ret);
		db_ret = NULL;
	}

OUT:
	dbfree(db_ret);
	wys_free(save_datas);
	wys_free(kvs);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 ww_del_all_dynamic_data
/// @brief 常规删除全部原始用户行为日志数据
///
/// @param gmt_u64->删除具体时间之前的数据(正序)
/// @return 0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_del_all_dynamic_data(_u64 gmt_u64)
{
	_u64 tm = GMT_REVERS_MASK - gmt_u64;

	struct ww_user_action_st * user = NULL;
	struct list_head* pos = NULL;

	list_for_each(pos, &ww_user_list)
	{
		user = list_entry(pos, struct ww_user_action_st, user_list);
	
		ww_del_user_db_data_by_gmt_uid_prefix(tm, user->user_id, PRE_USER_ACTION);
		ww_del_user_db_data_by_gmt_uid_prefix(tm, user->user_id, PRE_USER_QQ_MESSAGE);
	}

	return 0;
}

//----------------------------------------------------------------
// 函数名称 ww_time_init
/// @brief 检测并同步系统时间(根据用户行为日志记录)
///
/// @param void->无参
/// @return 0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_time_init(void)
{
	_s32 ret = 0;
	DB* db_ret = NULL;
	DB* db_users = NULL;
	time_t tm;
	_u64 gmt_u64 = 0;
	_u32 uid = 0;
	_u8 state = 0;
	_u32 start_len = 0, end_len = 0;
	_s8 scanstart[64] = {0};
	_s8 scanend[64] = {0};
	_u32 i = 0, j = 0;
	_u64 time_arr[DB_SCAN_MAX] = {0};
	_u32 time_flag[DB_SCAN_MAX] = {0};
	_u32 find_num = 0;
	_s32 online_flag = 0;
	//先获取时间，判断本机时间是否同步至最新日期
	tm = time(NULL);

	//未同步时，将日期同步最新用户行为记录的日期
	//↑最好修改成同步路由器时间
	db_users = dbscan(PRE_USER_STAT, strlen(PRE_USER_STAT), PRE_USER_STAT_SE, strlen(PRE_USER_STAT_SE),DB_SCAN_MAX);
	if (!db_users || db_users->rno != DB_SCAN_OK)
	{
		goto OUT;
	}

	if(db_users->kvn == 0)
		goto OUT;

	for(i = 0; i < db_users->kvn; i++)
	{
		state = BTOU8(db_users->kvs[i].val);
		uid = atoi(db_users->kvs[i].key + strlen(PRE_USER_STAT));
		if(state == 1)
		{
			online_flag = 1;
			start_len = snprintf(scanstart, sizeof(scanstart), PRE_USER_ACTION"%u-", uid);
			end_len = snprintf(scanend, sizeof(scanend), PRE_USER_ACTION"%u.", uid);
			dbfree(db_ret);
			db_ret = dbscan(scanstart, start_len, scanend, end_len, 10);
			if(!db_ret || db_ret->rno != DB_SCAN_OK)
			{
				goto OUT;
			}

			if(db_ret->kvn == 0)
			{
				goto OUT;
			}

			gmt_u64 = atoull(db_ret->kvs[0].key + strlen(PRE_USER_ACTION) + USERID_LEN);
			time_arr[i] = (GMT_REVERS_MASK-gmt_u64);
		}
	}

	if(online_flag == 0)
		goto OUT;
	
	//剔除异常用户时间
	for(i = 0; i < db_users->kvn; i++)
	{
		for(j = i+1; j < db_users->kvn; j++)
		{
			if(time_arr[i] == 0)
				time_flag[i]++;
			if(time_arr[j] == 0)
				time_flag[j]++;
			if((time_arr[i] > time_arr[j] + 3600) || (time_arr[i] < time_arr[j] - 3600))
			{
				time_flag[i]++;
				time_flag[j]++;
			}
		}
	}

	//筛选出要同步的时间
	find_num = 0;
	for(i = 0; i < db_users->kvn; i++)
	{
		if(time_flag[find_num] > time_flag[i])
			find_num = i;
	}

	//进行同步(需有守护进程,同步后重启)
	if(online_flag)
	{
		if(tm > (time_arr[find_num]+30*24*3600))//系统时间比该用户时间快30天以上
		{
			set_systime(time_arr[find_num]);
			INFO("sys time too early to syn time and restart\n");
			an_exit(__L__, 0);
		}
	
		if(tm < (time_arr[find_num]-3600))//系统时间比该用户时间慢1小时
		{
			set_systime(time_arr[find_num]);
			INFO("sys time too late to syn time and restart\n");
			an_exit(__L__, 0);
		}
	}

OUT:
	dbfree(db_ret);
	dbfree(db_users);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 ww_save_all_user_detail_act_to_db
/// @brief 常规保存全部用户行为记录
///
/// @param void->无参
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_save_all_user_detail_act_to_db(void)
{
	enum{
		act_id = AN_DETAIL_MAX-1
		,time_id
		,max_id
	};

	_s32 ret = 0;
	DB* db_ret = NULL;
	_s8 key_buf[max_id][64] = {};
	_s32 key_len = 0;

	an_detail_user_t* user = NULL;
	an_detail_act_t * action = NULL;
	an_detail_act_t ** del_action = NULL;
	an_detail_act_t ** pp_del = NULL;
	an_detail_act_t ** final_del = NULL;
	struct list_head* user_pos = NULL;
	struct list_head* act_pos = NULL;
	struct list_head* temp_pos = NULL;
	an_user_detail_acts_t* save_datas[AN_DETAIL_MAX] = {};
	an_user_detail_act_t* save_data[AN_DETAIL_MAX] = {};

	_u32 year,mon,day,hour,min,sec;

	_s32 act_num[AN_DETAIL_MAX] = {};
	_s32 act_count[AN_DETAIL_MAX] = {};
	_s32 act_num_max = MAX_RULE_NUM;
	_s32 act_all_num = 0;
	_s32 enum_id = 0;
	_s32 i = 0;
	_u64 save_time = 0;
	_u64 day_time = 0;

	_s32 base_save_len = sizeof(an_user_detail_acts_t) + act_num_max*sizeof(an_user_detail_act_t);

	db_kv* kvs = NULL;

	for(i = 0; i < AN_DETAIL_MAX; i++)
	{
		save_datas[i] = (an_user_detail_acts_t*)wys_malloc(base_save_len);
		if(!save_datas[i])
		{
			ret = -1;
			goto OUT;
		}

		act_num[i] = act_num_max;
	}

	del_action = (an_detail_act_t**)wys_malloc(act_num_max*sizeof(del_action));
	if(!del_action)
	{
		ret = -1;
		goto OUT;
	}
	pp_del = del_action;
	final_del = del_action;

	kvs = (db_kv*)wys_malloc(sizeof(db_kv)*max_id);
	if(!kvs)
	{
		ret = -1;
		goto OUT;
	}


	list_for_each(user_pos, &an_detail_user_list)
	{
		user = list_entry(user_pos, an_detail_user_t, user_list);
		while(user->end_time - user->start_time > 3600)
		{
			for(i = 0; i < AN_DETAIL_MAX; i++)
			{
				act_count[i] = 0;
				save_data[i] = (an_user_detail_act_t*)save_datas[i]->data;
			}
			act_all_num = 0;
			save_time = user->end_time - 3600 + 1;//当前小时59分59秒
			list_for_each_safe(act_pos, temp_pos, &user->action_list)
			{
				action = list_entry(act_pos, an_detail_act_t, list);
				//未统计的数据时间比记录的最后统计时间还晚
				//↑判断为垃圾数据，直接抛弃(这样的情况几乎不存在,保险起见还是判断一下)
				if(action->time > user->end_time)
				{
					an_detail_act_t_del(action);
					continue;
				}
				//仅同步当前小时的数据，其他数据暂时无视
				if(action->time < save_time)
					continue;

				enum_id = action->category;

				*del_action = action;
				del_action++;
				
				act_count[enum_id]++;
				act_all_num++;
				if(act_count[enum_id] > act_num[enum_id])
				{
					act_num[enum_id] += act_num[enum_id];
					save_datas[enum_id] = wys_realloc(save_datas[enum_id]
									, sizeof(an_user_detail_acts_t) + act_num[enum_id]*sizeof(an_user_detail_act_t));
					if(save_datas[enum_id] == NULL)
					{
						ret = -1;
						goto OUT;
					}
					save_data[enum_id] = (an_user_detail_act_t*)save_datas[enum_id]->data;
					save_data[enum_id] += act_count[enum_id]-1;
				}
				
				if(act_all_num > act_num_max)
				{
					act_num_max += act_num_max;
					del_action = wys_realloc(del_action, act_num_max*sizeof(del_action));
					if(del_action == NULL)
					{
						ret = -1;
						goto OUT;
					}
					pp_del = del_action;
					final_del = del_action;
					del_action += act_all_num;
				}

				save_data[enum_id]->time = action->time;
				save_data[enum_id]->type = action->type;
				snprintf(save_data[enum_id]->process,sizeof(action->process),"%s",action->process);
				snprintf(save_data[enum_id]->actname,sizeof(action->actname),"%s",action->actname);
				save_data[enum_id]->category = action->category;
				save_data[enum_id]++;
			}

			day_time = an_gmt_format_to_day_inver(user->end_time);
			an_gmt_to_time(GMT_REVERS_MASK-user->end_time, &year, &mon, &day, &hour, &min, &sec);

			for(i = 0; i < AN_DETAIL_MAX; i++)
			{
				save_datas[i]->num = act_count[i];
				save_datas[i]->gid = user->gid;
				key_len = snprintf(key_buf[i],sizeof(key_buf[act_id]),PRE_USER_DETAIL_ACTION"%d-%d-%llu-%u"
									,user->uid,i,day_time,hour);
				kvs[i].key = key_buf[i];
				kvs[i].key_len = key_len;
				kvs[i].val = (_s8*)save_datas[i];
				kvs[i].val_len = sizeof(an_user_detail_acts_t) + act_count[i]*sizeof(an_user_detail_act_t);
			}

			key_len = snprintf(key_buf[time_id], sizeof(key_buf[time_id]), PRE_USER_DETAIL_ACTTIME"%d", user->uid);
			kvs[time_id].key = key_buf[time_id];
			kvs[time_id].key_len = key_len;
			kvs[time_id].val = (_s8*)&user->end_time;
			if(user->end_time == 0)
			{
				ret = -1;
				goto OUT;
			}
			kvs[time_id].val_len = sizeof(user->end_time);

			db_ret = dbmulti_set(kvs, max_id);
			if(!db_ret || db_ret->rno != DB_MULTI_SET_OK)
			{
				ret = -1;
				goto OUT;
			}

			dbfree(db_ret);
			db_ret = NULL;

			//成功后的处理
			user->end_time -= 3600;
			for(i = 0; i < act_all_num; i++)
			{
				an_detail_act_t_del(*pp_del);
				*pp_del = NULL;
				pp_del++;
			}
			pp_del = final_del;
			del_action = final_del;
		}
	}

OUT:
	dbfree(db_ret);
	for(i = 0; i < AN_DETAIL_MAX; i++)
	{
		wys_free(save_datas[i]);
	}
	wys_free(kvs);
	wys_free(final_del);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 ww_save_all_user_qq_msg_to_db
/// @brief 常规保存全部用户聊天记录
///
/// @param void->无参
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s32 ww_save_all_user_qq_msg_to_db(void)
{
	enum{
		act_id
		,time_id
		,max_id
	};

	_s32 ret = 0;
	DB* db_ret = NULL;
	DB* db_msg = NULL;
	_s8 key_buf[max_id][64] = {};
	_s32 key_len = 0;

	an_qq_user_t* user = NULL;
	an_qq_obj_t * obj = NULL;
	an_qq_msg_t * msg = NULL;
	an_qq_msg_t ** del_msg = NULL;
	an_qq_msg_t ** pp_del = NULL;
	an_qq_msg_t ** final_del = NULL;
	struct list_head* user_pos = NULL;
	struct list_head* obj_pos = NULL;
	struct list_head* msg_pos = NULL;
	an_qq_calc_msgs_t* save_datas = NULL;
	an_qq_calc_msg_t* save_data = NULL;

	an_qq_calc_msgtime_t msg_time_t = {};

	_s8* p_save_data = NULL;
	_s32 offset_len = 0;

	_u32 year,mon,day,hour,min,sec;

	_s32 act_num = MAX_RULE_NUM;
	_s32 base_buf_len = 102400;
	_s32 msg_count = 0;
	_s32 msg_save_len = 0;
	_s32 i = 0;
	_u64 save_time = 0;
	_u64 day_time = 0;

	_s32 base_save_len = sizeof(an_qq_calc_msgs_t) + base_buf_len;

	db_kv* kvs = NULL;

	save_datas = (an_qq_calc_msgs_t*)wys_malloc(base_save_len);
	if(!save_datas)
	{
		ret = -1;
		goto OUT;
	}

	del_msg = (an_qq_msg_t**)wys_malloc(act_num*sizeof(del_msg));
	if(!del_msg)
	{
		ret = -1;
		goto OUT;
	}
	pp_del = del_msg;
	final_del = del_msg;

	kvs = (db_kv*)wys_malloc(sizeof(db_kv)*max_id);
	if(!kvs)
	{
		ret = -1;
		goto OUT;
	}

	list_for_each(user_pos, &an_qq_user_list)
	{
		user = list_entry(user_pos, an_qq_user_t, user_list);
		while(user->end_time - user->start_time > 3600)
		{
			save_time = user->end_time - 3600 + 1;//当前小时59分59秒
			list_for_each(obj_pos, &user->qq_list)
			{
				obj = list_entry(obj_pos, an_qq_obj_t, qq_list);
				//仅同步当前小时的数据，其他数据暂时无视
				if(obj->end_time < save_time)
					continue;

				key_len = snprintf(key_buf[0],sizeof(key_buf[0]),PRE_USER_QQ_CALC_MSGTIME"%d-%d"
										,user->uid, obj->qq_account);
				db_msg = dbget(key_buf[0], key_len);
				if(!db_msg || (db_msg->rno != DB_GET_OK && db_msg->rno != DB_NOT_FOUND))
				{
					ret = -1;
					goto OUT;
				}

				if(db_msg->rno == DB_NOT_FOUND)
				{
					msg_time_t.account = obj->qq_account;
					msg_time_t.time = user->end_time;
					snprintf(msg_time_t.name, sizeof(msg_time_t.name), "%s", obj->name);
					msg_time_t.num = 0;
				}
				else if(db_msg->kvn)
				{
					memcpy(&msg_time_t,db_msg->kvs[0].val,db_msg->kvs[0].val_len);
					msg_time_t.account = obj->qq_account;
					msg_time_t.time = user->end_time;
					snprintf(msg_time_t.name, sizeof(msg_time_t.name), "%s", obj->name);
				}

				p_save_data = save_datas->data;
				save_data = (an_qq_calc_msg_t*)p_save_data;
				msg_save_len = 0;
				msg_count = 0;
				list_for_each(msg_pos, &obj->msg_list)
				{
					msg = list_entry(msg_pos, an_qq_msg_t, msg_list);
					if(msg->time < save_time)
						continue;

					*del_msg = msg;
					del_msg++;

					msg_count++;
					if(msg_count > act_num)
					{
						act_num += act_num;
						del_msg = wys_realloc(del_msg, act_num*sizeof(del_msg));
						if(del_msg == NULL)
						{
							ret = -1;
							goto OUT;
						}
						pp_del = del_msg;
						final_del = del_msg;
						del_msg += msg_count;
					}
					msg_save_len += msg->msg_len + sizeof(an_qq_calc_msg_t);
					if(msg_save_len > base_buf_len)
					{
						base_buf_len += base_buf_len;
						save_datas = wys_realloc(save_datas, sizeof(an_qq_calc_msgs_t) + base_buf_len);
						if(save_datas == NULL)
						{
							ret = -1;
							goto OUT;
						}
						p_save_data = save_datas->data + offset_len;
						save_data = (an_qq_calc_msg_t*)p_save_data;
					}

					save_data->issend = msg->issend;
					save_data->msg_len = msg->msg_len;
					save_data->time = msg->time;
					save_data->user_account = msg->user_account;
					snprintf(save_data->obj_name,sizeof(save_data->obj_name),"%s",msg->obj_name);
					snprintf(save_data->user_name,sizeof(save_data->user_name),"%s",msg->user_name);
					snprintf(save_data->data,save_data->msg_len,"%s",msg->data);

					if(save_datas->time == 0 || save_datas->time < save_data->time)
						save_datas->time = save_data->time;
					
					offset_len = msg_save_len;
					p_save_data = save_datas->data + offset_len;
					save_data = (an_qq_calc_msg_t*)p_save_data;
				}

				save_datas->num = msg_count;
				msg_time_t.num += msg_count;

				day_time = an_gmt_format_to_day_inver(user->end_time);
				an_gmt_to_time(GMT_REVERS_MASK-user->end_time, &year, &mon, &day, &hour, &min, &sec);

				key_len = snprintf(key_buf[act_id],sizeof(key_buf[act_id]),PRE_USER_QQ_CALC_MSG"%d-%u-%llu-%u"
									,user->uid,obj->qq_account,day_time,hour);
				kvs[act_id].key = key_buf[act_id];
				kvs[act_id].key_len = key_len;
				kvs[act_id].val = (_s8*)save_datas;
				kvs[act_id].val_len = sizeof(an_qq_calc_msgs_t) + msg_save_len;

				key_len = snprintf(key_buf[time_id], sizeof(key_buf[time_id]), PRE_USER_QQ_CALC_MSGTIME"%d-%u"
									, user->uid, obj->qq_account);
				kvs[time_id].key = key_buf[time_id];
				kvs[time_id].key_len = key_len;
				kvs[time_id].val = (_s8*)&msg_time_t;
				kvs[time_id].val_len = sizeof(msg_time_t);

				db_ret = dbmulti_set(kvs, max_id);
				if(!db_ret || db_ret->rno != DB_MULTI_SET_OK)
				{
					ret = -1;
					goto OUT;
				}

				dbfree(db_ret);
				db_ret = NULL;

				//成功后的处理
				obj->end_time = user->end_time;
				for(i = 0; i < msg_count; i++)
				{
					an_qq_msg_t_del(*pp_del);
					pp_del++;
				}
				obj->msg_num -= msg_count;
				pp_del = final_del;
				del_msg = final_del;
			}
			user->end_time -= 3600;
		}
	}

OUT:
	dbfree(db_ret);
	wys_free(save_datas);
	wys_free(kvs);
	wys_free(final_del);
	return ret;
}

//----------------------------------------------------------------
// 函数名称 ww_del_all_analyzer_data
/// @brief 常规删除全部analyzer处理后的数据
///
/// @param _in gmt_u64->删除多久以前的数据(正序GMT)
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void ww_del_all_analyzer_data(_u64 gmt_u64)
{
	_u64 tm = GMT_REVERS_MASK - gmt_u64;
	_s32 i = 0;

	struct ww_user_action_st * user = NULL;
	ww_group_base_info_t* group = NULL;
	struct list_head* pos = NULL;

	list_for_each(pos, &ww_user_list)
	{
		user = list_entry(pos, struct ww_user_action_st, user_list);

		for (i = 0; i < AN_DETAIL_MAX; i++)
		{
			ww_del_user_db_data_by_uid_type_gmt_prefix(tm, user->user_id, PRE_USER_DETAIL_ACTION, i);
		}
		ww_del_user_db_data_by_gmt_uid_prefix(tm, user->user_id, PRE_USER_CALC_ACTION);
	}

	ww_del_group_db_data_by_gmt_gid_prefix(tm, 0, PRE_USERGROUP_CALC_ACTION);
	list_for_each(pos, &an_group_list)
	{
		group = list_entry(pos, ww_group_base_info_t, list);

		ww_del_group_db_data_by_gmt_gid_prefix(tm, group->gid, PRE_USERGROUP_CALC_ACTION);
	}
	ww_del_user_qq_data_by_gmt_uid_prefix(tm, user->user_id, PRE_USER_QQ_CALC_MSGTIME);

	return;
}

//----------------------------------------------------------------
// 函数名称 an_auto_data_save
/// @brief 检测更新保存所有analyzer缓存数据
///
/// @param _inout wk->包含记录时间信息与返回给worker的信息
/// @return WW_LOCAL_ACTION->值为100(表示成功),-1->error
/// @note 这个操作由worker调用(会影响用户访问网页)
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_auto_data_save(wk_task_t* wk)
{
	wk_local_task_t* local_task = (wk_local_task_t*)wk->data;
	struct an_auto_data_save_st* data_save = local_task->data_save;
	
	_u32 year,mon,day,hour,min,sec;
	time_t tm;

	//_u32 save_flag = 0;

	//先获取时间，判断本机时间是否同步至最新日期
	tm = time(NULL);

	//未同步时，将日期同步最新用户行为记录的日期
	//↑最好修改成同步路由器时间
	ww_time_init();

	an_gmt_to_time(tm,&year,&mon,&day,&hour,&min,&sec);

	if(data_save->day_time == 0)
	{
		data_save->day_time = (tm - hour*3600 - min*60 - sec);
		data_save->mday = day;
	}

	//需要做一个昨天的消息记录是否已统筹保存的判断
	//↑由wk_init时，进行相关处理工作。
	//如果时间完全同步，可考虑0点就保存(意义不大)(越接近0点，保存效率越高，重新初始化的数据没那么多)
	if(hour >= 3 && data_save->mday != day && tm > data_save->day_time)
	{
		//统筹记录当天数据
		//1单个用户当天的行为统计
		INFO("time 1 \n");
		if(ww_save_all_user_calc_act_to_db())
			goto ERR;
		//各组(gid 0 为全部用户组)当天的行为统计
		INFO("time 2 \n");
		if(ww_save_all_group_calc_act_to_db())
			goto ERR;

		//统计并储存行为记录(这里是否记录影响不大)
		INFO("time 3 \n");
		ww_save_all_user_detail_act_to_db();
		//统计并存储QQ聊天记录(同上意义不大)
		INFO("time 4 \n");
		ww_save_all_user_qq_msg_to_db();

		//成功后删除之前的dyua 及 dyuq等记录
		//删除失败没关系，下次继续删
		INFO("time 5 \n");
		ww_del_all_dynamic_data((tm - hour*3600 - min*60 - sec));
		//删除半年以前的统计数据(谨慎操作)
		// TODO:此处应该根据硬盘大小来修改删除时间
		// TODO:用户自己设置的话，需要自适应空间
		INFO("time 6 \n");
		ww_del_all_analyzer_data((tm - hour*3600 - min*60 - sec)-180*24*3600);

		//清空链表数据
		INFO("time 7 \n");
		ww_user_action_st_clear();
		ww_user_action_st_group_clear();
		an_detail_user_t_clear();
		an_qq_user_t_clear();
		
		//重新初始化链表
		INFO("time 8 \n");
		ww_add_or_init_calc_behavior_to_list();
		ww_add_or_init_detail_behavior_to_list();
		ww_add_or_init_qq_msg_to_list(1);

		//给时间赋值表示到了下一天
		INFO("time 9 \n");
		data_save->day_time = (tm - hour*3600 - min*60 - sec);
		data_save->mday = day;

		//重新初始化一些相关动态JS
		if(ww_init_ww_month_action_js())
			goto ERR;

		if(ww_init_ww_high_action_js())
			goto ERR;

		if(ww_detail_act_list_init())
			goto ERR;
		
	}
	else
	{
		//正常每10分钟的更新
		// TODO:需操作失败的判断(特殊错误结束进程(会自动重启)，其他错误忽视)
		INFO("time 10 \n");
		ww_add_or_init_calc_behavior_to_list();
		//先更新，然后保存(会判断是否需要保存)
		INFO("time 11 \n");
		ww_add_or_init_detail_behavior_to_list();
		INFO("time 111 \n");
		ww_save_all_user_detail_act_to_db();
		//更新并统计聊天记录
		INFO("time 12 \n");
		ww_add_or_init_qq_msg_to_list(0);
		INFO("time 13 \n");
		ww_save_all_user_qq_msg_to_db();
		
	}


	return WW_LOCAL_ACTION;
ERR:
	//统计出错，重新统计 (程序退出重启)
	an_exit(__L__, 0);

	return -1;
}

//----------------------------------------------------------------
// 函数名称 an_auto_data_save_timeout
/// @brief 添加更新保存任务,并更新timer
///
/// @param _inout data->包含记录时间信息
/// @return void->无返回值
/// @see an_auto_data_save auto_data_save.c
/// @author miaomiao
//---------------------------------------------------------------
void an_auto_data_save_timeout(unsigned long data)
{
	struct an_auto_data_save_st* data_save = (struct an_auto_data_save_st*)data;
	wk_local_task_t* local_task = NULL;
	wk_task_t* wk = wys_malloc(sizeof(wk_task_t)+sizeof(wk_local_task_t));
	if(!wk)
	{
		goto OUT;
	}
	
	local_task = (wk_local_task_t*)wk->data;
	wk->head.cmd = LOCAL_AZ_AUTO_SAVE;
	local_task->data_save = data_save;
	wk_add_task(wk);

OUT:
	mod_timer(&data_save->timeout, jiffies + 600*HZ);
	return;
}

// TODO:必要的话，这个结构体内存改为全局变量，可随时调整timer达到想更新就更新的效果
//----------------------------------------------------------------
// 函数名称 an_data_save_init
/// @brief 初始化自动更新保存的结构体
///
/// @param void->无参
/// @return -1->failed,0->sucess
/// @see an_auto_data_save_timeout auto_data_save.c
/// @author miaomiao
//---------------------------------------------------------------
_s32 an_data_save_init()
{
	//创建timer|用以处理AP离线日志信息
    int err = 0;
	struct an_auto_data_save_st* data_save = NULL;
	data_save = (struct an_auto_data_save_st*)wys_malloc(sizeof(*data_save));
	if(!data_save)
	{
		DEBUG("an data save init failed");
		err = -1;
	}

	init_timer(&data_save->timeout);
	data_save->timeout.data = (unsigned long)data_save;
	data_save->timeout.function = an_auto_data_save_timeout;
	data_save->timeout.expires = jiffies + 60*HZ;
	add_timer(&data_save->timeout);
	
	return err;
}

