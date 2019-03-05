/////////////////////////////////////////////////////////////////////
/// @file auto_data_save.h
/// @brief 提供将analyzer缓存与数据库交互相关接口,如保存/删除等
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#ifndef _AUTO_DATA_SAVE_H
#define _AUTO_DATA_SAVE_H "auto_data_save.h"
#include <sys/time.h> 
#include <timer.h>

#include "types.h"
#include "worker.h"

/// @brief analyzer自动更新保存动态数据结构
///
struct an_auto_data_save_st
{
	_u64 mday;					///< 当月几号
	_u64 day_time;				///< 今天的时间(00:00)正序GMT
	
	struct timer_list timeout;	///< timer
};

/// @brief 根据用户ID,时间,指定头清理数据库中数据
_s32 ww_del_user_db_data_by_gmt_uid_prefix(_u64 gmt_u64, _u32 uid, _s8* prefix);
/// @brief 根据用户ID,类型,时间,指定头清理数据库中数据
_s32 ww_del_user_db_data_by_uid_type_gmt_prefix(_u64 gmt_u64, _u32 uid, _s8* prefix, _u32 type);
/// @brief 根据用户ID,时间,指定头清理数据库中聊天统计数据
_s32 ww_del_user_qq_data_by_gmt_uid_prefix(_u64 gmt_u64, _u32 uid, _s8* prefix);
/// @brief 根据组ID,时间,指定头清理数据库中聊天统计数据
_s32 ww_del_group_db_data_by_gmt_gid_prefix(_u64 gmt_u64, _u32 gid, _s8* prefix);
/// @brief 将用户行为统计数据同步至其用户组
_s32 ww_auto_save_syn_to_group(struct ww_user_action_st * group, struct ww_action_list_st * action);
	
//每天凌晨3点将昨天的数据统计后存入数据库内

/// @brief 初始化自动更新保存的结构体
_s32 an_data_save_init(void);
/// @brief 检测更新保存所有analyzer缓存数据
_s32 an_auto_data_save(wk_task_t* data_save);
/// @brief 检查全部用户行为统计记录情况,异常则重新记录
_s32 ww_user_calc_action_save_check(void);
/// @brief 检测并同步系统时间(根据用户行为日志记录)
_s32 ww_time_init(void);

#endif // end of  "public.h"
