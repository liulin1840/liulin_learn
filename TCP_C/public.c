/////////////////////////////////////////////////////////////////////
/// @file public.c
/// @brief 公共函数存放:时间转换,字符串分解等
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>

#include "public.h"
#include "mem.h"

#define _U	0x01	/* upper */
#define _L	0x02	/* lower */
#define _D	0x04	/* digit */
#define _C	0x08	/* cntrl */
#define _P	0x10	/* punct */
#define _S	0x20	/* white space (space/lf/tab) */
#define _X	0x40	/* hex digit */
#define _SP	0x80	/* hard space (0x20) */

unsigned char _ctype[] = {
_C,_C,_C,_C,_C,_C,_C,_C,			/* 0-7 */
_C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C,		/* 8-15 */
_C,_C,_C,_C,_C,_C,_C,_C,			/* 16-23 */
_C,_C,_C,_C,_C,_C,_C,_C,			/* 24-31 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,			/* 32-39 */
_P,_P,_P,_P,_P,_P,_P,_P,			/* 40-47 */
_D,_D,_D,_D,_D,_D,_D,_D,			/* 48-55 */
_D,_D,_P,_P,_P,_P,_P,_P,			/* 56-63 */
_P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U,	/* 64-71 */
_U,_U,_U,_U,_U,_U,_U,_U,			/* 72-79 */
_U,_U,_U,_U,_U,_U,_U,_U,			/* 80-87 */
_U,_U,_U,_P,_P,_P,_P,_P,			/* 88-95 */
_P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L,	/* 96-103 */
_L,_L,_L,_L,_L,_L,_L,_L,			/* 104-111 */
_L,_L,_L,_L,_L,_L,_L,_L,			/* 112-119 */
_L,_L,_L,_P,_P,_P,_P,_C,			/* 120-127 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		/* 128-143 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		/* 144-159 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,   /* 160-175 */
_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,       /* 176-191 */
_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,       /* 192-207 */
_U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,       /* 208-223 */
_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,       /* 224-239 */
_L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L};      /* 240-255 */

#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define J_isalnum(c)	((__ismask(c)&(_U|_L|_D)) != 0)
#define J_isalpha(c)	((__ismask(c)&(_U|_L)) != 0)
#define J_iscntrl(c)	((__ismask(c)&(_C)) != 0)
#define J_isdigit(c)	((__ismask(c)&(_D)) != 0)
#define J_isgraph(c)	((__ismask(c)&(_P|_U|_L|_D)) != 0)
#define J_islower(c)	((__ismask(c)&(_L)) != 0)
#define J_isprint(c)	((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
#define J_ispunct(c)	((__ismask(c)&(_P)) != 0)
#define J_isspace(c)	((__ismask(c)&(_S)) != 0)
#define J_isupper(c)	((__ismask(c)&(_U)) != 0)
#define J_isxdigit(c)	((__ismask(c)&(_D|_X)) != 0)

#define J_isascii(c) (((unsigned char)(c))<=0x7f)
#define J_toascii(c) (((unsigned char)(c))&0x7f)

static inline unsigned char __tolower(unsigned char c)
{
	if (J_isupper(c))
		c -= 'A'-'a';
	return c;
}

static inline unsigned char __toupper(unsigned char c)
{
	if (J_islower(c))
		c -= 'a'-'A';
	return c;
}

#define J_tolower(c) __tolower(c)
#define J_toupper(c) __toupper(c)

//----------------------------------------------------------------
// 函数名称 split_string
/// @brief 根据关键字符分解字符串
/// 
/// @param _in rule_str->需要分解的原始数据(会破坏原始数据)
/// @param _in ge->关键字符
/// @param _out array->指针数组,用于接收分解后的字符串
/// @param _in max->array数组成员个数
/// @return 本地序unsigned long long int数据
/// @author miaomiao
//---------------------------------------------------------------
_s32 split_string(_s8 * rule_str,_s8 ge,_s8 **array,int max)
{
	_s32 i=0,j=0;
	_s8 *cp = rule_str;	
	_s32 is_ge=0;

	for(j=0;j<max;j++)
		array[j] = NULL;
	
	while(cp && *cp && i < max)  //::
	{
		array[i++] = cp;
		is_ge=0;
		while(*cp && *cp != ge)
		{
			cp++;		
		}
		//cp= strchr(cp, ge);
		if(*cp)
		{
			is_ge = 1;
			*cp++=0;
		}
	}
	if((is_ge == 1) && (i < max))
		array[i++] = cp;
	return i;	
}

//----------------------------------------------------------------
// 函数名称 check_string_have_cn_char
/// @brief 检查字符串中是否有中文字符
/// 
/// @param _in str->需要检查的字符串
/// @param _in len->字符串长度(不含'\0')
/// @return 0:无,1:有
/// @author miaomiao
//---------------------------------------------------------------
_s32 check_string_have_cn_char(_s8* str, _s32 len)
{
	_s32 ret = 0;
	_s32 i = 0;

	if(len > 0)
	{
		for(i = 0; i < len - 1; i++)
		{
			if(is_cn_char(str[i]))
			{
				ret = 1;
				goto OUT;
			}
		}
	}

OUT:
	return ret;
}

//----------------------------------------------------------------
// 函数名称 get_json_data_safe
/// @brief 非标准json_data特殊字符转换(主要用于三代页面框架)
/// 
/// @param _in org->需要处理的原始数据
/// @param _out buf->接收处理后的数据
/// @return org->数据无需处理,buf->数据经过处理
/// @note buf指向的内存大小必须是strlen(org)的2倍以上，否则可能越界访问
/// @author miaomiao
//---------------------------------------------------------------
_s8 *get_json_data_safe(const _s8 *org,_s8*buf)
{
	_u8 c;	
	_s8 *b;
	const _s8*s=org;
	_s32 ismod = 0;
	b = buf;
	while ((c = *s++) != 0) {
		switch(c)
		{
			case '\'':
				b += sprintf(b, "\\'");
				ismod = 1;
				break;
			case '\n':
				b += sprintf(b, "\\n");
				ismod = 1;
				break;
			case '\r':
				b += sprintf(b, "\\r");
				ismod = 1;
				break;
			case '\"':
				b += sprintf(b, "\\\"");
				ismod = 1;
				break;
			case '[':
				b += sprintf(b, "\\[");
				ismod = 1;
				break;
			case ']':
				b += sprintf(b, "\\]");
				ismod = 1;
				break;
			case '{':
				b += sprintf(b, "\\{");
				ismod = 1;
				break;
			case '}':
				b += sprintf(b, "\\}");
				ismod = 1;
				break;
			case ':':
				b += sprintf(b, "\\:");
				ismod = 1;
				break;
			case ';':
				b += sprintf(b, "\\;");
				ismod = 1;
				break;
			case '|':
				b += sprintf(b, "\\|");
				ismod = 1;
				break;
			case '\\':
				b += sprintf(b, "\\\\");
				ismod = 1;
				break;
			case ',':
				b += sprintf(b, "\\,");
				ismod = 1;
				break;
			case '.':
				b += sprintf(b, "\\.");
				ismod = 1;
				break;
			case '/':
				b += sprintf(b, "\\/");
				ismod = 1;
				break;
			case '(':
				b += sprintf(b, "\\(");
				ismod = 1;
				break;
			case ')':
				b += sprintf(b, "\\)");
				ismod = 1;
				break;
			case '\t':
				b += sprintf(b, "\\t");
				ismod = 1;
				break;	
			default:
				*b++ = c;
				break;
		}

	}
	*b = 0;

	if(ismod)
		return buf;
	
	return (_s8*)org;
}

//----------------------------------------------------------------
// 函数名称 an_change_json_data_symbo
/// @brief 将字符串中的'\"'替换成'\''
/// 
/// @param _inout buf->需要处理的数据
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_change_json_data_symbo(_s8* buf)
{
	char *p = NULL;
	if(buf && strlen(buf))
	{
		p = buf;
		while(*p)
		{
			if(*p == '\"')
				*p='\'';
			p++;
		}
	}
}

//----------------------------------------------------------------
// 函数名称 an_change_html_lt_to_dst
/// @brief 标准json_data特殊字符转换
/// 
/// @param _in src->需要处理的原始数据
/// @param _in dst->接收处理后的数据
/// @param _in dst_len->dst所占内存大小(防止越界访问)
/// @return void->无返回值
/// @note analyzer模块中,仅用于获取行为记录时使用,主要替换'<'符号
/// @author miaomiao
//---------------------------------------------------------------
void an_change_html_lt_to_dst(_s8* src,_s8* dst, _u32 dst_len)
{
	_s8 *p = NULL;
	_u32 len = 0;
	_u8 c;
	memset(dst,0,dst_len);
	if(src && strlen(src))
	{
		p = src;
		while((c = *p++) != 0)
		{
			switch(c)
			{
				case '\\':
					len += snprintf(dst+len,dst_len-len,"\\\\");
					break;
				case '\n':
					len += snprintf(dst+len,dst_len-len,"\\n");
					break;
				case '\r':
					len += snprintf(dst+len,dst_len-len,"\\r");
					break;
				case '\t':
					len += snprintf(dst+len,dst_len-len,"\\t");
					break;
				case '\b':
					len += snprintf(dst+len,dst_len-len,"\\b");
					break;
				case '<':
					len += snprintf(dst+len,dst_len-len,"&lt;");
					break;
				default:
					//不可见字符
					if(J_iscntrl(c))
					{
						break;
					}
					else
					{
						len += snprintf(dst+len,dst_len-len,"%c",c);
						break;
					}
			}
			if(dst_len - len <= 1)
				return;
		}
	}
}

//----------------------------------------------------------------
// 函数名称 an_change_json_data_symbo_to_dst
/// @brief 标准json_data特殊字符转换
/// 
/// @param _in src->需要处理的原始数据
/// @param _in dst->接收处理后的数据
/// @param _in dst_len->dst所占内存大小(防止越界访问)
/// @return void->无返回值
/// @note analyzer模块中,仅用于获取聊天记录时使用,其他消息参考see
/// @see an_get_json_data_safe public.c
/// @author miaomiao
//---------------------------------------------------------------
void an_change_json_data_symbo_to_dst(_s8* src,_s8* dst, _u32 dst_len)
{
	_s8 *p = NULL;
	_u32 len = 0;
	_u8 c;
	memset(dst,0,dst_len);
	if(src && strlen(src))
	{
		p = src;
		while((c = *p++) != 0)
		{
			switch(c)
			{
				case '\"':
					len += snprintf(dst+len,dst_len-len,"\\\"");
					break;
				case '\\':
					len += snprintf(dst+len,dst_len-len,"\\\\");
					break;
				case '\n':
					len += snprintf(dst+len,dst_len-len,"\\n");
					break;
				case '\r':
					len += snprintf(dst+len,dst_len-len,"\\r");
					break;
				case '\t':
					len += snprintf(dst+len,dst_len-len,"\\t");
					break;
				case '\b':
					len += snprintf(dst+len,dst_len-len,"\\b");
					break;
				default:
					//不可见字符
					if(J_iscntrl(c))
					{
						break;
					}
					else
					{
						len += snprintf(dst+len,dst_len-len,"%c",c);
						break;
					}
			}
			if(dst_len - len <= 1)
				return;
		}
	}
}

//----------------------------------------------------------------
// 函数名称 an_get_json_data_safe
/// @brief 发送tcp消息前,标准json_data特殊字符转换
/// 
/// @param _inout sock_t->需要处理的an_socket_t结构体
/// @return -1->failed,0->sucess
/// @author miaomiao
//---------------------------------------------------------------
_s8 an_get_json_data_safe(an_socket_t* sock_t)
{
	_u8 c;	
	_s8 *b;
	const _s8*s=sock_t->org_send_buf;
	_u32 send_len = 0;
	b = sock_t->send_buf;
	while ((c = *s++) != 0) {
		switch(c)
		{
			case '\\':
				b += sprintf(b, "\\\\");
				send_len += 2;
				break;
			case '\n':
				b += sprintf(b, "\\n");
				send_len += 2;
				break;
			case '\r':
				b += sprintf(b, "\\r");
				send_len += 2;
				break;
			case '\t':
				b += sprintf(b, "\\t");
				send_len += 2;
				break;
			case '\b':
				b += sprintf(b, "\\b");
				send_len += 2;
				break;
			default:
				//不可见字符
				if(J_iscntrl(c))
				{
					break;
				}
				else
				{
					*b++ = c;
					send_len++;
					break;
				}
		}
		if(send_len >= (sock_t->send_len+1024-1))
		{
			sock_t->send_len = send_len;
			sock_t->send_buf = wys_realloc(sock_t->send_buf, sock_t->send_len+1024);
			b = sock_t->send_buf + send_len;
			if(!b)
			{
				return -1;
			}
		}
	}
	*b = 0;

	sock_t->send_len = send_len;
	return 0;
}

//----------------------------------------------------------------
// 函数名称 an_get_json_data_safe
/// @brief 获取具体日期的GMT时间
/// 
/// @param _in year->年
/// @param _in mon->月
/// @param _in day->日
/// @param _in hour->时
/// @param _in min->分
/// @param _in sec->秒
/// @return 具体日期的GMT时间
/// @note 传入 2016,9,8,11,53,50 即返回 2016-09-08 11:53:50的GMT时间
/// @author miaomiao
//---------------------------------------------------------------
_u64 an_time_to_gmt(_u32 year, _u32 mon, _u32 day, _u32 hour, _u32 min, _u32 sec)
{
	struct tm time = {0};
	time.tm_sec = sec;
	time.tm_min = min;
	time.tm_hour = hour;
	time.tm_mday = day;
	time.tm_mon = mon - 1;
	time.tm_year = year - 1900;

	return mktime(&time);
}

//----------------------------------------------------------------
// 函数名称 an_get_json_data_safe
/// @brief 根据GMT时间获取具体日期
///
/// @param _in gmt->GMT时间
/// @param _out year->年
/// @param _out mon->月
/// @param _out day->日
/// @param _out hour->时
/// @param _out min->分
/// @param _out sec->秒
/// @return void->无返回值
/// @author miaomiao
//---------------------------------------------------------------
void an_gmt_to_time(time_t gmt, _u32* year, _u32* mon, _u32* day, _u32* hour, _u32* min, _u32* sec)
{
	struct tm *timep = NULL;
	timep = localtime(&gmt);

	*year = timep->tm_year + 1900;
	*mon = timep->tm_mon + 1;
	*day = timep->tm_mday;
	*hour = timep->tm_hour;
	*min = timep->tm_min;
	*sec = timep->tm_sec;

	return;
}

//----------------------------------------------------------------
// 函数名称 an_get_json_data_safe
/// @brief 根据GMT时间得到该时间点所在年份的哪一天,区间[1-366]
///
/// @param _in gmt->GMT时间
/// @return 该年的第几天,返回值区间[1-366]
/// @author miaomiao
//---------------------------------------------------------------
_u32 an_gmt_to_yday(time_t gmt)
{
	struct tm *timep = NULL;
	timep = localtime(&gmt);
	return timep->tm_yday + 1;
}

//----------------------------------------------------------------
// 函数名称 an_gmt_format_to_hour_inver
/// @brief 获取逆序GMT当前小时0分0秒时的GMT时间
///
/// @param _in gmt->逆序GMT时间
/// @return GMT时间
/// @note 传入2016.09.13 08:33:45的逆序GMT时间 返回2016.09.13 08:00:00的逆序GMT时间
/// @author miaomiao
//---------------------------------------------------------------
_u64 an_gmt_format_to_hour_inver(time_t gmt)
{
	_u32 year,mon,day,hour,min,sec;
	_u64 ret_time;
	
	gmt = GMT_REVERS_MASK - gmt;
	an_gmt_to_time(gmt, &year, &mon, &day, &hour, &min, &sec);
	gmt -= (min*60 + sec);
	ret_time = GMT_REVERS_MASK - gmt;

	return ret_time;
}

//----------------------------------------------------------------
// 函数名称 an_gmt_format_to_hour_inver
/// @brief 获取逆序GMT当天0时0分0秒时的GMT时间
///
/// @param _in gmt->逆序GMT时间
/// @return GMT时间
/// @note 传入2016.09.13 08:33:45的逆序GMT时间 返回2016.09.13 00:00:00的逆序GMT时间
/// @author miaomiao
//---------------------------------------------------------------
_u64 an_gmt_format_to_day_inver(time_t gmt)
{
	_u32 year,mon,day,hour,min,sec;
	_u64 ret_time;
	
	gmt = GMT_REVERS_MASK - gmt;
	an_gmt_to_time(gmt, &year, &mon, &day, &hour, &min, &sec);
	gmt -= (hour*3600 + min*60 + sec);
	ret_time = GMT_REVERS_MASK - gmt;

	return ret_time;
}

//----------------------------------------------------------------
// 函数名称 simple_strtoul_ull
/// @brief 字符串转unsigned long long int
///
/// @param _in cp->需要转换的原始数据
/// @param _out endp->接收转换后原始数据的最后一位非数字地址
/// @param _in base->原始数据为8/10/16进制数据
/// @return 转换后的数据
/// @author miaomiao
//---------------------------------------------------------------
_u64 simple_strtoul_ull(const _s8 *cp,_s8 **endp,_u32 base)
{
	_u64 result = 0,value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x' || *cp == 'X') && J_isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (J_isxdigit(*cp) && (value = J_isdigit(*cp) ? *cp-'0' : (J_islower(*cp)
	    ? J_toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (_s8 *)cp;
	return result;
}

//----------------------------------------------------------------
// 函数名称 atoull
/// @brief 字符串转unsigned long long int
///
/// @param _in buf->需要转换的原始数据
/// @return 字符串为空返回0，否则返回转换后的数据
/// @author miaomiao
//---------------------------------------------------------------
_u64 atoull(_s8 * buf)
{
	if(buf == NULL || buf[0] == 0)
		return 0;
	else
		return simple_strtoul_ull(buf, NULL, 0);
}

//----------------------------------------------------------------
// 函数名称 ntohll
/// @brief 64位网络序转本地序
/// 
/// @param _in net_var->网络序unsigned long long int数据
/// @return 本地序unsigned long long int数据
/// @author 陈瑾寒
//---------------------------------------------------------------
_u64 ntohll(_u64 net_var)
{

#if __BYTE_ORDER == __BIG_ENDIAN
	return net_var;
#else

	union
	{
		_u32 tmp[2];
		_u64 h_var;
	}host_var;

	host_var.h_var = net_var;
	
	return (((_u64)ntohl(host_var.tmp[0]) << 32) | (_u64)ntohl(host_var.tmp[1]));
#endif

}

//----------------------------------------------------------------
// 函数名称 htonll
/// @brief 64位网络序转本地序
/// 
/// @param _in host_var->本地序unsigned long long int数据
/// @return 网络序unsigned long long int数据
/// @author 陈瑾寒
//---------------------------------------------------------------
_u64 htonll(_u64 host_var)
{

#if __BYTE_ORDER == __BIG_ENDIAN
	return host_var;
#else	
	union
	{
		_u32 tmp[2];
		_u64 n_var;
	}net_var;

	net_var.tmp[0] = htonl(host_var >> 32);
	net_var.tmp[1] = htonl(host_var & 0xFFFFFFFFULL);

	return net_var.n_var;
#endif
}

