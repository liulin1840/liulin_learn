/////////////////////////////////////////////////////////////////////
/// @file worker.h
/// @brief 主要提供添加数据处理任务,数据初始化以及数据更新接口
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.10
/////////////////////////////////////////////////////////////////////
#ifndef _WORKER_H
#define _WORKER_H "worker.h"
#include "list.h"
#include "timer.h"
#include "types.h"
#include "head.h"
#include "ww_action.h"

#define USERID_LEN 6	  ///< 用户ID长度,用在需要偏移量的地方,5位数字+1位字符'-'
#define GROUP_ID_MAX 256  ///< 用户组最多允许创建数量
#define MAX_DAY_LEN 367
#define MAX_SAVE_DAY 180
#define MAX_USER_ID 1024  ///< 用户最多允许创建数量
#define MAX_RULE_NUM 1024 //无特殊意义(但修改需注意相关地方是否有冲突)

#define NORMAL_BUFFER_LENGTH 1024

#define HASH_SIZE 0x100
#define HASH_MASK 0xff
#define KEY_BUFFER_LENGTH_MAX  32    ///< 关键字缓冲区最大的尺寸
#define DETAIL_BUF_LENGTH_MAX  256    ///< 关键字缓冲区最大的尺寸
#define ENCRYPTION_FILTERS_NUMBER_MAX       1024 ///< 支持的最大文件加密规则数
#define GLOBLE_GROUP_ID       0   ///< 文件加密条件全局组的ID号

#define COOKIE_AGEING_TIME	3600	///< cookie老化时间

/// @brief worker模块处理函数特殊返回值枚举
///
/// @note 基本返回值:0 成功,-1 失败
enum WW_ERRNO
{
	WW_ERR_DATABASE = 10		///< 数据库操作错误
	,WW_ERR_MALLOC				///< 内存申请失败
	,WW_ERR_INVAILD_ID			///< 无效的ID(用户ID 组ID等)
	
	,WW_ERR_FROBID_OUT_OF_SYNC 	///< 违规行为hash和数据库不同步
	,WW_ERR_ENCRYPT_OUT_OF_SYNC ///< 文件加密规则hash和数据库不同步
	,WW_ERR_WORK_RULE_OUT_OF_SYNC ///< 办公行为规则hash和数据库不同步

	,WW_ERR_FULL_USER_NUM				///< 用户数量达到上限
	,WW_ERR_LESS_PARAM_NUM				///< 参数过少
	,WW_ERR_ACCOUNT_HAVE_CN_PARAM 		///< 账号含有中文字符
	,WW_ERR_PASSWORD_HAVE_CN_PARAM		///< 密码含有中文字符
	,WW_ERR_CREATE_GROUP_FAILED = 20 	///< 创建组失败
	,WW_ERR_FULL_GROUP_NUM				///< 用户组数量达到上限
	,WW_ERR_REMARK_TOO_LONG				///< 名称过长
	,WW_ERR_ACCOUNT_TOO_LONG			///< 账号过长
	,WW_ERR_PASSWORD_TOO_LONG			///< 密码过长
	,WW_ERR_GROUP_NAME_TOO_LONG 		///< 部门名过长
	,WW_ERR_HAVE_EMPTY_PARAM			///< 含有空参数
	,WW_ERR_ACCOUNT_EXIST				///< 账号已存在

	///以下为SEND操作
	,WW_JS_HIGH_ACT_GET  = 50	///< 获取ww_high_action.js
	,WW_JS_MONTH_ACT_GET		///< 获取ww_month_action.js
	,WW_JS_ALL_ACT_GET			///< 获取ww_all_action.js(此为动态变换js，不能缓存)
	,WW_JS_GROUP_ACT_GET		///< 获取ww_group_action.js(此为动态变换js，不能缓存)
	,WW_JS_USER_ACT_GET			///< 获取ww_user_action.js(此为动态变换js,不能缓存)

	,WW_CSV_DETAIL_ACT_GET = 100 ///< 获取water_wall_record.csv
	,WW_CSV_USER_EXPORT_BASEINFO_GET	///< 获取water_wall_user.csv
	,WW_CSV_USER_EXPORT_ERROR_GET	///< 获取ww_user_import_err.csv

	,WW_HTML_IMPORT_ERR_GET = 200 ///< 获取导入出错页面(数据异常)
	,WW_HTML_IMPORT_SUCESS_GET	  ///< 获取导入成功页面
	,WW_HTML_IMPORT_FAILED_GET	  ///< 获取导入失败页面(程序崩溃)
	
	,WW_LOCAL_ACTION = 100		///< 内部请求，无需发送消息给WEB服务器
};

#pragma pack(4)
typedef struct  ww_forbidact_s
{
	struct hlist_node h_nd;             ///< 哈希表
	struct list_head all_list;          ///< 所有规则链表
	struct list_head group_list;        ///< 用户组链表

	_u8     kind;		                ///< 关键字种类(1:域名,2:搜索关键字)                 参考 win_key_filter_t,url_key_filter_t;
	_u8     type;		                ///< 1:违规 2:禁止                                  参考 win_key_filter_t,url_key_filter_t;  1/2
	_s8     name[32];                   ///< 规则名
	_s32    gid;                        ///< 用户组ID
	_u8     act_type;                   ///< 规则类型
	_u8     is_forbid;                  ///< 是否禁止                                       打包用的；
	_u16    enable;                     ///< TRUE:启用该过滤器,FALSE:禁止该过滤器            参考app_ctrl_filter_t;
	_u16	option;                     ///< 1:禁止运行,2：禁止改名                          参考app_ctrl_filter_t; 1/3     按位或
	
}ww_forbidact_t;                        ///< 存在哈希表中的违规行为结构体

typedef struct ww_encryptfile_s
{
	struct hlist_node h_nd;             ///< 哈希表
	struct list_head all_list;          ///< 所有规则链表
	struct list_head group_list;        ///< 用户组链表

	_s8 	name[256];                      ///< 描述
	_s32 	gid;                           ///< 用户组ID
	_u8 	filetype;                       ///< 规则类型
	_u32	authority;                     ///<权限   默认全开
	_u16	enable;                        ///< 是否允许
	
}ww_encryptfile_t;                      ///<文件加密结构体

/// @brief 用户组基本信息结构
///
typedef struct ww_group_base_info_s
{
	struct list_head	list;		///< 链表节点 @see an_group_list[gid] hash.c
	struct hlist_node	hash;		///< hash节点(gid作为hash值) @see an_group_hash hash.c

	_s32 gid;				///< 组ID
	_s8  name[32];			///< 组名
	_u32 cd_auth;			///< CD访问权限 @see PRE_SET_CDACCESS prefix.h
	_u32 usb_auth;			///< USB访问权限 @see PRE_SET_USBACCESS prefix.h
	_u32 monitor;			///< 用户活动监视 @see PRE_USERGROUP_MONITOROPT	prefix.h
	_u32 fileauth;			///< 加密访问权限 @see PRE_USERGROUP_FILEAUTH prefix.h
	_u32 user_num;			///< 该组拥有的用户数量
}ww_group_base_info_t;

/// @brief 用户行为统计,具体行为信息结构
///
struct ww_action_list_st
{
	struct list_head action_list;	///< 关联行为链表(无固定关联,一般关联struct ww_user_action_st的action_list)
	struct hlist_node name_hash;	///< hash节点(窗口根据pro_name做hash,域名根据act_name)

	_s8 	act_name[48];			///< 行为描述/网址域名
	_s8 	pro_name[32];			///< 进程名
	_u32	time;					///< 持续时间
	_u32	type;					///< 行为类型  @see USER_ACTION_TYPE ww_action.h 
	_u32	key_click;				///< 键盘按键数
	_u32	mou_click;				///< 鼠标点击数
	_u32	category;				///< 消息类别  @see LOG_CATEGORY ww_action.h
};

/// @brief 用户行为统计,具体用户结构
///
struct ww_user_action_st
{
	struct list_head user_list;		///< 关联用户链表@see ww_user_list hash.c
	struct hlist_node user_hash;	///< 用户hash节点@see ww_user_hash hash.c
	struct list_head action_list;	///< 一般自己作为表头 (关联struct ww_action_list_st的action_list)
	struct list_head group_list;	///< 关联用户组链表@see ww_user_group_list[group_id] hash.c
	struct hlist_head action_hash_table[HASH_SIZE]; ///< 自己的行为统计hash表(关联struct ww_action_list_st的name_hash)

	_s32 	user_id;					///< 用户ID
	_s32 	group_id;					///< 所属组ID，每次获取data都要更新此值
	_u64	start_time;					///< 允许最近的查询时间(当天23:59:59逆序)
	_u64	end_time;					///< 最新查询数据所在时间(同数据库为逆序)
	_u64	day_time;					///< 最远追溯时间(当天00:00:00同数据库为逆序)
};

/// @brief 页面获取用户行为记录时,具体行为结构(比原始结构多了一些参数方便处理)
///
typedef struct an_detail_wk_act_s
{
	struct list_head list;		///< 关联单个用户的行为记录

	_u64	time;				///< 行为发生时间 (逆序)
	_u32	type;				///< 行为类型 @see USER_ACTION_TYPE ww_action.h 
	_u32	category;			///< 消息类别 @see auto_save_detail_act ww_action.h 
//	_u16	is_ciphertext;//是否与密文相关事件 -->抛弃此参数create的时候直接根据user_action修改category的值
//	_u16	pad;
	_u32 	uid;				///< 行为所属用户ID
	_s32 	gid;				///< 行为所属组ID
	_s8		process[32];		///< 与该事件相关进程名
	_s8		actname[128];		///< 事件相关名称(窗口名 url网址域名 文件路径等)
}an_detail_wk_act_t;

/// @brief 用户行为记录,具体行为结构(缓存中原始结构)
///
typedef struct an_detail_act_s
{
	struct list_head list;		///< 关联单个用户的行为记录

	_u64	time;				///< 行为发生时间 (逆序)
	_u32	type;				///< 行为类型 @see USER_ACTION_TYPE ww_action.h 
	_u32	category;			///< 消息类别 @see auto_save_detail_act ww_action.h 
//	_u16	is_ciphertext;// 是否与密文相关事件 -->抛弃此参数create的时候直接根据user_action修改category的值
//	_u16	pad;
	_s8		process[32];		///< 与该事件相关进程
	_s8		actname[128];		///< 事件相关名称(窗口名 url网址域名 文件路径等)
}an_detail_act_t;

/// @brief 用户行为记录,具体用户结构
///
typedef struct an_detail_user_s
{
	struct list_head user_list;		///< 关联所有用户 @see an_detail_user_list hash.c
	struct list_head group_list;	///< 当前用户组所有数据 @see an_detail_user_group_list[gid] hash.c
	struct list_head action_list;	///< 关联所有行为记录规则 (自己是表头) @see an_detail_act_t worker.h
	struct hlist_node hash;			///< hash节点(uid作为hash值) @see an_detail_user_hash hash.c

	_s32 	uid;					///< 用户ID
	_s32 	gid;					///< 获取该行为数据时所属的组ID
	_u64	start_time;				///< 最新数据所在时间 (逆序)
	_u64	end_time;				///< 最老数据所在时间 (逆序) ->保存为小时00:00  数据无序，保存剔除时需遍历所有 
}an_detail_user_t;

/// @brief 页面获取用户聊天记录时,具体单个消息结构(比原始结构多了一些参数方便处理)
///
typedef struct an_qq_wk_msg_s
{
	struct list_head msg_list;	///< 关联与对应uid的所有聊天记录
	struct hlist_node hash;		///< obj_account做hash

	_u64 time;					///< 消息产生时间 (逆序)
	_u32 user_account;			///< 用户所使用账号
	_s8  user_name[32];			///< 用户当时账号对应昵称
	_u32 obj_account;			///< 聊天对象当时使用账号
	_s8  obj_name[32];			///< 聊天对象当时使用的昵称
	_u32 msg_num;				///< 时间段内共有多少聊天记录
	_u32 issend;				///< 用户是是否发送方
	_u32 msg_len;				///< 消息长度
	_u32 uid;					///< 用户ID
	_s8  data[0];				///< 消息内容
}an_qq_wk_msg_t;

/// @brief 用户聊天记录,具体单个消息结构(缓存中原始结构)
///
typedef struct an_qq_msg_s
{
	struct list_head msg_list;	///< 关联与对应uid(用户ID)的所有聊天记录

	_u64 time;					///< 消息产生时间 (逆序)
	_u32 user_account;			///< 用户所使用账号
	_s8  user_name[32];			///< 用户当时账号对应昵称
	_s8  obj_name[32];			///< 聊天对象当时使用的昵称
	_u32 issend;				///< 用户是是否发送方
	_u32 msg_len;				///< 消息长度
	_s8  data[0];				///< 消息内容
}an_qq_msg_t;

/// @brief 用户聊天记录,具体聊天对象结构
///
typedef struct an_qq_obj_s
{
	struct list_head qq_list;		///< 关联对应uid(用户ID)所有聊天对象
	struct hlist_node hash;			///< qq_account做hash,关联对应uid(用户ID)所有聊天对象
	struct list_head msg_list;		///< 关联与对应uid(用户ID)的所有聊天记录

	_u32	qq_account;				///< 对象的QQ号 -->作为哈希值
	_s8		name[32];				///< QQ昵称 -->随时更新
	_u32	msg_num;				///< 总消息数量
	_u64	start_time;				///< 最新数据所在时间 (逆序) 小时为单位
	_u64	end_time;				///< 最老数据所在时间 (逆序) 小时为单位
}an_qq_obj_t;

/// @brief 用户聊天记录,具体用户结构
///
typedef struct an_qq_user_s
{
	struct list_head user_list;				///< 关联所有用户 @see an_qq_user_list hash.c
	struct list_head group_list;			///< 当前用户组所有数据 @see an_qq_user_group_list[gid] hash.c
	struct hlist_head qq_hash[HASH_SIZE];	///< 关联所有该用户所有聊天对象 @see an_qq_obj_t worker.h
	struct list_head qq_list;				///< 关联该用户所有聊天对象 @see an_qq_obj_t worker.h
	struct hlist_node user_hash;			///< uid做hash @see an_qq_user_hash hash.c

	_u32	uid;		///< 用户ID
	_s32 	gid;		///< 行为发生时所属gid(组ID)(不保存,获取数据时用户属于哪个组,就归类至哪个组)
	_u64	start_time; ///< 最新数据所在时间 (逆序)
	_u64	end_time;	///< 最老数据所在时间 (逆序) ->保存为小时00:00	数据无序，保存剔除时需遍历所有
}an_qq_user_t;

/// @brief 缓存中办公行为结构
///
typedef struct an_work_rule_s
{
	struct list_head rule_list;		///< 关联所有规则 @see an_work_rule_list hash.c
	struct hlist_node hash;			///< type 0->process做hash|type 1->workname做hash @see an_work_rule_hash hash.c
	struct list_head group_list;	///< 规则关联组 @see an_work_group_list[gid] hash.c

	_s8 	workname[48];	///< 行为描述(进程为空字符串,网址直接给域名)
	_s8 	process[32];	///< 进程名(网址没有)
	_s32	gid;			///< 生效组id
	_u8 	worktype;		///< 行为类型:0:使用软件,1:浏览网页
	_u8 	pad[3];			///< 预留字节(注意对齐)
}an_work_rule_t;

/// @brief 管理员页面查看的缓存列表
///
typedef struct an_web_cache_s
{
	struct list_head detail_act_list;	///< 行为记录链表
	struct list_head qq_msg_list;		///< QQ消息链表
	struct hlist_node hash;				///< 关联an_web_cache_hash,token作为哈希值

	_s32 token;							///< 网页cookie,作为管理员标识

	struct timer_list timeout;	///< timer
}an_web_cache_t;

#pragma pack()

/// @brief 处理各种数据处理请求(包括内部/外部请求)
void* wk_routine(void* parms);
/// @brief worker模块初始化函数,包括线程池,链表缓存等
_s32 wk_init(void);
/// @brief 添加处理任务至线程池wk_thread_pool,处理函数为wk_routine
_s32 wk_add_task(void* parms);

/// @brief 更新数据库中用户行为日志到行为统计缓存链表中(ww_user_list)
_s32 ww_add_or_init_calc_behavior_to_list(void);
/// @brief 更新数据库中用户行为日志到行为记录缓存链表中(an_detail_user_list)
_s32 ww_add_or_init_detail_behavior_to_list(void);
/// @brief 更新数据库中用户聊天日志到聊天记录缓存链表中(an_qq_user_list)
_s32 ww_add_or_init_qq_msg_to_list(_s32 init_flag);
/// @brief 初始化首页动态js(ww_month_action_js)
_s32 ww_init_ww_month_action_js(void);
/// @brief 初始化最近一个月用户行为统计至动态js缓存(ww_high_action_js)
_s32 ww_init_ww_high_action_js(void);
/// @brief 初始化gl_detail_all_act_head等行为记录缓存数据
_s32 ww_detail_act_list_init(void);

/// @brief 将未保存的跨天行为保存至指定用户的行为统计中(数据库)
_s32 resave_out_time_calc_act_to_user(action_st* user_action, _u32 user_id, _u32 group_id);
/// @brief 将未保存的跨天行为保存至指定用户组的行为统计中(数据库)
_s32 resave_out_time_calc_act_to_group(action_st* user_action, _u32 group_id);

/// @brief 同步缓存(an_work_rule_list)中的数据到数据库
_s32 syn_work_rule_list_to_db(void);

/// @brief 添加新用户组
_s32 wk_user_group_add_cmd_do(wk_task_t* wk);


#endif // end of "worker.h"
////