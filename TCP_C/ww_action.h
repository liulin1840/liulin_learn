#ifndef _WW_ACTION_H
#define _WW_ACTION_H "ww_action.h"
#include "types.h"
#include "list.h"
#pragma pack(1)
typedef struct
{
	_u64 		start_time;		//发生的时间或行为开始时间
	_u64 		end_time;		//行为结束时间(很多时候会被忽略)
	_u32 		type;			//操作类型,参考定义USER_ACTION_TYPE
	_u32 		category;		//消息类别,参考LOG_CATEGORY
	_u32 		pid;			//进程ID
	_s8 		process[32];	//与该事件相关的进程
	_u32 		data_len;		//消息数据的长度
	_u32 		key_click;		//键盘按键数
	_u32 		mou_click;		//鼠标点击数
	_u16        is_ciphertext;  //是否与密文相关事件
	char 		data[0];		//日志消息,也可以是图像数据,具体是什么根据消息类型来判断
}action_st;

typedef struct
{
	_u64 time;
	_u32 sender_account;
	_s8  sender[32];
	_u32 reciver_account;
	_s8  reciver[32];
	_u32 msg_len;
	_u32 issend;
	_s8  data[0];
}qq_msg_st;


typedef struct
{
	_u32 url_len;		// url长度
	_u32 title_len;		// 标题长度
	_u32 key_len;		// 关键字长度
	char data[0];		// 根据上面长度取内容
}browser_action_log_st;

typedef struct
{
	_u32 title_len;		// 标题长度
	char data[0];		// 根据上面长度取内容
}wnd_swtich_log_st;

#pragma pack()

#if JHL_SYS_TYPE == SYS_TYPE_X86
#define WW_CLIENT_UPDATE_PATH		"/root/hd/water_wall/"
#define WW_CLIENT_UPDATE_TXT_PATH	"/root/hd/water_wall/update.txt"
#define WW_CLIENT_UPDATE_RAR_PATH	"/root/hd/water_wall/update.rar"
#else
#define WW_CLIENT_UPDATE_PATH		"/root/water_wall/"
#define WW_CLIENT_UPDATE_TXT_PATH	"/root/water_wall/update.txt"
#define WW_CLIENT_UPDATE_RAR_PATH	"/root/water_wall/update.rar"
#endif

#define WW_CLIENT_UPDATE_TXT_URL	"ww_client_update.data?type=txt"
#define WW_CLIENT_UPDATE_RAR_URL	"ww_client_update.data?type=rar"

/// 日志分类
enum LOG_CATEGORY
{
	LC_GENERAL  = 0,      ///< 枚举，普通行为日志
	LC_WARNING = 1,     ///< 枚举，警告行为日志
	LC_FORBID    = 2,     ///< 枚举，禁止行为日志
};

//用户活动类型列表
enum USER_ACTION_TYPE
{
	OT_UNKNOWN_TYPE                  = 0,

	//下面的操作主要是针对文件加密部分


	//下面的操作主要是针对用户普通操作
	OT_PROCESS_CREATE       =100,          //启动一个新的进程,没有载荷数据
	OT_PROCESS_TERMINATE,                  //终止了一个进程，没有你载荷数据 
	OT_PROCESS_ATTEMPT_CREATE,             //试图创建一个进程，没有载荷数据 

	OT_BOOT_TIME,						   //开机时刻
	OT_USER_LEAVE,						   //用户离开

	//下面是窗口相关事件，具体内容见WND_SWITCH_LOG
	OT_WINDOW_OPEN = 150,                 //显示一个窗口, 
	OT_WINDOW_CLOSE,                          //关闭一个窗口
	OT_WINDOW_ATTEMPT_OPEN,            //企图打开一个禁止的窗口

	//下面是文件相关事件(一定要和驱动中的设置相同,否则在处理文件加密驱动事件的时候就出问题)
	OT_FILE_CREATE = 200,                      //创建一个新的文件,载荷数据是文件的完整路径
	OT_FILE_OPEN,                                  //打开一个文件,载荷数据是文件的完整路径
	OT_FILE_CLOSE,                                 //关闭一个文件,载荷数据是文件的完整路径
	OT_FILE_DELETE,                                //删除一个文件,载荷数据是文件的完整路径
	OT_FILE_ATTEMPT_DELETE,                 //企图删除一个文件,载荷数据是文件的完整路径
	OT_FILE_RENAME,                               //重命名一个文件,载荷数据是文件的完整路径
	OT_FILE_ATTEMPT_RENAME,                //企图重命名一个文件,载荷数据是文件的完整路径
	OT_FILE_ATTEMPT_DELETE_FOLDER,     //企图删除一个目录
	OT_FILE_ATTEMPT_RENAME_FOLDER,    //企图重命名一个目录
	OT_FILE_ATTEMPT_CREATE,                 //企图创建一个新的文件，但是因为权限没有成功,载荷数据是文件的完整路径
	OT_FILE_ATTEMPT_OPEN,                    //企图打开一个文件，但是因为权限没有成功,载荷数据是文件的完整路径
	OT_FILE_ATTEMPT_CREATE_PROCESS,    //企图创建一个新进程，但是因为权限没有成功,载荷数据是文件的完整路径
	OT_FILE_USB_PULL_IN,                //USB插入事件
	OT_FILE_USB_PULL_OUT,                      //USB拔出事件

	//下面是浏览器相关事件, 具体内容见BROWSER_ACTION_LOG
	OT_BROWSER_OPEN_URL = 300,          //打开了一个URL
	OT_BROWSER_CLOSE_URL,				  //关闭了一个URL
	OT_BROWSER_SEARCH,                     //搜索某个关键字
	OT_BROWSER_PLAY_GAME,               //玩网络游戏
	OT_BROWSER_PLAY_VIDEO,              //通过浏览器看视频
	OT_BROWSER_ATTEMPT_OPEN_URL,  //企图打开一个禁止的URL

	//下面是与视频播放相关的的时间
	OT_PLAY_A_FILE   = 400,                   //播放一个视频文件



	OT_SEND_MESSAGE_REQUEST,            //请求发送内部信息
	OT_RECEIVE_MESSAGE_REQUEST,        //请求接收内部消息
};


enum AN_CATEGORY
{
	AN_UNKNOWN_TYPE = 0				//未知
	//行为统计用
	,AN_CALC_ATTEMPT				//违规行为
	,AN_CALC_VIDEO					//影音娱乐
	,AN_CALC_CHAT					//聊天工具
	,AN_CALC_URL					//网页浏览
	,AN_CALC_WORK					//办公行为
	,AN_CALC_OTHER					//其他
	,AN_CALC_MAX
};

//针对页面关注的行为类型
enum auto_save_detail_act
{
	AN_DETAIL_UNKNOWN	= 0			//查询时,代表全部
	,AN_DETAIL_ATTEMPT 				//违规行为
	,AN_DETAIL_WORK					//办公行为
	,AN_DETAIL_URL_OPEN				//打开网页
	,AN_DETAIL_WINDOW_OPEN			//打开窗口
	,AN_DETAIL_PROCESS_RUN			//运行程序
	,AN_DETAIL_FILE_DEL				//文件删除
	,AN_DETAIL_FILE_CREATE			//文件新增或复制
	,AN_DETAIL_MAX
};


//目录访问控制设置
enum FOLDER_RW_CONTROL
{
	//读写控制
	CRWC_DISENABLE_RW                    = 0,                       //禁止读写
	CRWC_ENABLE_R                            = 1,   //允许读
	CRWC_ENABLE_W                           = 2,   //允许写

	//加密控制，设置了该选项，读写该目录中的文件，自动加解密
	CRWC_ENABLE_ENCIPHER                = CRWC_ENABLE_W + CRWC_ENABLE_R + 4,    //允许读写密文
	
	//目录中文件考出控制
	CRWC_ENABLE_C_PLAINTEXT           = CRWC_ENABLE_R + 16,        //允许考出非密文
	CRWC_ENABLE_C_CIPHERTEXT         = CRWC_ENABLE_R + 32,        //允许考出密文,不解密
	CRWC_ENABLE_C_CIPHERTEXT_EX    = CRWC_ENABLE_R + 64,        //允许考出密文，需要解密

	CRWC_ENABLE_REMOVE                  = 128,       //删除
	CRWC_ENABLE_RENAME                  = 256,       //重命名
	CRWC_ENABLE_PRINT                     = 512,        //打印
	
	CRWC_QUERY_ADMINISTRATOR      = 4095      //询问管理员
};

//用户访问加密文件的权限属性，可以是这些值中的任何一个，也可以是他们的联合
enum ACCESS_ENCRYPTION_FILE_AUTHORITY
{
	AEFA_READ_ONLY = 1,        //只读
	AEFA_EDIT      = 2,        //编辑
	AEFA_DOWNLOAD  = 4,        //下载	//暂忽视
	AEFA_OFF_LINE  = 8,        //离线	//暂忽视
	AEFA_REMOVE    = 16,       //删除
	AEFA_RENAME    = 32,       //重命名
	AEFA_PRINT     = 64,        //打印
	AEFA_ALL        = 127        //全部权限
};

//用户监视选项
enum USER_ACTION_MONITOR_OPTION
{
	UAMO_ENABLE_NOTHING = 0,
	UAMO_ENABLE_USER_ACTION_MONITOR = 1,		//用户的常规行为监视
	UAMO_ENABLE_USER_ACTION_CONTROL = 2,		//用户的常规行为控制
	UAMO_ENABLE_CAPTURE_QQMSG		= 4, 		//监视用户的QQ聊天信息

	UAMO_ENABLE_SHOW_WARNING_INFO = 8,			//是否显示警告消息
	UAMO_ENABLE_SHOW_FORBIDDING_INFO = 16		//是否显示静止消息
};

/*jhttpd与防水墙服务器通信协议

一、通信方式
UDP通信。jhttpd主动发给防水墙服务器，服务器并不做任何回复。

二、通信时机及通信内容。
当用户配置发生变化且用户在线时，由jhttpd发送具体的二进制数据给服务器，提示服务器下发更新数据给客户端。

三、数据格式
1.数据头*/
typedef struct
{
	_u32 magic;		//识别码,固定为:0XFCAABDE0
	_u32 cmd;		//消息命令,详见命令枚举
	_u32 data_len;	//变长数据长度
	_s8 data[0];	//这里面应该是一个 _u32 uids[nuid] 的数组,存放有所有需要更新设置的在线用户的uid	
}udp_head_t;

//.对于更新配置的命令,需要的参数仅仅为所有需要通知的在线用户的uid,cmd < 20 的都用这个结构体
typedef struct
{
	_u32 uid_count;
	_s8 data[0];
}users_t;
//注:所有参数均为网络序,包括uids数组中的uid。
//3.广播消息的结构体
typedef struct
{
	_u32 type;
	_u8  msg[512];
	users_t users;
}broadcast_t;
//四、命令枚举
enum UDP_CMD
{	
	UC_SETTINGS_UPDATE = 0,					//更新基础配置	
	UC_UPDATA_APPS_ENCRYPTION_LIST, 		//更新应用程序加密列表	
	UC_UPDATE_FILE_FORMAT_ENCRYPTION_LIST, 	//更新文件格式加密列表	
	UC_UPDATE_DIRECTORY_ENCRYPTION_LIST, 	//更新加密目录列表	
	UC_UPDATE_APP_CTRL_FILTER_LIST, 		//更新应用管控过滤器列表
	UC_UPDATE_URL_FILTER_LIST,				//更新url关键字列表	
	UC_UPDATE_WIN_KEY_FILTER_LIST,			//更新窗口关键字列表
	UC_CATCH_PROCESS_LIST,					//抓取进程列表
	UC_KICK_OFF_USER,						//踢某用户下线
	UC_UPDATE_UPGRADE_INFO,					//客户端的升级配置信息改变
	UC_UPDATE_BREAK_TIME_INFO,				//更新客户端休息时间
	UC_UPDATE_JUMP_URL_INFO,				//更新违规跳转链接
	UC_SERVER_BROADCAST_MESSAGE = 20		//发送一条广播消息
};

//五、待补充

#endif

