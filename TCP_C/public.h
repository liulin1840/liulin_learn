/////////////////////////////////////////////////////////////////////
/// @file public.h
/// @brief 公共函数提供:时间转换,字符串分解等
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#ifndef _PUBLIC_H
#define _PUBLIC_H "public.h"
#include <time.h>
#include <arpa/inet.h>

#include "types.h"
#include "hash.h"

/*判断该字节是否汉字字节,连续两个字节都是汉字字节才代表字符串中带有汉字*/
#define is_cn_char(p) ((_u8)p >= 161)?1:0

#define GMT_REVERS_MASK 0xffffffffull

extern _s32 gl_send_len;
extern _s32 max_send_len;
extern _s32 max_buf_len;

/// @brief 标准json_data特殊字符转换
void an_change_html_lt_to_dst(_s8* src,_s8* dst, _u32 dst_len);
/// @brief 检查字符串中是否有中文字符
_s32 check_string_have_cn_char(_s8* str, _s32 len);
/// @brief 根据关键字符分解字符串
_s32 split_string(_s8 * rule_str,_s8 ge,_s8 **array,int max);
/// @brief 非标准json_data特殊字符转换(主要用于三代页面框架)
_s8 *get_json_data_safe(const _s8 *org,_s8*buf);
/// @brief 发送tcp消息前,标准json_data特殊字符转换
_s8 an_get_json_data_safe(an_socket_t* sock_t);
/// @brief 将字符串中的'\"'替换成'\''
void an_change_json_data_symbo(_s8* buf);
/// @brief 标准json_data特殊字符转换
void an_change_json_data_symbo_to_dst(_s8* src,_s8* dst, _u32 dst_len);
/// @brief 获取逆序GMT当前小时0分0秒时的GMT时间
_u64 an_gmt_format_to_hour_inver(time_t gmt);
/// @brief 获取逆序GMT当天0时0分0秒时的GMT时间
_u64 an_gmt_format_to_day_inver(time_t gmt);
/// @brief 根据GMT时间得到该时间点所在年份的哪一天,区间[1-366]
_u32 an_gmt_to_yday(time_t gmt);
/// @brief 获取具体日期的GMT时间
_u64 an_time_to_gmt(_u32 year, _u32 mon, _u32 day, _u32 hour, _u32 min, _u32 sec);
/// @brief 根据GMT时间获取具体日期
void an_gmt_to_time(time_t gmt, _u32* year, _u32* mon, _u32* day, _u32* hour, _u32* min, _u32* sec);
/// @brief 字符串转unsigned long long int
_u64 atoull(_s8 * buf);
/// @brief 64位网络序转本地序
_u64 ntohll(_u64 net_var);
/// @brief 64位本地序转网络序
_u64 htonll(_u64 host_var);

void m_sock_add_write(_s32 index);
void an_exit(int line, int flag);
#endif // end of "public.h"

