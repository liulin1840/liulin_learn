/////////////////////////////////////////////////////////////////////
/// @file allocator.c
/// @brief 解析TCP包,将JSON转struct,然后发给worker处理
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#include <arpa/inet.h>
#include <stdlib.h>


#include "hash.h"
#include "allocator.h"
#include "worker.h"
#include "debug.h"
#include "mem.h"
#include "cJSON.h"
#include "head.h"
#include "cmds.h"
#include "ww_action.h"
#include "public.h"
#include "prefix.h"


//----------------------------------------------------------------
// 函数名称 hex_to_char
/// @brief 将16进制字符串(2字节字符)转化为unsigned char
/// 
/// @param _in c1->高位
/// @param _in c2->地位
/// @return 经过转换的unsigned char
/// @author miaomiao
//---------------------------------------------------------------
static inline unsigned char hex_to_char(char c1,char c2)
{
	unsigned char ret=0x0;
	if((c1 >= '0') && (c1 <= '9'))
	{
		ret=c1-'0';
	}
	else if((c1 >= 'a') && (c1 <= 'f'))
	{
		ret=(c1-'a')+10;
	}
	else if((c1 >= 'A') && (c1 <= 'F'))
	{
		ret=(c1-'A')+10;
	}
	ret=ret*16;
	
	if((c2 >= '0') && (c2 <= '9'))
	{
		ret+=c2-'0';
	}
	else if((c2 >= 'a') && (c2 <= 'f'))
	{
		ret+=(c2-'a')+10;
	}
	else if((c2 >= 'A') && (c2 <= 'F'))
	{
		ret+=(c2-'A')+10;
	}

	return ret;
}
//----------------------------------------------------------------
// 函数名称 check_hex
/// @brief 检查是否16进制字符
/// 
/// @param _in c->需检查的字符
/// @return 1->是,2->否
/// @author miaomiao
//---------------------------------------------------------------
static inline int check_hex(char c)
{
	if(((c >= '0') && (c <= '9'))
		|| ((c >= 'a') && (c <= 'f'))
		|| ((c >= 'A') && (c <= 'F')))
		return 1;
	return 0;
}
//----------------------------------------------------------------
// 函数名称 __httpd_parm_do
/// @brief 将字符串的URI编码还原
/// 
/// @param _inout str->需要还原的字符串
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void __httpd_parm_do(char*str)
{
	char * tmp = str;
	char*ptr,*ptr2;
	while(*tmp)
	{
		if((*tmp == '%')
			&& check_hex(*(tmp+1))
			&& check_hex(*(tmp+2))
			)
		{
			*tmp=hex_to_char(*(tmp+1),*(tmp+2));
			ptr2=tmp+1;
			ptr = tmp+3;
			while(*ptr)
			{
				*ptr2=*ptr;
				ptr2++;
				ptr++;
			}
			*ptr2=0;
		}

		tmp++;
	}
}

static inline void snprintf_decodeURI(char* dest_buf, _s32 buf_size, _s8* fmt, _s8* src_buf)
{
	__httpd_parm_do(src_buf);
	snprintf(dest_buf, buf_size, fmt, src_buf);
}

//----------------------------------------------------------------
// 函数名称 al_parse_user_data
/// @brief 解析用户操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_parse_user_data(cJSON* json_root, wk_task_t* wk)
{
	cJSON *json_value = NULL;
	az_task_recv_user_t* user = (az_task_recv_user_t*)wk->data;

	if((json_value = cJSON_GetObjectItem(json_root, "uid")) != NULL) {
		user->uid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "remark")) != NULL) {
		snprintf_decodeURI(user->remark, sizeof(user->remark), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "account")) != NULL) {
		snprintf_decodeURI(user->account, sizeof(user->account), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "password")) != NULL) {
		snprintf_decodeURI(user->password, sizeof(user->password), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "old_gid")) != NULL) {
		user->old_gid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "new_gid")) != NULL) {
		user->new_gid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "bind")) != NULL) {
		user->bind = atoi(json_value->valuestring);
	}
	
}

//----------------------------------------------------------------
// 函数名称 al_parse_group_data
/// @brief 解析用户组操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_parse_group_data(cJSON* json_root, wk_task_t* wk)
{
	cJSON *json_value = NULL;
	az_task_recv_group_t* group = (az_task_recv_group_t*)wk->data;

	if((json_value = cJSON_GetObjectItem(json_root, "gid")) != NULL) {
		group->gid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "group_name")) != NULL) {
		snprintf_decodeURI(group->group_name, sizeof(group->group_name), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "chart")) != NULL) {
		group->chat = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "record")) != NULL) {
		group->record = atoi(json_value->valuestring);
	}
	
	if((json_value = cJSON_GetObjectItem(json_root, "control")) != NULL) {
		group->control = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "usb_auth")) != NULL) {
		group->usb_auth = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "cd_auth")) != NULL) {
		group->cd_auth = atoi(json_value->valuestring);
	}

}

//----------------------------------------------------------------
// 函数名称 al_parse_file_data
/// @brief 解析文件加密操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author 李杰明
//---------------------------------------------------------------
void al_parse_file_data(cJSON* json_root, wk_task_t* wk)
{
	cJSON *json_value = NULL;
	az_task_recv_file_t* file = (az_task_recv_file_t*)wk->data;

	if((json_value = cJSON_GetObjectItem(json_root, "old_gid")) != NULL) {
		file->old_gid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "old_filename")) != NULL) {
		snprintf_decodeURI(file->old_filename ,sizeof(file->old_filename) ,"%s" ,json_value->valuestring);
	}

	if ((json_value = cJSON_GetObjectItem(json_root, "new_gid")) != NULL){
		file->new_gid = atoi(json_value->valuestring);
	}

	if ((json_value = cJSON_GetObjectItem(json_root, "new_filename")) != NULL){
		snprintf_decodeURI(file->new_filename, sizeof(file->new_filename), "%s", json_value->valuestring);
	}
	
	if((json_value = cJSON_GetObjectItem(json_root, "authority")) != NULL) {
		file->authority = ntohl(atoi(json_value->valuestring));
	}else{
		file->authority = AEFA_ALL;
	}

	if((json_value = cJSON_GetObjectItem(json_root, "filetype")) != NULL) {
		file->filetype = atoi(json_value->valuestring);
	}
	
}

//----------------------------------------------------------------
// 函数名称 al_parse_encrypt_data
/// @brief 解析密文权限操作相关数据
/// 
/// 数据参考:{
///  "group_num":"5"		//5个组
///  ,"8":"auth"			//组ID:权限
/// }
/// AEFA_READ_ONLY			//查看
/// AEFA_EDIT | AEFA_RENAME	//修改
/// AEFA_REMOVE				//删除
/// AEFA_PRINT				//打印
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_parse_encrypt_data(cJSON* json_root, wk_task_t* wk)
{
	cJSON *json_value = NULL;
	az_task_recv_encrypts_t* enc_s = (az_task_recv_encrypts_t*)wk->data;
	az_task_recv_encrypt_t* enc = (az_task_recv_encrypt_t*)enc_s->data;

	_s32 i = 0;
	_s32 group_count = 0;
	_s8 json_key[32];

	if((json_value = cJSON_GetObjectItem(json_root, "group_num")) != NULL) {
		enc_s->group_num = atoi(json_value->valuestring);
	}
	
	for(i = 0; group_count < enc_s->group_num; i++)
	{
		snprintf(json_key, sizeof(json_key), "%d", i);
		if((json_value = cJSON_GetObjectItem(json_root, json_key)) != NULL) {
			enc->gid = i;
			enc->authority = htonl(atoi(json_value->valuestring));
			enc++;
			group_count++;
		}
	}
	
	return;
}

//----------------------------------------------------------------
// 函数名称 al_parse_qq_msg_data
/// @brief 解析聊天记录操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_parse_qq_msg_data(cJSON* json_root, wk_task_t* wk)
{
	cJSON *json_value = NULL;
	az_task_recv_qq_t* qq = (az_task_recv_qq_t*)wk->data;

	if((json_value = cJSON_GetObjectItem(json_root, "uid")) != NULL) {
		qq->uid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "gid")) != NULL) {
		qq->gid= atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "qq_id")) != NULL) {
		qq->qq_id = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "start_time")) != NULL) {
		qq->start_time = (GMT_REVERS_MASK-atoull(json_value->valuestring));
	}

	if((json_value = cJSON_GetObjectItem(json_root, "end_time")) != NULL) {
		qq->end_time = (GMT_REVERS_MASK-atoull(json_value->valuestring));
	}

	if((json_value = cJSON_GetObjectItem(json_root, "page_flag")) != NULL) {
		qq->page_flag= atoi(json_value->valuestring);
	}

	if ((json_value = cJSON_GetObjectItem(json_root, "keyword")) != NULL){
		snprintf_decodeURI(qq->keyword, sizeof(qq->keyword), "%s", json_value->valuestring);
	}
}

//----------------------------------------------------------------
// 函数名称 al_parse_process_data
/// @brief 解析进程列表操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author 李杰明
//---------------------------------------------------------------
void al_parse_process_data(cJSON*json_root, wk_task_t*wk)
{
	cJSON* json_value = NULL;
	_u32* userid = (_u32*)wk->data;
	if ((json_value = cJSON_GetObjectItem(json_root, "uid")) != NULL){
		*userid = atoi(json_value->valuestring);
	}
}

//----------------------------------------------------------------
// 函数名称 al_parse_detail_data
/// @brief 解析进程列表操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_parse_detail_data(cJSON* json_root, wk_task_t* wk)
{
	cJSON *json_value = NULL;
	az_task_recv_detail_t* detail = (az_task_recv_detail_t*)wk->data;

	if((json_value = cJSON_GetObjectItem(json_root, "uid")) != NULL) {
		detail->uid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "gid")) != NULL) {
		detail->gid= atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "act_type")) != NULL) {
		detail->act_type = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "start_time")) != NULL) {
		detail->start_time = (GMT_REVERS_MASK-atoull(json_value->valuestring));
	}

	if((json_value = cJSON_GetObjectItem(json_root, "end_time")) != NULL) {
		detail->end_time = (GMT_REVERS_MASK-atoull(json_value->valuestring));
	}

	if((json_value = cJSON_GetObjectItem(json_root, "page_flag")) != NULL) {
		detail->page_flag= atoi(json_value->valuestring);
	}
}

//----------------------------------------------------------------
// 函数名称 al_parse_calc_data
/// @brief 解析行为统计操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_parse_calc_data(cJSON* json_root, wk_task_t* wk)
{
	cJSON *json_value = NULL;
	az_task_recv_calc_t* calc = (az_task_recv_calc_t*)wk->data;

	if((json_value = cJSON_GetObjectItem(json_root, "uid")) != NULL) {
		calc->uid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "gid")) != NULL) {
		calc->gid= atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "start_time")) != NULL) {
		calc->start_time = (GMT_REVERS_MASK-atoull(json_value->valuestring));
	}

	if((json_value = cJSON_GetObjectItem(json_root, "end_time")) != NULL) {
		calc->end_time = (GMT_REVERS_MASK-atoull(json_value->valuestring));
	}

	if((json_value = cJSON_GetObjectItem(json_root, "page_flag")) != NULL) {
		calc->page_flag= atoi(json_value->valuestring);
	}
}

//----------------------------------------------------------------
// 函数名称 al_parse_work_data
/// @brief 解析办公行为操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_parse_work_data(cJSON* json_root, wk_task_t* wk)
{
	enum{
		gid_id = 0  //组id
		,oldgid_id	//旧的组id
		,type_id	//类型 0:进程  1:域名  100:所有类型
		,process_id	//进程名
		,descrp_id	//行为描述/域名
		,time_id	//时间
		,max_id
	};


	cJSON *json_value = NULL;
	az_task_recv_work_acts_t* works = (az_task_recv_work_acts_t*)wk->data;
	az_task_recv_work_act_t* work = (az_task_recv_work_act_t*)works->data;

	_s32 i = 0;
	_s8 json_key[32];

	_s8* carg[max_id] = {};
	_s32 cnum = 0;
	_s8 temp_buf[1024];
	
	if((json_value = cJSON_GetObjectItem(json_root, "listcount")) != NULL) {
		works->listcount = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "gid")) != NULL) {
		works->gid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "type")) != NULL) {
		works->type = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "describe")) != NULL) {
		snprintf_decodeURI(works->describe, sizeof(works->describe), "%s", json_value->valuestring);
	}
	
	for(i = 0; i < works->listcount; i++)
	{
		snprintf(json_key, sizeof(json_key), "%d", i);
		if((json_value = cJSON_GetObjectItem(json_root, json_key)) != NULL) {
			snprintf_decodeURI(temp_buf,sizeof(temp_buf),"%s",json_value->valuestring);
			INFO("temp_buf = %s\n",temp_buf);
			cnum = split_string(temp_buf, '|', carg, max_id);
			INFO("cnum = %d\n",cnum);
			if(cnum >= descrp_id)
			{
				work->gid = atoi(carg[gid_id]);
				work->old_gid = atoi(carg[oldgid_id]);
				work->worktype = atoi(carg[type_id]);
				snprintf(work->process, sizeof(work->process), "%s", carg[process_id]);
				snprintf(work->workname, sizeof(work->workname), "%s", carg[descrp_id]);
				INFO("process = %s\n",work->process);
				INFO("workname = %s\n",work->workname);
				work++;
			}
		}
	}
}

//----------------------------------------------------------------
// 函数名称 al_parse_forbid_data
/// @brief 解析违规行为操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author 李杰明
//---------------------------------------------------------------
void al_parse_forbid_data(cJSON* json_root, wk_task_t* wk)
{
	cJSON *json_value = NULL;
	az_task_recv_forbidact_t* forbid = (az_task_recv_forbidact_t*)wk->data;

	if((json_value = cJSON_GetObjectItem(json_root, "old_name")) != NULL) {
		snprintf_decodeURI(forbid->old_name, sizeof(forbid->old_name), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "new_name")) != NULL) {
		snprintf_decodeURI(forbid->new_name, sizeof(forbid->new_name), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "old_gid")) != NULL) {
		forbid->old_gid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "new_gid")) != NULL) {
		forbid->new_gid = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "act_type")) != NULL) {
		forbid->act_type = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "is_forbid")) != NULL) {
		forbid->is_forbid = atoi(json_value->valuestring);
	}

	if ((json_value = cJSON_GetObjectItem(json_root, "search_flag")) != NULL){
		forbid->search_flag = atoi(json_value->valuestring);
	}
}

//----------------------------------------------------------------
// 函数名称 al_parse_waring_data
/// @brief 解析违规行为操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author 李杰明
//---------------------------------------------------------------
void al_parse_waring_data(cJSON* json_root, wk_task_t* wk)
{
	cJSON *json_value = NULL;
	az_task_recv_warning_msg_t* warning = (az_task_recv_warning_msg_t*)wk->data;

	if((json_value = cJSON_GetObjectItem(json_root, "warning_msg")) != NULL) {
		snprintf_decodeURI(warning->warning_msg, sizeof(warning->warning_msg), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "error_msg")) != NULL) {
		snprintf_decodeURI(warning->error_msg, sizeof(warning->error_msg), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "warning_en")) != NULL) {
		warning->warning_en = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "error_en")) != NULL) {
		warning->error_en = atoi(json_value->valuestring);
	}
	
}

//----------------------------------------------------------------
// 函数名称 al_parse_system_data
/// @brief 解析系统设置操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_parse_system_data(cJSON* json_root, wk_task_t* wk)
{
	cJSON *json_value = NULL;
	az_task_recv_system_t* sys = (az_task_recv_system_t*)wk->data;
	
	if((json_value = cJSON_GetObjectItem(json_root, "breaktime")) != NULL) {
		snprintf_decodeURI(sys->breaktime, sizeof(sys->breaktime), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "jumpurl")) != NULL) {
		snprintf_decodeURI(sys->jumpurl, sizeof(sys->jumpurl), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "password")) != NULL) {
		snprintf_decodeURI(sys->password, sizeof(sys->password), "%s", json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "login_bind")) != NULL) {
		sys->login_bind = atoi(json_value->valuestring);
	}
	if ((json_value = cJSON_GetObjectItem(json_root, "start_time")) != NULL){
		sys->start_time = atoll(json_value->valuestring);
	}
	if ((json_value = cJSON_GetObjectItem(json_root, "end_time")) != NULL){
		sys->end_time = atoll(json_value->valuestring);
	}
	if ((json_value = cJSON_GetObjectItem(json_root, "t_start_time")) != NULL){
		sys->t_start_time = atoll(json_value->valuestring);
	}
	if ((json_value = cJSON_GetObjectItem(json_root, "t_end_time")) != NULL){
		sys->t_end_time = atoll(json_value->valuestring);
	}
	if ((json_value = cJSON_GetObjectItem(json_root, "work_day")) != NULL){
		sys->work_day = atoi(json_value->valuestring);
	}
}

//----------------------------------------------------------------
// 函数名称 al_parse_client_data
/// @brief 解析客户端更新操作相关数据
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author 黄华
//---------------------------------------------------------------
void al_parse_client_data(cJSON* json_root, wk_task_t* wk)
{
	update_info_t *ut = (update_info_t*)wk->data;
	cJSON *json_value = NULL;
	char    port[8] = {0};
	char    ip[64] = {0};
	if ((json_value = cJSON_GetObjectItem(json_root, "ip")) != NULL){
		snprintf(ip,64,"%s",json_value->valuestring);
	}
	if ((json_value = cJSON_GetObjectItem(json_root, "port")) != NULL){
		snprintf(port,8,"%s",json_value->valuestring);
	}
	if ((json_value = cJSON_GetObjectItem(json_root, "txt_name")) != NULL){
		snprintf(ut->version_info,1024,"http://%s:%s/%s",ip,port,json_value->valuestring);
	}
	if ((json_value = cJSON_GetObjectItem(json_root, "rar_name")) != NULL){
		snprintf(ut->upgrade_url,1024,"http://%s:%s/%s",ip,port,json_value->valuestring);
	}
}

//----------------------------------------------------------------
// 函数名称 al_parse_import_data
/// @brief 解析规则导入文件内容
/// 
/// @param _in json_root->包含原始数据的cjson根
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_parse_import_data(cJSON* json_root, wk_task_t* wk)
{
	_s8* filedata = wk->data;
	cJSON *json_value = NULL;

	if ((json_value = cJSON_GetObjectItem(json_root, "filedata")) != NULL){
		sprintf(filedata,"%s",json_value->valuestring);//此处内存空间肯定够
	}
}

//----------------------------------------------------------------
// 函数名称 al_parse_value_to_wk_task
/// @brief 解析数据放入wk中,并发送给worker处理
/// 
/// @param _in json_data->经过TCP切包后的原始数据
/// @param _inout wk->按格式接收解析后的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_parse_value_to_wk_task(_s8* json_data, wk_task_t* wk)
{
	cJSON *json_root = NULL, *json_value = NULL;
	char* p_space = json_data;
	wk_head_t* head = &wk->head;
	an_web_cache_t* web_cache = NULL;

	
	//还原空格字符
	while(*p_space)
	{
		if(*p_space == '+')
			*p_space = ' ';
		p_space++;
	}

	json_root = cJSON_Parse(json_data); // json 根
	if (json_root == NULL) {
		INFO("parse json root error\n");
		goto END;
	}

	// 消息
	if((json_value = cJSON_GetObjectItem(json_root, "cmd")) != NULL) {
		INFO("cmd = %d\n",atoi(json_value->valuestring));
		head->cmd = atoi(json_value->valuestring);
	}

	if((json_value = cJSON_GetObjectItem(json_root, "token")) != NULL) {
		INFO("token = %d\n",atoi(json_value->valuestring));
		head->token= atoi(json_value->valuestring);
	}

	web_cache = an_web_cache_t_find_create_by_token(head->token);
	
	if(NULL == web_cache)
	{
		DEBUG("web cache can't create\n");
		goto END;
	}

	mod_timer(&web_cache->timeout, jiffies + COOKIE_AGEING_TIME * HZ);

	switch(wk->head.cmd)	
	{
		case AZ_USER_ADD:
		case AZ_USER_MOD:
		case AZ_USER_DEL:
		case AZ_USER_GET:
		case AZ_USER_GETALL:
		case AZ_USER_GETBASE:
			al_parse_user_data(json_root,wk);
			break;
		case AZ_USER_IMPORT:
			al_parse_import_data(json_root,wk);
			break;
		case AZ_GROUP_ADD:
		case AZ_GROUP_DEL:
		case AZ_GROUP_MOD:
		case AZ_GROUP_GET:
		case AZ_GROUP_GETALL:
		case AZ_GROUP_GETBASE:
			al_parse_group_data(json_root,wk);
			break;
		case AZ_FILE_ADD:
		case AZ_FILE_DEL:
		case AZ_FILE_MOD:
		case AZ_FILE_GET:
		case AZ_FILE_GETALL:
			al_parse_file_data(json_root,wk);
			break;
		case AZ_FILE_GETAUTH:
		case AZ_FILE_MODAUTH:
			al_parse_encrypt_data(json_root,wk);
			break;
		case AZ_QQ_MSG_GETUSER:
		case AZ_QQ_MSG_GETGROUP:
		case AZ_QQ_MSG_GETQQ:
			// TODO:放这里可能对效率影响很大,测试后考虑优化
			ww_add_or_init_qq_msg_to_list(0);
			al_parse_qq_msg_data(json_root,wk);
			break;
		case AZ_DETAIL_GETUSER:
		case AZ_DETAIL_GETGROUP:
		case AZ_DETAIL_GETALL:
		case AZ_DETAIL_EXPORTUSER:
		case AZ_DETAIL_EXPORTGROUP:
		case AZ_DETAIL_EXPORTALL:
			// TODO:放这里可能对效率影响很大,测试后考虑优化
			ww_add_or_init_detail_behavior_to_list();
			al_parse_detail_data(json_root,wk);
			break;
		case AZ_GET_USER_PROCESS:
			al_parse_process_data(json_root, wk);
			break;
		case AZ_SET_USER_PROCESS_STAT:
			al_parse_process_data(json_root, wk);
			break;
		case AZ_CALC_GETUSER:
		case AZ_CALC_GETGROUP:
		case AZ_CALC_GETALL:
		case AZ_CALC_GETINDEX:
			// TODO:放这里可能对效率影响很大,测试后考虑优化
			ww_add_or_init_calc_behavior_to_list();
		case AZ_CALC_GETGROUP_JS:
		case AZ_CALC_GETALL_JS:
		case AZ_CALC_GETMONTH_JS:
		case AZ_CALC_GETUSER_JS:
			al_parse_calc_data(json_root,wk);
			break;
		case AZ_WORK_ADD:
		case AZ_WORK_DEL:
		case AZ_WORK_MOD:
		case AZ_WORK_GET:
		case AZ_WORK_GETALL:
		case AZ_WORK_GETACT:
			al_parse_work_data(json_root,wk);
			break;
		case AZ_FORBID_ADD:
		case AZ_FORBID_MOD:
		case AZ_FORBID_DEL:
		case AZ_FORBID_GET:
		case AZ_FORBID_GETALL:
			al_parse_forbid_data(json_root,wk);
			break;
		case AZ_WARNING_MOD:
			al_parse_waring_data(json_root,wk);
			break;
		case AZ_SYSTEM_MOD:
			al_parse_system_data(json_root,wk);
			break;
		case AZ_CLIENT_UPDATA:
			al_parse_client_data(json_root,wk);
			break;
	}

	INFO("parse ok\n");

	wk_add_task(wk);

END:
	cJSON_Delete(json_root);
	json_root = NULL;
	return;
}

//----------------------------------------------------------------
// 函数名称 al_allocate_task
/// @brief 解析TCP包中的参数,完成解析后交给worker处理
/// 
/// @param _in data->经过TCP切包后的原始数据
/// @param _inout data_len->data的长度
/// @param _in index->an_conn_array数组中的下标
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void al_allocate_task(_s8* data, _s32 data_len, _s32 index)
{
	//错误还需发送错误信息
	wk_task_t* wk = wys_malloc(sizeof(wk_task_t) + data_len + 1024);
	if(!wk)
	{
		return;
	}
	INFO("allocate recv data:%s\n",data);
	wk->head.fd_index = index;
	al_parse_value_to_wk_task(data, wk);

	wys_free(data);
	return;
}

