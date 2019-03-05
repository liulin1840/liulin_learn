/////////////////////////////////////////////////////////////////////
/// @file prefix.h
/// @brief 数据库KEY及相关VAL定义
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#ifndef _PREFIX_H
#define _PREFIX_H "prefix.h"
#include "types.h"

#define DIR_NAME_LEN 256

/***************SSDB key头定义************************/

/*********************用户账户类*************************/
/*
@key	:	stui-useraccount
@val	:	userid
@val type:	unique interger(RAND_ID)
@remark:	用户ID
*/
#define PRE_USER_ID			"stui-"
#define PRE_USER_ID_SE		"stui."

/*
@key	:	stun-userid
@val	:	useraccount
@val type:	32 bytes char
@remark:	用户名
*/
#define PRE_USER_NAME		"stun-"

/*
@key	:	stua-userid
@val	:	password
@val type:	16 bytes char
@remark:	用户密码
*/
#define PRE_USER_ACCOUNT	"stua-"

/*
@key	:	stug-userid
@val	:	groupid
@val type:	unique integer
@remark:	用户所属组
*/
#define PRE_USER_GROUP		"stug-"
#define PRE_USER_GROUP_SE	"stug."

/*
@key	:	stur-userid
@val	:	remark
@val type:	64 bytes char
@remark:	用户备注
*/
#define PRE_USER_REMARK		"stur-"

/*
@key	:	stub-userid
@val	:	0/1		(0:否,1:是)
@val type:	unsigned char
@remark:	是否绑定电脑
*/
#define PRE_USER_BIND		"stub-"			

/*
@key	:	stum-userid
@val	:	mac
@val type:	unsigned long long
@remark:	最后6位为用户MAC地址
*/
#define PRE_USER_MAC		"stum-"

/*
@key	:	stucid-userid
@val	:	computerid
@val type:	32 bytes char
@remark:	计算机唯一识别码
*/
#define PRE_USER_COMPUTER_ID "stucid-"

/*
@key	:	stubid-userid
@val	:	computerid
@val type:	32 bytes char
@remark:	账号绑定码
*/
#define PRE_USER_BIND_ID "stubid-"


/*
@key	:	stus-userid
@val	:	0/1		(0/not found:离线,1:在线)
@val type:	unsigned char
@remark:	用户当前状态
*/
#define PRE_USER_STAT		"stus-"
#define PRE_USER_STAT_SE	"stus."

/*
@key	:	stuf-userid
@val	:	authority (参考ACCESS_ENCRYPTION_FILE_AUTHORITY)
@val type:	32 bytes unsigned int(网络字节序)
@remark:	用户访问加密文件的缺省权限
*/
#define PRE_USER_FILEAUTH	"stuf-"

/*
@key	:	stue-userid
@val	:	option(0x00000000) (参考ENCRYPT_OPTION，目前没在用 -> 磁盘加密)
@val type:	32 bytes unsigned int
@remark:	用户加密选项缺省设置
*/
#define PRE_USER_ENCRYPTOPT	"stue-"

/*
@key	:	stups-userid
@val	:	0:无操作,1:更新成功,2:更新失败,3:正在更新
@val type:	unsigned char
@remark:	用户进程列表更新状态
*/
#define PRE_USER_PROCESS_STAT	"stups-"

/*
@key	:	stupi-userid
@val	:	processes_info_t
@val type:	struct
@remark:	用户进程列表信息
*/
#define PRE_USER_PROCESS_INFO	"stupi-"

/*
@key	:	stuct-userid
@val	:	time
@val type:	32 bit unsigned int	//全文的32 bytes基本都是bit
@remark:	用户创建时间
*/
#define PRE_USER_CREATE_TIME	"stuct-"

/***********************分组管理类****************************/
/*
@key	:	stugi-user_group_name
@val	:	usergroupid
@val type:	unique int	(新增ID从JS获取,从1开始递增,0代表全部用户)
@remark:	分组ID
*/
#define PRE_USERGROUP_ID		"stugi-"
#define PRE_USERGROUP_ID_SE		"stugi."

/*
@key	:	stugn-usergroupid
@val	:	user_group_name
@val type:	32 bytes char
@remark:	分组名
*/
#define PRE_USERGROUP_NAME		"stugn-"

/*
@key	:	stugr-usergroupid
@val	:	authority	(参照USER_ACTION_MONITOR_OPTION)
@val type:	32 bytes unsigned int(网络字节序)
@remark:	用户活动监视选项
*/
#define PRE_USERGROUP_MONITOROPT "stugm-"

/*
@key	:	stugf-usergroupid
@val	:	authority (参考ACCESS_ENCRYPTION_FILE_AUTHORITY)
@val type:	32 bytes unsigned int(网络字节序)
@remark:	用户访问加密文件的缺省权限
*/
#define PRE_USERGROUP_FILEAUTH	"stugf-"

/*
@key	:	anuga-usergroupid-GMT -->gid长度不一样，注意scan
@val	:	struct an_user_calc_acts_s (均为本地序)
@val type:	struct
@remark :	用户组每天行为统计
*/
#define PRE_USERGROUP_CALC_ACTION		"anuga-"
#define PRE_USERGROUP_CALC_ACTION_SE	"anuga."

/*
@key	:	anugat-usergroupid
@val	:	GMT		(逆序)
@val type:	_u64
@remark	:	用户最近一次行为统计时间(以天为单位)
*/
#define PRE_USERGROUP_CALC_ACTTIME		"anugat-"
#define PRE_USERGROUP_CALC_ACTTIME_SE	"anugat."

/*********************目录保护类*****************/
//权限参考 FOLDER_RW_CONTROL

/*
@key	:	stsu-usergroupid
@val	:	authority	(参考FOLDER_RW_CONTROL)
@val type:	32 bytes unsigned int(网络字节序)
@remark:	USB访问控制
*/
#define PRE_SET_USBACCESS	"stsu-"

/*
@key	:	stsn-usergroupid
@val	:	authority	(参考FOLDER_RW_CONTROL)
@val type:	32 bytes unsigned int(网络字节序)
@remark:	光盘设备访问控制
*/
#define PRE_SET_CDACCESS	"stsc-"
/********************用户动态行为数据************************/
/*
@key	:	dyua-userid-GMT-type
@val	:	action_st	(ww_action.h)
@val type:	struct
@remark:	用户操作基础数据
*/
#define PRE_USER_ACTION 	"dyua-"
#define PRE_USER_ACTION_CNT "dyuac-"
#define PRE_USER_ACTION_SE	"dyua."

/*
@key	:	dyuq-userid-GMT-count
@val	:	qq_msg_st	(ww_action.h)
@val type:	struct
@remark:	用户QQ聊天信息
*/
#define PRE_USER_QQ_MESSAGE			"dyuq-"
#define PRE_USER_QQ_MESSAGE_SE		"dyuq."

/*
@key	:	anuq-userid-qqid-GMT-hour //GMT为当天00:00:00 逆序 
@val	:	an_qq_calc_msgs_t	--->当天最新消息通过hour 23~0 的有效time遍历数据找到time相同的
@val type:	struct					↑数据无序，取出时需要相关排序接口
@remark:	用户QQ聊天统计信息
*/
#define PRE_USER_QQ_CALC_MSG		"anuq-"
#define PRE_USER_QQ_CALC_MSG_SE		"anuq."

/*
@key	:	anuqt-userid-qqid
@val	:	an_qq_calc_msgtime_t
@val type:	struct
@remark:	用户QQ聊天统计信息
*/
#define PRE_USER_QQ_CALC_MSGTIME		"anuqt-"
#define PRE_USER_QQ_CALC_MSGTIME_SE		"anuqt."

/*
@key	:	anua-userid-GMT
@val	:	struct an_user_calc_acts_s (均为本地序)
@val type:	struct
@remark :	用户每天行为统计
*/
#define PRE_USER_CALC_ACTION		"anua-"
#define PRE_USER_CALC_ACTION_SE		"anua."

/*
@key	:	anuat-userid
@val	:	GMT		(逆序)
@val type:	_u64
@remark	:	用户最近一次行为统计时间(以天为单位)
*/
#define PRE_USER_CALC_ACTTIME		"anuat-"
#define PRE_USER_CALC_ACTTIME_SE	"anuat."

/*
@key	:	anua-userid-type-GMT-hour (GMT以天为单位，是当天00:00:00, 逆序)
@val	:	struct an_user_detail_acts_t (均为本地序) -->内部数据无序
										↑需写个相关排序算法(导出及筛选内容添加均需要)
@val type:	struct
@remark :	用户每天行为记录
*/
#define PRE_USER_DETAIL_ACTION		"anud-"
#define PRE_USER_DETAIL_ACTION_SE	"anud."

/*
@key	:	anudt-userid
@val	:	GMT		(逆序)
@val type:	_u64
@remark	:	用户最近一次行为记录的统计时间(以小时为单位 ->当天某小时 20:00:00)
*/
#define PRE_USER_DETAIL_ACTTIME		"anudt-"


/************************办公行为类*****************************/
/*
下面的全抛弃，改用结构体存储
内容为analyzer使用，全为本地序
@key	:	stws-workrule		->(静态唯一key)(所有规则均在此key内)usergroupid相关
@val	:	struct az_task_recv_work_acts_s	  		↑上限1024条
@val type:	struct
@remark	:	办公行为规则
*/
#define PRE_WORK_STRUCT		"stws-workrule"

/************************违规行为类*****************************/
/*
@key	:	stfam-wenable	(静态key)
@val	:	0/1					(0:否,1:是)
@val type:	unsigned char
@remark:	警告消息是否开启
*/
#define PRE_FORBIDACT_MWE		"stfam-wenable"

/*
@key	:	stfam-warning		(静态key)
@val	:	warning_message
@val type:	128 bytes char
@remark:	警告消息
*/
#define PRE_FORBIDACT_MW		"stfam-warning"

/*
@key	:	stfam-eenable		(静态key)
@val	:	0/1					(0:否,1:是)
@val type:	unsigned char
@remark:	禁止消息是否开启
*/
#define PRE_FORBIDACT_MEE		"stfam-eenable"

/*
@key	:	stfam-error			(静态key)
@val	:	error_message
@val type:	128 bytes char
@remark:	禁止消息
*/
#define PRE_FORBIDACT_ME		"stfam-error"


/**********************与服务器通信结构**************************/
/*
@key	:	stac-usergroupid
@val	:	app_ctrl_filters_t
@val type:	struct
@remark:	用户组违规进程结构
*/
#define PRE_APP_CONTROL			"stac-"

/*
@key	:	stuc-usergroupid
@val	:	filters_key_t -> url_key_filter_t
@val type:	struct
@remark:	用户组违规网页搜索/域名关键字结构
*/
#define PRE_URL_KEY_CONTROL		"stuc-"

/*
@key	:	stwc-usergroupid
@val	:	filters_key_t -> win_key_filter_t
@val type:	struct
@remark:	用户组违规窗口关键字结构
*/
#define PRE_WINDOW_KEY_CONTROL	"stwc-"

/*
@key	:	stsps-usergroupid
@val	:	encrypt_file_filters_data_buffer
@val type:	struct
@remark:	用户组应用程序加密结构
*/
#define PRE_SET_PROCESS_ST		"stsps-"
#define PRE_SET_PROCESS_ST_SE	"stsps."

/*
@key	:	stses-usergroupid
@val	:	encrypt_file_filters_data_buffer
@val type:	struct
@remark:	用户组后缀名文件加密结构
*/
#define PRE_SET_EXT_ST		"stses-"
#define PRE_SET_EXT_ST_SE	"stses."

/*
@key	:	stspt-usergroupid
@val	:	encrypt_file_filters_data_buffer
@val type:	struct
@remark:	用户组后缀名文件加密结构
*/
#define PRE_SET_DIR_ST		"stsds-"
#define PRE_SET_DIR_ST_SE	"stsds."

/************************系统设置类****************************/
//系统设置头
#define PRE_SYSTEM_OPTION		"stsys-"

/*
@key	:	stsys-login			(静态key)
@val	:	0/1					(0:否,1:是)
@val type:	unsigned char
@remark:	强制员工登陆
*/
#define SYSTEM_LOGIN_CLIENT	"stsys-login"

/*
@key	:	stsys-cachepath		(静态key)
@val	:	path
@val type:	128 bytes char		(可以扩充)
@remark:	缓存路径设置
*/
#define SYSTEM_CACHE_PATH		"stsys-cachepath"

/*
@key	:	stsys-breaktime		(静态key)
@val	:	time1;time2;time3...	(可以修改-->页面看)
@val type:	128 bytes char		(可以扩充)
@remark:	休息时间设置
*/
#define SYSTEM_BREAK_TIME		"stsys-breaktime"

/*
@key	:	stsys-breaktime-struct	(静态key)
@val	:	client_break_times_t
@val type:	struct
@remark:	休息时间设置(发送给客户端设置用)
*/
#define SYSTEM_BREAK_TIME_STRUCT	"stsys-breaktime-struct"

/*
@key	:	stsys-jumpurl		(静态key)
@val	:	url
@val type:	128 bytes char
@remark:	违规跳转链接
*/
#define SYSTEM_JUMP_URL			"stsys-jumpurl"

/*
@key	:	stsys-password		(静态key)
@val	:	password
@val type:	16 bytes char
@remark:	管理端密码
*/
#define SYSTEM_PASSWORD			"stsys-password"

/************************ 私有数据定义 *************************/

/*
@key	:	prfk-guid	(guid 文件唯一标识)
@val	:	key
@val type:	48 bytes char
@remark	:	文件加密aes-key
*/
#define PRE_FILE_KEY	"prfk-"

/*
@key	:	prsu-		(静态key)
@val	:	password
@val type:	32 bytes char
@remark	:	数据库超级权限密码
*/
#define PRE_SUPER_PWD	"prsu-"

/*
@key	:	prui-
@val	:	update_info_t
@val type:	struct
@remark	:	更新包及版本信息下载路径结构体
*/
#define PRE_UPDATE_INFO	"prui-"

#if 0
//	预留(暂时不用，如果用了的话，要在cgi_dbdel_ww_group函数中做相应处理)
//	即删除组时，要删除相关规则
/*
@key	:	stsai-usergroupid-pathname
@val	:	pathid	(新增ID从JS获取，批量导入时由analyzer判断)
@val type:	unique unsigned char	(可以扩大)
@remark:	其他目录访问控制规则ID
*/
#define PRE_SET_ACCESSID	"stsai-"

/*
@key	:	stsap-usergroupid-pathid
@val	:	pathname
@val type:	256 bytes char	(扩大需要客户端进行协调)
@remark:	其他目录路径
*/
#define PRE_SET_ACCESSPATH	"stsap-"

/*
@key	:	stsaa-usergroupid-pathid
@val	:	authority
@val type:	32 bytes unsigned int
@remark:	其他目录访问权限
*/
#define PRE_SET_ACCESSAUTH	"stsaa-"

/*
@key	:	stsat-usergroupid-pathid
@val	:	type(0:本地目录,1:网络映射目录)
@val type:	unsigned char
@remark:	其他目录类型
*/
#define PRE_SET_ACCESSTYPE	"stsat-"


//暂时不用的分组
/************************网址分组类*****************************/
//网址分组ID	key:stdgi-dnsgroupname	val:dnsgroupid		(unique unsigned char)
#define PRE_DNSGROUP_ID		"stdgi-"
//网址分组名	key:stdgi-dnsgroupid	val:dnsgroupname	(64 bytes char)
#define PRE_DNSGROUP_NAME	"stdgn-"

/************************网址域名类******************************/
//网址ID	key:stdi-dnsname	val:dnsid	(unique integer)(从1开始递增,新增ID从JS判断)
#define PRE_DNS_ID		"stdi-"
//网址域名	key:stdi-dnsid		val:dnsname	(64 bytes char)
#define PRE_DNS_NAME	"stdn-"
//所属分组	key:stdg-dnsid		val:dnsgroupid	(unsigned char)
#define PRE_DNS_GROUP	"stdg-"

/************************程序分组类******************************/
//程序分组ID	key:stpgi-programgroupname	val:programgroupid	(unique unsigned char)
#define PRE_PROGRAMGROUP_ID		"stpgi-"
//程序分组名	key:stpgn-programgroupid	val:programgroupname	(64 bytes char)
#define PRE_PROGRAMGROUP_NAME	"stpgn-"

/************************程序名类******************************/
//程序名ID		key:stpi-programname		val:programid		(unique unsigned integer)(从1开始递增，新增ID从JS判断)
#define PRE_PROGRAM_ID		"stpi-"
//进程名		key:stpi-programid			val:programname		(32 bytes char)
#define PRE_PROGRAM_NAME	"stpn-"
//所属分组		key:stpg-programid			val:programgroupid	(unsigned char)
#define PRE_PROGRAM_GROUP	"stpg-"
#endif

/// @brief 加密规则类型
enum  encryption_filter_type
{	
	EFT_EXT_FILTER     = 0,
	EFT_PROCESS_FILTER = 1,
	EFT_DIR_FILTER     = 2,
};

#pragma pack(1)

/// @brief 单个进程过滤器结构(文件加密)(成员均为网络序)
///
/// @see ACCESS_ENCRYPTION_FILE_AUTHORITY ww_action.h
typedef struct
{
	char   name[32] ;      ///< 进程名称
	_u32   authority;      ///< 访问权限
	_u16   enable;         ///< 是否启用该过滤器   1:是,0:否
	_u16   exts;		   ///< related_ext 条目数
	char   data[0];		   ///< related_ext[][10];(暂未使用)
}process_filter_t;


/// @brief 单个扩展名称过滤器(文件加密)(成员均为网络序)
///
/// @see ACCESS_ENCRYPTION_FILE_AUTHORITY ww_action.h
typedef struct
{
	char    ext[10] ;		///< 扩展名称
	_u32    authority;      ///< 访问权限
	_u16    enable ;        ///< 是否启用该过滤器  1:是,0:否
}ext_filter_t;



/// @brief 单个目录过滤器过滤器(文件加密)(成员均为网络序)
///
typedef struct
{
	_u16     enable ;           ///< 是否启用该过滤器
	_s32     option;            ///< 此处使用文件加密的选项(即:访问权限)
	char     dir[DIR_NAME_LEN]; ///< 目录名称
}dir_filter_t;

/// @brief 过滤器数据(适应用于所有文件加密过滤器)(成员均为网络序)
///
typedef struct
{
	_s32	num;			///< 包含几个过滤器
	char	data[0];		///< 过滤器数据，可以process_filter_t，ext_filter_t或dir_filter_t
}encrypt_file_filters_data_buffer;

/// @brief 单个应用程序控制过滤器(违规行为)(成员均为网络序)
///
typedef struct
{
	char       app[32];  	///< 进程名称
	_u16       enable;  	///< TRUE:启用该过滤器,FALSE:禁止该过滤器
	_u16	   option; 		///< 1:禁止运行,2：禁止改名
}app_ctrl_filter_t;

/// @brief 用户应用程序控制过滤器(违规行为)(成员均为网络序)
///
typedef struct
{
	_s32	num;			///< 包含几个过滤器
	char	data[0];		///< app_ctrl_filter_t* filters;
}app_ctrl_filters_t;

/// @brief 单个窗口/域名控制过滤器(违规行为)(成员均为网络序)
///
typedef struct
{
	_u8 kind;		///< 关键字种类(1:域名,2:搜索关键字)
	_u8 type;		///< 1:违规 2:禁止
	char data[32];	///< 违规描述
}win_key_filter_t,url_key_filter_t;

/// @brief 用户窗口/域名控制过滤器(违规行为)(成员均为网络序)
///
typedef struct
{
    _u16   num;       ///< 携带过滤器个数
	char   data[0];   ///< win_key_filter_t/url_key_filter_t
}filters_key_t;

/// @brief 单个进程信息(进程列表)(成员均为网络序)
///
typedef struct
{
	_u32	pid;			///< pid
	char	pro_name[32];	///< 进程名
	char	user_name[32];	///< 用户名
	_u32	cpu_per;		///< Cpu百分比
	_u32	mem_own;		///< 内存占用K
	char    description[48];///< 进程描述
}process_info_t;

/// @brief 用户进程信息(进程列表)(成员均为网络序)
///
typedef struct
{
	_u32 num;			///< 进程数量
	char data[0];		///< process_info_t
}processes_info_t;

/// @brief 客户端升级信息(成员均为网络序)
///
typedef struct
{
	_s8 upgrade_url[1024];	///< 升级压缩包下载地址
	_s8 version_info[1024];	///< 版本信息下载地址
}update_info_t;

/// @brief 单个工作时间段(客户端监控时间)(成员均为网络序)
///
/// 工作时间段以24小时为一个周期，单位为秒
/// 比如9:00-12:00->9*60*60-12*60*60->32400-43200
typedef struct
{
	_u64 start_time;	///< 起始时间
	_u64 end_time;		///< 结束时间
}working_period_t;

/// @brief 客户端工作时间(监控时间)(成员均为网络序)
///
/// 只有工作时间内做监控(注意时间段必须是有序的)
/// 传输方向：客户端<--服务器
typedef struct
{
	_u16  week_check;          ///< 星期设置(第一位置为表示选中星期一，第二位表示信息二,以此类推)
	_u16  data_len;            ///< data的长度
	char  data[0];             ///< 1到n个working_period_t
}client_break_times_t;


/// @brief 单个行为记录数据(analyzer处理后)
///
/// 消息类别在保存瞬间确定，可能与行为统计的类别有差
/// 行为记录每小时保存一次，行为统计每天保存一次
/// 理论上行为记录更精确，行为统计更贴近用户需求
/// @see USER_ACTION_TYPE ww_action.h
/// @see auto_save_detail_act ww_action.h
typedef struct an_user_detail_act_s
{
	_u64	time;			///< 行为发生时间 (逆序)
	_u32	type;			///< 行为类型 @enum USER_ACTION_TYPE ww_action.h
	_u32	category;		///< 消息类别 @enum auto_save_detail_act ww_action.h
//	_u16	is_ciphertext;	///< 是否与密文相关事件
//	_u16	pad;			///< 预留字节(顺便对齐)
	_s8		process[32];	///< 与该事件相关进程
	_s8		actname[128];	///< 事件相关名称(窗口名 url网址域名 文件路径等)
}an_user_detail_act_t;

/// @brief 单个行为记录数据(analyzer处理后)
///
typedef struct an_user_detail_acts_s
{
	_s32	gid;		///< 当时行为所属gid(可能有一小时左右的误差)
	_u32	num;		///< 数据条目数
	_s8		data[0];	///< an_user_detail_act_t	数据无序，获取后需排序
}an_user_detail_acts_t;

/// @brief 单个行为统计数据(analyzer处理后)
///
/// 历史消息类别在保存瞬间确定,当天类别动态决定
/// 主要指是否办公行为,其他行为无冲突
///	这里仅统计OT_BROWSER_CLOSE_URL与OT_WINDOW_CLOSE
/// @see USER_ACTION_TYPE ww_action.h
/// @see AN_CATEGORY ww_action.h
typedef struct an_user_calc_act_s
{
	_s8		act_name[48];			///< 行为描述/网址域名
	_s8		pro_name[32];			///< 进程名
	_u32	time;					///< 持续时间
	_u32	type;					///< 行为类型 @enum USER_ACTION_TYPE ww_action.h
	_u32	category;				///< 消息类别 @enum AN_CATEGORY ww_action.h
	_u32	key_click;				///< 键盘按键数
	_u32	mou_click;				///< 鼠标点击数
}an_user_calc_act_t;

/// @brief 单个用户/用户组记录数据(analyzer处理后)
///
/// 历史消息类别在保存瞬间确定,当天类别动态决定
/// 主要指是否办公行为,其他行为无冲突
/// @see USER_ACTION_TYPE ww_action.h
/// @see AN_CATEGORY ww_action.h
typedef struct an_user_calc_acts_s
{
	_u32	gid;		///< 当天行为所属组ID
	_u32	num;		///< 行为条目数
	_u32	user_cnt;	///< 当天行为由几个用户产生-->用于求平均值(作为用户组数据时才使用)
	_s8		data[0];	///< an_user_calc_act_t
}an_user_calc_acts_t;

/// @brief 单个聊天记录数据(analyzer处理后)
///
typedef struct an_qq_calc_msg_s
{
	_u64 	time;				///< 消息产生时间   (逆序)
	_u32 	user_account;		///< 用户所使用账号
	_s8  	user_name[32];		///< 用户当时账号对应昵称
	_s8  	obj_name[32];		///< 聊天对象当时使用的昵称
	_u32 	issend;				///< 用户是是否发送方
	_u32 	msg_len;			///< 消息长度
	_s8  	data[0];			///< 消息内容
}an_qq_calc_msg_t;

/// @brief 与单个对象聊天记录数据(analyzer处理后)
///
typedef struct an_qq_calc_msgs_s
{
	_u64	time;		///< 最新数据时间  (逆序)
	_u32	num;		///< 聊天记录条目数
	_s8		data[0];	///< an_qq_calc_msg_t
}an_qq_calc_msgs_t;

/// @brief 与单个对象聊天统计数据(analyzer处理后)
///
typedef struct an_qq_calc_msgtime_s
{
	_u32	account;	///< 聊天对象的QQ号(在结构体中加了方便处理)
	_u32	num;		///< 总聊天记录条目数(仅保存，暂未使用)
	_s8		name[32];	///< 聊天对象最后一条聊天记录所使用昵称
	_u64	time;		///< 最后一次统计时间(GMT 小时为单位 10:00:00 逆序)
}an_qq_calc_msgtime_t;

#pragma pack()

#endif //end of #ifndef

