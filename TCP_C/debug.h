/*****************************************************
@Copyright (c) 2015-2016 WAYOS.Co.Ltd. All rights reserved
@Auther    : miaomiao
@Department: R&D
@E-mail	   : chenjinhan@wayos.cn
******************************************************/
#ifndef _DEBUG_H
#define _DEBUG_H "debug.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define WYS_DEBUG_USE 1
#if WYS_DEBUG_USE
#define DEBUG(format, args...)do{printf("[%s|%d]"format,__FUNCTION__, __LINE__,##args);}while(0)		
#define ERROR(args)	do{printf("["args" error!|%d] errno=%d, errstr=%s\n", __LINE__, errno, strerror(errno));}while(0)
#define INFO(format, args...) do{printf(format,##args);}while(0)
#define POINT(interval) do{printf("(%s|%d) break point\n",__FUNCTION__, __LINE__);sleep(interval);}while(0)
#define CHECK(n) do{printf("%s|%d|-----check----->0x0%d\n",__FUNCTION__, __LINE__,n);}while(0)
#else
#define DEBUG(format, args...)
#define ERROR(args)
#define INFO(format, args...)
#define POINT(interval)
#define CHECK(n)
#endif

#endif

