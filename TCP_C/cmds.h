/////////////////////////////////////////////////////////////////////
/// @file cmds.h
/// @brief 与WEB SERVER通信命令枚举定义
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#ifndef _CMDS_H
#define _CMDS_H "cmds.h"
/// @brief 与WEB SERVER的通信命令
/// 
enum az_task_order_enum
{
	AZ_UNKNOW = 0			///< 未知请求

	//用户信息相关
	,AZ_USER_ADD = 100		///< 添加用户
	,AZ_USER_DEL			///< 删除用户
	,AZ_USER_MOD			///< 修改用户
	,AZ_USER_GET			///< 查询单个用户信息
	,AZ_USER_GETALL 		///< 查询所有用户信息
	,AZ_USER_GETBASE		///< 查询员工基本信息
	,AZ_USER_EXPORT			///< 导出当前员工信息
	,AZ_USER_IMPORT			///< 导入员工信息
	,AZ_USER_IMPORT_ERR_CSV	///< 导出"导入错误信息"

	//组信息相关
	,AZ_GROUP_ADD = 200		///< 添加用户组
	,AZ_GROUP_DEL			///< 删除用户组
	,AZ_GROUP_MOD			///< 修改用户组
	,AZ_GROUP_GET			///< 查询单个用户组信息
	,AZ_GROUP_GETALL		///< 查询所有用户组信息
	,AZ_GROUP_GETBASE		///< 查询所有用户组基本信息(department.data---->{组ID|组名称})

	//文件加密信息相关
	,AZ_FILE_ADD = 300		///< 添加文件加密规则
	,AZ_FILE_DEL			///< 删除文件加密规则
	,AZ_FILE_MOD			///< 修改文件加密规则
	,AZ_FILE_GET			///< 获取某用户组文件加密规则
	,AZ_FILE_GETALL 		///< 获取全部用户组文件加密规则
	,AZ_FILE_GETAUTH		///< 获取全部用户组访问密文的权限
	,AZ_FILE_MODAUTH		///< 修改全部用户组访问密文的权限

	//QQ聊天记录
	,AZ_QQ_MSG_GETUSER = 400	///< 获取(查询)某个用户的聊天记录
	,AZ_QQ_MSG_GETGROUP			///< 获取(查询)某个用户组的聊天记录
	,AZ_QQ_MSG_GETQQ			///< 获取(查询)某个用户与具体QQ号码的聊天记录

	//用户动态数据(行为统计/行为记录)相关
	,AZ_DYNAMIC_DELALL = 500///< 删除某用户所有动态数据
	,AZ_DYNAMIC_GET			///< 获取某用户部分动态数据
	,AZ_DYNAMIC_GETALL		///< 获取某用户所有动态
	//行为记录
	,AZ_DETAIL_GETUSER		///< 获取(查询)某个用户的行为记录
	,AZ_DETAIL_GETGROUP		///< 获取(查询)某个用户组的行为记录
	,AZ_DETAIL_GETALL		///< 获取(查询)全部用户的行为记录
	,AZ_DETAIL_EXPORTUSER	///< 导出当前查询的用户的违规记录
	,AZ_DETAIL_EXPORTGROUP	///< 导出当前查询的用户组的违规记录
	,AZ_DETAIL_EXPORTALL	///< 导出当前查询的全部用户的违规记录
	//行为统计
	,AZ_CALC_GETUSER		///< 获取(查询)某个用户的行为统计
	,AZ_CALC_GETGROUP = 510	///< 获取(查询)某个用户组的行为统计
	,AZ_CALC_GETALL			///< 获取(查询)全部用户的行为统计
	,AZ_CALC_GETINDEX		///< 获取(查询)全部用户的行为统计 ->首页专用
	,AZ_CALC_GETUSER_JS		///< 获取某个用户的所有行为统计js
	,AZ_CALC_GETGROUP_JS	///< 获取某个用户组的所有用户行为统计js
	,AZ_CALC_GETALL_JS		///< 获取全部用户组的行为统计js
	,AZ_CALC_GETMONTH_JS	///< 获取首页所需的js
	//导出行为记录
	,AZ_DETAIL_EXPORT_CSV	///< 获取water_wall_record.csv(注:此处不会传opt,WEB服务器需直接把cmd传过来)
	//进程相关
	,AZ_GET_USER_PROCESS    ///< 获取当前用户的进程信息
	,AZ_SET_USER_PROCESS_STAT ///< 设置当前进程获取状态

	//办公行为相关
	,AZ_WORK_ADD = 600		///< 增加办公行为规则
	,AZ_WORK_DEL			///< 删除办公行为规则
	,AZ_WORK_MOD			///< 修改办公行为规则
	,AZ_WORK_GET			///< 获取某用户组办公行为规则
	,AZ_WORK_GETALL			///< 获取所有办公行为规则 -> 不使用
	,AZ_WORK_GETACT			///< 获取所有用户行为--->用以显示高频行为
	,AZ_WORK_GETJS			///< 获取ww_high_action.js

	//违规行为相关
	,AZ_FORBID_ADD = 700	///< 增加违规行为规则
	,AZ_FORBID_MOD			///< 修改违规行为规则
	,AZ_FORBID_DEL			///< 删除违规行为规则
	,AZ_FORBID_GET			///< 获取某用户组违规行为规则
	,AZ_FORBID_GETALL		///< 获取所有违规行为规则

	//违规警告相关
	,AZ_WARNING_MOD = 800	///< 修改违规警告

	//系统设置相关
	,AZ_SYSTEM_MOD = 900	///< 系统修改设置相关
	,AZ_SYSTEM_GET_ALL		///< 获取当前系统设置

	,AZ_CLIENT_UPDATA = 950   ///< 上传客户端升级文件
	//非网络请求,本地自用
	,LOCAL_AZ_USER_DEL_DYN = 1000	///< 删除用户全部动态数据
	,LOCAL_AZ_AUTO_SAVE				///< 定时处理统计
};

#endif // end of "worker.h"
