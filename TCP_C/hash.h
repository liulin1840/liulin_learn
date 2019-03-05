/////////////////////////////////////////////////////////////////////
/// @file hash.h
/// @brief 提供外部使用的hash操作及相关全局变量声明
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#ifndef HASH_H
#define HASH_H
#include "types.h"
#include "worker.h"
#include "ww_action.h"

#define list_commen_compare(list_one, list_two, type, key, list_name, return_val) \
({\
	type* one = list_entry(list_one, type, list_name);\
	type* two = list_entry(list_two, type, list_name);\
	*return_val = one->key - two->key;\
})

//----------------------------------------------------------------
// 宏函数名称 sort_list_to_new_list
/// @brief 按照key将src_list链表中所有节点按照key成员由大至小排序并转移至new_list
/// 
/// new_list最好为空表或内部节点有序
/// new_list中节点与src_list中节点类型必须相同
/// @param src_list->类型struct list_head*,原始链表表头地址
/// @param new_list->类型struct list_head*,目标链表表头地址
/// @param type->链表中节点的类型
/// @param key->排序用比较大小的节点成员
/// @param list_name->链表关联的节点成员名(计算偏移量)
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
#define sort_list_to_new_list(src_list, new_list, type, key, list_name) \
({ _s32 move_flag = 0;\
	type * src = NULL;\
	type * sort = NULL;\
	struct list_head * pos = NULL;\
	struct list_head * temp_pos = NULL;\
	struct list_head * sort_pos = NULL;\
	list_for_each_safe(pos, temp_pos, src_list)\
	{\
		move_flag = 0;\
		src = list_entry(pos, type, list_name);\
		list_for_each(sort_pos, new_list)\
		{\
			sort = list_entry(sort_pos, type, list_name);\
			if(src->key <= sort->key)\
			{\
				list_move_tail(&src->list_name, &sort->list_name);\
				move_flag = 1;\
				break;\
			}\
		}\
		if(move_flag == 0)\
			list_move_tail(&src->list_name, new_list);\
	}\
})

//----------------------------------------------------------------
// 宏函数名称 copy_list_to_new_list_tail
/// @brief 拷贝src_list中所有节点至new_list
///
/// @param src_list->类型struct list_head*,原始链表表头地址
/// @param new_list->类型struct list_head*,目标链表表头地址
/// @param type->链表中节点的类型
/// @param list_name->链表关联的节点成员名(计算偏移量)
/// @attention type成员只能关联一个链表,成员不得包含指针类型
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
#define copy_list_to_new_list_tail(src_list, new_list, type, list_name) \
({ \
	type * src = NULL;\
	type * sort = NULL;\
	struct list_head * pos = NULL;\
	struct list_head * temp_pos = NULL;\
	list_for_each_safe(pos, temp_pos, src_list)\
	{\
		src = list_entry(pos, type, list_name);\
		sort = wys_malloc(sizeof(type));\
		if(!sort)an_exit(__L__,-1);\
		memcpy(sort,src,sizeof(type));\
		INIT_LIST_HEAD(&sort->list_name);\
		/*此处优先加到尾部，可提升之后的排序效率(原数据有序)*/\
		list_add_tail(&sort->list_name, new_list);\
	}\
})

enum LIST_COMMEN_SORT_TYPE
{
	LIST_QQ_WK_MSG_SORT = 0
	,LIST_DETAIL_WK_ACT_SORT = 1
	,LIST_GROUP_BASE_INFO_SORT = 2
};

/// @brief 存储每个socket连接信息
///
typedef struct an_socket_s
{
	struct hlist_node hash;		///< 关联an_sock_hash
	
	_s32 index;					///< 在an_conn_array中的index

	_s8* org_send_buf;			///< 原始数据(未替换'\'等特殊字符)
	_s8* send_buf;				///< 最终发送数据
	_s32 send_len;				///< 发送数据长度
	_s32 send_buflen; 			///< 发送最大字节数，暂定 204800(暂未使用)
	_s8* rcv_buf;				///< 实际接收缓存区
	_s32 rcv_len;				///< 实际接收缓存区大小
	_s32 rcv_buflen;			///< 接收缓存区总容量(内存空间)
}an_socket_t;

#define AN_POLL_MAX 	1024

extern struct hlist_head an_forbid_hash[HASH_SIZE];  ///< 违规行为hash表
extern struct list_head  an_forbid_list;             ///<所有违规行为链表
extern struct list_head  an_forbid_group_list[GROUP_ID_MAX];   ///<用户组违规行为链表

extern struct list_head  an_group_list; 			///< 组基本信息链表
extern struct hlist_head an_group_hash[HASH_SIZE]; 	///< 组基本信息哈希表

extern struct hlist_head encrypt_hash[HASH_SIZE];   ///< 文件加密hash表
extern struct list_head  encrypt_all_list;          ///<所有文件加密规则链表
extern struct list_head  encrypt_group_list[GROUP_ID_MAX]; ///<用户组文件加密链表  

extern struct hlist_head an_sock_hash[HASH_SIZE];	///< sock哈希表,以在an_conn_array中的下标作为哈希值
extern struct hlist_head an_web_cache_hash[HASH_SIZE];	///< sock哈希表,以在an_conn_array中的下标作为哈希值

extern struct hlist_head ww_user_hash[HASH_SIZE];			///< 全部用户的行为统计哈希表(关闭窗口/关闭URL)
extern struct list_head ww_user_list;						///< 全部用户的行为统计链表(关闭窗口/关闭URL)
extern struct list_head ww_user_group_list[GROUP_ID_MAX];	///< 所有用户组的行为统计链表(关闭窗口/关闭URL)

extern struct list_head auto_save_user_group_list;			///< 用户组的行为统计链表,每天保存时临时生成一个
extern _u32 auto_save_user_group_user_cnt[GROUP_ID_MAX];	///< 用户组的行为统计人数,每天保存时临时生成一个

extern struct hlist_head an_detail_user_hash[HASH_SIZE];			///< 全部用户行为记录哈希表
extern struct list_head an_detail_user_list;						///< 全部用户行为记录链表
extern struct list_head an_detail_user_group_list[GROUP_ID_MAX];	///< 用户组行为记录链表

extern struct list_head an_qq_user_list;						///< 全部用户聊天记录链表
extern struct list_head an_qq_user_group_list[GROUP_ID_MAX];	///< 用户组聊天记录链表
extern struct hlist_head an_qq_user_hash[HASH_SIZE];			///< 全部用户聊天记录哈希表

extern struct list_head an_work_rule_list;					///< 全部办公行为链表
extern struct list_head an_work_group_list[GROUP_ID_MAX];	///< 用户组办公行为链表
extern struct hlist_head an_work_rule_hash[HASH_SIZE];		///< 全部办公行为哈希表

extern _s32 an_group_num;		///< an_group_list节点数量(用户组数量 没算GID==0)
extern _s32 an_work_rule_num;	///< an_work_rule_list节点数量(办公行为规则数量)

extern _s8* ww_chat_list[];		///< 聊天软件进程名表
extern _s8* ww_move_list[];		///< 影音娱乐进程名表


_s32 forbid_act_hash_value(_s8*name);   //必须是字符串  小于32字节
_s32 forbid_act_hash_del(_s8*name,_s32 gid);
void forbid_act_hash_del_group(_s32 gid);
void forbid_act_hash_del_all(void);
_s32 forbid_act_hash_add(_s8* name, _s32 size, _s32 gid, _u8 act_type, _u8 is_forbid, _u8 flag);
ww_forbidact_t* forbid_act_hash_find(_s8*name,_s32 gid);
//-----------------------------------------------
//forbid_act_hash_value 
/// @brief 算出哈希值
///
/// @param _in name 违规行为名称
/// @return 返回算出的哈希值
/// @author 李杰明
//----------------------------------------------

//-------------------------------------------------
//forbid_act_hash_del
/// @brief 删除哈希表中的单条规则
///
/// @param _in name违规行为名称  
/// @param _in gid用户组ID
/// @return 成功0 失败-1
/// @author 李杰明
//------------------------------------------------

//------------------------------------------------
//forbid_act_hash_del_group
/// @brief 删除某组的所有规则
///
/// @param _in gid用户组ID
/// @author 李杰明
//------------------------------------------------

//------------------------------------------------
//forbid_act_hash_del_all
/// @brief  删除所有规则
///
/// @author 李杰明
//------------------------------------------------

//------------------------------------------------
//forbid_act_hash_add
/// @brief 添加规则
///
/// @param _in name规则名 
/// @param _in size规则名长度 
/// @param _in gid用户组ID 
/// @param _in act_type违规行为类型 
/// @param _in is_forbid是否禁止 
/// @param _in flag调用位置
/// @return 成功0 失败-1
/// @author 李杰明
//------------------------------------------------

_s32 encrypt_file_hash_del(_s8*name, _s32 gid);
void encrypt_file_hash_del_group(_s32 gid);
void encrypt_file_hash_del_all(void);
_s32 encrypt_file_hash_add(_s8* name, _s32 name_len, _s32 gid, _u8 filetype, _u32 authority, _u16 enable, _u8 flag);


//----------------------------------------
//encrypt_file_hash_del
/// @brief 删除单一规则
///
/// @param _in name加密文件名 
/// @param _in gid用户组ID
/// @return 成功0 失败-1
/// @author 李杰明
//----------------------------------------

//---------------------------------------
//encrypt_file_hash_del_group
/// @brief 删除用户组所有规则
///
/// @param _in gid用户组ID
/// @author 李杰明
//---------------------------------------

//---------------------------------------
//encrypt_file_hash_del_all
/// @brief 删除所有规则
///
/// @author 李杰明
//--------------------------------------

//--------------------------------------
//encrypt_file_hash_add 
/// @brief 添加单一规则
///
/// @param _in name加密文件名 
/// @param _in namelen文件名长度 
/// @param _in gid用户组ID 
/// @param _in filetype文件类型 
/// @param _in authority权限 
/// @param _in enable是否允许 
/// @param _in flag调用位置
/// @return 成功0 失败-1
/// @author 李杰明
//--------------------------------------

/// @brief 根据gid获取hash值(针对an_group_hash)
_u32 ww_group_base_info_st_get_hash(_s32 gid);
/// @brief 创建ww_group_base_info_t并初始化
ww_group_base_info_t* ww_group_base_info_st_create(void);
/// @brief 根据gid,在an_group_hash中查找ww_group_base_info_t是否存在
ww_group_base_info_t * ww_group_base_info_st_find_by_id(_s32 gid);
/// @brief 删除ww_group_base_info_t
void ww_group_base_info_st_del(_s32 gid);

/// @brief 通过index获取哈希值(针对an_sock_hash)
_s32 an_socket_t_get_hash(_s32 index);
/// @brief 创建an_socket_t并初始化
an_socket_t* an_socket_t_create(void);
/// @brief 根据index,在an_sock_hash查找an_socket_t是否存在
an_socket_t* an_socket_t_find_by_index(_s32 index);
/// @brief 删除an_socket_t
void an_socket_t_del(_s32 index);

/// @brief 根据token,在an_web_cache_hash查找an_web_cache_t是否存在
an_web_cache_t* an_web_cache_t_find_by_token(_s32 token);
/// @brief 根据token,在an_web_cache_hash查找或创建an_web_cache_t
an_web_cache_t* an_web_cache_t_find_create_by_token(_s32 token);

/// @brief 通过user_id获取哈希值(针对ww_user_hash)
_s32 ww_user_action_st_get_hash(_s32 user_id);
/// @brief 根据name获取hash值(针对ww_action_list_st)
_u32 ww_action_name_hash(_s8* name);
/// @brief 删除单个ww_user_action_st节点
void ww_user_action_st_del(struct ww_user_action_st * user);
/// @brief 清理某个用户组下所有用户(针对ww_user_group_list,基本没用)
void ww_user_action_st_del_group(_s32 gid);
/// @brief 清空临时auto_save_user_group_list
void ww_user_action_st_group_clear(void);
/// @brief 清空所有用户统计(针对ww_user_list)
void ww_user_action_st_clear(void);
/// @brief 根据user_id,在ww_user_hash查找user_action_st是否存在
struct ww_user_action_st * ww_user_action_find_by_id(_u32 user_id);
/// @brief 根据gid查找或创建user_action_st
struct ww_user_action_st* ww_user_action_gid_find_create(_u32 group_id);
/// @brief 根据user_id查找或创建user_action_st
struct ww_user_action_st* ww_user_action_id_find_create(_u32 user_id);
/// @brief 查找或创建ww_action_list_st
struct ww_action_list_st *ww_action_st_user_name_type_find_create(
	struct ww_user_action_st * user, _s8* pro_name, _s8* act_name, _u32 type, _u32 category);
/// @brief 在指定hash表中查找并创建ww_action_list_st
struct ww_action_list_st *ww_action_st_hash_name_type_find_create(
	struct hlist_head * hash_list, _s8* pro_name, _s8* act_name, _u32 type);
/// @brief 删除某个ww_action_list_st规则链表
void ww_user_action_list_del(struct list_head * head);
/// @brief 创建user_action_st并初始化
struct ww_user_action_st* ww_user_action_st_create(void);

/// @brief 删除单个an_detail_act_t
void an_detail_act_t_del(an_detail_act_t* action);
/// @brief 创建an_detail_wk_act_t节点并初始化
an_detail_wk_act_t* an_detail_wk_act_t_create(void);
/// @brief 删除单个an_detail_wk_act_t节点
void an_detail_wk_act_t_del(an_detail_wk_act_t* temp);
/// @brief 清理指定链表(针对an_detail_wk_act_t)
void an_detail_wk_act_t_del_list(struct list_head* head);
/// @brief 创建an_detail_act_t并初始化(添加至用户行为记录链表)
an_detail_act_t* an_detail_act_t_create_to_list(struct list_head* list, action_st * user_action, _u32 gid);

/// @brief 通过uid获取哈希值(针对an_detail_user_hash)
_s32 an_detail_user_t_get_hash(_u32 uid);
/// @brief 删除an_detail_user_t节点
void an_detail_user_t_del(an_detail_user_t * temp);
/// @brief 清理某组下所有用户(针对an_detail_user_group_list,基本没用)
void an_detail_user_t_del_group(_s32 gid);
/// @brief 清空an_detail_user_list
void an_detail_user_t_clear(void);
/// @brief 根据uid,在an_detail_user_hash中查找an_detail_user_t是否存在
an_detail_user_t* an_detail_user_t_find_by_uid(_u32 uid);
/// @brief 根据uid,查找或创建an_detail_user_t
an_detail_user_t* an_detail_user_t_find_create_by_uid(_u32 uid);

/// @brief 根据QQ账号获取hash值
_s32 an_qq_wk_msg_t_get_hash(_u32 qq_account);
/// @brief 删除an_qq_wk_msg_t节点
void an_qq_wk_msg_t_del(an_qq_wk_msg_t* temp);
/// @brief 清理具体链表(针对an_qq_wk_msg_t)
void an_qq_wk_msg_t_del_list(struct list_head* head);
/// @brief 创建an_qq_wk_msg_t并初始化
an_qq_wk_msg_t* an_qq_wk_msg_t_create(_u32 msg_len);
/// @brief 根据QQ账号和用户ID在指定hash表中查找an_qq_wk_msg_t节点
an_qq_wk_msg_t* an_qq_wk_msg_t_find_by_qq_account(_u32 qq_account, _u32 uid, struct hlist_head * hash_head);

/// @brief 删除an_qq_msg_t
void an_qq_msg_t_del(an_qq_msg_t* temp);
/// @brief 创建an_qq_msg_t节点并初始化
an_qq_msg_t* an_qq_msg_t_create(_u32 msg_len);

/// @brief 通过QQ账号获取哈希值(针对an_qq_user_t中qq_hash成员)
_s32 an_qq_obj_t_get_hash(_u32 qq_account);
/// @brief 根据QQ账号,在指定的hash中查找an_qq_obj_t是否存在
an_qq_obj_t* an_qq_obj_t_find_by_qq_account(_u32 qq_account, struct hlist_head * hash_head);
/// @brief 根据QQ账号,在指定的hash中查找或创建an_qq_obj_t
an_qq_obj_t* an_qq_obj_t_find_create_by_qq_account(_u32 qq_account, struct hlist_head * hash_head);

/// @brief 清理全部用户聊天记录(针对an_qq_user_list)
void an_qq_user_t_clear(void);
/// @brief 通过uid获取哈希值,(针对an_qq_user_hash)
_s32 an_qq_user_t_get_hash(_u32 uid);
/// @brief 删除an_qq_user_t节点
void an_qq_user_t_del(an_qq_user_t *temp);
/// @brief 清理某组下的用户聊天记录(针对an_qq_user_group_list,基本没用)
void an_qq_user_t_del_group(_s32 gid);
/// @brief 根据uid,在an_qq_user_hash中查找an_qq_user_t是否存在
an_qq_user_t* an_qq_user_t_find_by_uid(_u32 uid);
/// @brief 查找或创建an_qq_user_t
an_qq_user_t* an_qq_user_t_find_create_by_uid(_u32 uid);

/// @brief 通过name获取哈希值(针对an_work_rule_hash)
_s32 an_work_rule_t_get_hash(_s8* name);
/// @brief 删除an_work_rule_t(单个规则)
void an_work_rule_t_del(an_work_rule_t* temp);
/// @brief 清空某组的所有办公行为规则(仅针对链表,不处理数据库)
void an_work_rule_t_del_group(_s32 gid);
/// @brief 清空所有办公行为规则(仅针对链表,不处理数据库)
void an_work_rule_t_clear(void);
/// @brief 根据type,name,gid查找an_work_rule_t是否存在,全匹配
an_work_rule_t* an_work_rule_t_find_by_type_name_gid(_u8 type, _s8* name, _u32 gid);
/// @brief 根据type,name,gid(含全部员工)查找an_work_rule_t是否存在,全匹配
an_work_rule_t* an_work_rule_t_find_by_type_name_gid_all(_u8 type, _s8* name, _u32 gid);
/// @brief 根据type,name,gid查找或创建an_work_rule_t
an_work_rule_t* an_work_rule_t_find_create_by_name_gid(_u8 type, _s8* name, _u32 gid);

/// @brief 获取页面关注的行为统计类型(AN_CATEGORY)
_u32 ww_get_category_by_ww_action_list_st(struct ww_action_list_st * act, _u32 gid);

/// @brief 初始化全局hash表/链表
void an_hash_and_list_init(void);

/// @brief 链表快速排序函数
void list_quick_sort(struct list_head* head, struct list_head* begin, struct list_head* end, _s32 compare_fun_type
					, _s32 (*compare_fun)(struct list_head* a, struct list_head* b,_s32 compare_fun_type));
/// @brief 交换链表节点(节点必须初始化)
void inline list_swap(struct list_head* p, struct list_head* q);
/// @brief list快排用公共比较函数
_s32 list_commen_compare_fun(struct list_head* a, struct list_head* b, _s32 compare_type);

#endif

