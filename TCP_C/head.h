/////////////////////////////////////////////////////////////////////
/// @file head.h
/// @brief 与WEB SERVER通信及数据库缓存结构体定义
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#ifndef ANALYZER_DATA_H
#define ANALYZER_DATA_H
#include "list.h"
#pragma pack(4)

/// @brief worker处理模块头信息
///
typedef struct wk_head_st
{
//	struct sockaddr_in 	addr;
	_u16				check_code; ///< 识别码(暂未使用)
	_u16				port;		///< 端口号(暂未使用)
	_u32				ip;			///< IP地址(暂未使用)
	_s32				token;		///< 随机ID (网页cookie)
	_u16				mode;		///< mode标示 0:处理,1:结果(暂未使用)
	_u16				flag;		///< flag 0:未处理,1:处理成功,2:处理失败(暂未使用)
	_u32				cmd;		///< 请求命令 @see az_task_order_enum cmds.h
	_u32				fd_index;	///< 本地pollfd数组的index,用于发包 @see an_conn_array main.c
	_u32				data_len;	///< 请求数据的长度
}wk_head_t;

/// @brief worker处理模块参数
///
typedef struct wk_task_st
{
	wk_head_t 	head;			///< @see wk_head_t
	_s8			data[0];		///< 请求的具体数据,一般情况由allocator解析赋值 @see al_allocate_task allocator.c
}wk_task_t;

/// @brief 用于处理内部请求(wk_task_t中data数据)
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct wk_local_task_s
{
	_u32		uid;				///< 用户ID
	struct an_auto_data_save_st* data_save;	///< auto_data_save模块用结构,记录当天时间与timer信息
	_u8 		pad[116];			///< 预留字节(注意对齐)
}wk_local_task_t;


/// @brief 用户信息相关操作
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_user_s
{
	_u32		uid;			///< 用户ID
	_s8 		remark[64];		///< 用户备注(姓名)
	_s8 		account[32];	///< 用户账号
	_s8 		password[16];	///< 用户密码
	_s32		old_gid;		///< 用户旧的gid
	_s32		new_gid;		///< 用户新的gid
	_u8 		bind;			///< 是否绑定电脑(0:否，1:是)
	_u8 		pad[3];			///< 预留字节(顺便对齐)
}az_task_recv_user_t;

/// @brief 文件加密相关操作
///

/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_file_s
{
	_s32		old_gid;			///< 原规则适用组
	_s32        new_gid;            ///< 新规则适用组
	_s8 		old_filename[256];	///< 原规则内容
	_s8         new_filename[256];  ///< 新规则内容
	_u32    	authority;			///< 密文被访问权限(全开->AEFA_ALL)  (本地序)
	_u8 		filetype;			///< 规则类型 (0:后缀名,1:进程名,2:目录名)
	_u8 		pad[3];				///< 预留字节(顺便对齐)
}az_task_recv_file_t;

/// @brief 每个组的密文权限
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_encrypt_s
{
	_s32		gid;			///< 组ID
	_u32    	authority;		///< 组对于的密文访问权限(全开->AEFA_ALL)--> (赋值为网络序)
}az_task_recv_encrypt_t;

/// @brief 密文权限相关操作
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_encrypts_s
{
	_s32		group_num;		///< 一共有几个用户组
	_s8			data[0];		///< az_task_recv_file_t
}az_task_recv_encrypts_t;

/// @brief 聊天记录相关操作
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_qq_s
{
	_u32 		uid;		///< 用户ID
	_s32		gid;		///< 组ID
	_s32		qq_id;		///< QQ号码
	_u64		start_time;	///< 开始时间(网页顺序传,接收注意转逆序)	(反过来用)
	_u64		end_time;	///< 结束时间(网页顺序传,接收注意转逆序)	(逆序需要反过来用)
	_s8			keyword[16];///< 查询关键字(可扩充)
	_s32		page_flag;	///< 继续查询标识,(0:刷新,1:继续获取后续数据)
}az_task_recv_qq_t;

/// @brief 行为记录相关操作
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_detail_s
{
	_u32		act_type;	///< 数据类型,参考auto_save_detail_act   0:ALL
	_s32 		uid;		///< 用户ID
	_s32		gid;		///< 组ID
	_u64		start_time;	///< 开始时间(需要注意逆序)	(反过来用)
	_u64		end_time;	///< 结束时间(需要注意逆序)	(逆序需要反过来用)
	_s32		page_flag;	///< 继续查询标识,(0:刷新,1:继续获取后续数据)
}az_task_recv_detail_t;

/// @brief 行为统计相关操作
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_calc_s
{
	_u32		uid;		///< 用户ID
	_s32		gid;		///< 组ID
	_u64		start_time;	///< 开始时间(网页顺序传,接收注意转逆序)	(反过来用)
	_u64		end_time;	///< 结束时间(网页顺序传,接收注意转逆序)	(逆序需要反过来用)
	_s32		page_flag;	///< 继续查询标识,(0:刷新,1:继续获取后续数据) (未使用)
}az_task_recv_calc_t;

/// @brief 用户组(部门)相关操作
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_group_s
{
	_s32		gid;			///< 组ID
	_s8 		group_name[32];	///< 组名称
	_u8 		record;			///< 是否行为记录
	_u8 		chat;			///< 是否聊天记录
	_u8 		control;		///< 是否行为管控
	_u8 		usb_auth;		///< USB使用权限
	_u8 		cd_auth;		///< CD使用权限
}az_task_recv_group_t;

/// @brief 系统设置相关操作
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_system_s
{
	_s8 		breaktime[128];	///< 休息时间(格式待补充)
	_s8 		jumpurl[128];	///< 违规跳转URL
	_s8 		password[16];	///< 防水墙访问密码
	_u64        start_time;		///< 起始工作时间(第一段)
	_u64        end_time;		///< 结束工作时间(第一段)
	_u64        t_start_time;	///< 起始工作时间(第二段)
	_u64        t_end_time;		///< 结束工作时间(第二段)
	_u16        work_day;		///< 工作日(周一~周六)
	_u8 		login_bind;		///< 强制登录

	_u8 		pad[3];			///< 预留字节(顺便对齐)
}az_task_recv_system_t;

/// @brief 单个办公行为
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_work_act_s
{
	_s8 		workname[48];	///< 行为描述(进程的话，优先根据行为库匹配，网址直接给域名)
	_s8 		process[32];	///< 进程名(网址没有)
	_s32		gid;			///< 生效gid
	_u8 		worktype;		///< 行为类型:  0:使用软件,1:浏览网页
	_u32		old_gid;		///< 原生效gid
	_u8 		pad[3];			///< 预留字节(顺便对齐)
}az_task_recv_work_act_t;

/// @brief 办公行为相关操作
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_work_adds_s
{
	_s32 	listcount;		///< 要修改/增加/删除的规则有多少条
	_s32 	gid;			///< 查询用gid
	_u8		type;			///< 查询用类型:  0:使用软件,1:浏览网页
	_s8		describe[48];	///< 查询用描述
	_u8		pad[3];			///< 预留字节(顺便对齐)
	_s8		data[0];		///< az_task_recv_work_act_t
}az_task_recv_work_acts_t;

/// @brief 违规行为相关操作
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_forbidact_s
{
	_s8 		old_name[32];        ///< 原违规行为内容
	_s8 		new_name[32];        ///< 新违规行为内容
	_s32		old_gid;             ///< 删除用old   get也用old  name同理
	_s32		new_gid;             ///< 创建用new 
	_u8 		act_type;            ///< 违规行为类型
	_u8 		is_forbid;           ///< 是否禁止
	_u8         search_flag;         ///< 1有搜索选项 //0没有搜索选项
	_u8 		pad[1];				 ///< 预留字节(顺便对齐)
}az_task_recv_forbidact_t;

/// @brief 警告消息相关操作
///
/// @see az_task_order_enum cmds.h
/// @see al_parse_value_to_wk_task allocator.c
typedef struct az_task_recv_warning_msg_s
{
	_s8 		warning_msg[128];    ///< 警告消息
	_s8 		error_msg[128];      ///< 错误消息
	_u8 		warning_en;          ///< 是否启用警告消息
	_u8 		error_en;            ///< 是否启用错误消息
	_u8 		pad[2];				 ///< 预留字节(顺便对齐)
}az_task_recv_warning_msg_t;

#pragma pack()

#endif //ANALYZER_DATA_H
