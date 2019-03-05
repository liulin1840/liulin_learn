/*****************************************************
@Copyright (c) 2015-2016 GUN. All rights reserved
@Auther    : miaomiao
@Department: GUN
@E-mail	   : 545911533@qq.com
******************************************************/
#ifndef _MEM_H
#define _MEM_H "mem.h"
#include <stdlib.h>
#include <malloc.h>

#include "types.h"
#if 1
#define wys_free(ptr) _free(ptr,__FUNCTION__,__LINE__)
#define wys_malloc(size) _malloc(size,__FUNCTION__,__LINE__)
#define wys_realloc(ptr,size) _realloc(ptr,size,__FUNCTION__,__LINE__)
#else
#define wys_free(ptr) free(ptr)
#define wys_malloc(size) malloc(size)
#define wys_realloc(ptr,size) realloc(ptr,size)
#endif
void wys_mem_init(void);
void wys_show_mem_info(void);
void wys_hex_dump(_u8 *buf,_s32 len);
void* _malloc(_u32 size, const _s8* fname, _u32 line);
void* _realloc(void* ptr,_u32 size, const _s8* fname, _u32 line);
void _free(void* ptr, const _s8* fname, _u32 line);
#endif

