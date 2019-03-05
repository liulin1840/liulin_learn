/////////////////////////////////////////////////////////////////////
/// @file allocator.h
/// @brief 提供解析TCP包的函数
/// 
/// @author miaomiao
/// @version V2.0
/// @date 2016.11.08
/////////////////////////////////////////////////////////////////////
#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H "allocator.h"
#include "types.h"

/// @brief 解析TCP包中的参数,完成解析后交给worker处理
void al_allocate_task(_s8* data, _s32 data_len, _s32 index);
#endif // end of "worker.h"
