/*****************************************************
@Copyright (c) 2016 GUN. All rights reserved
@Auther    : miaomiao
@Department: GUN
@E-mail	   : 545911533@qq.com
******************************************************/
#include <x-types.h>

_s32 poll_process_run(_s32 interval);
_s32 poll_init(_s32 poll_max);
_s32 poll_setfd_noblocking(_s32 fd);
_s32 poll_add(_s32 fd);
_vd poll_del(_s32 index);
_vd poll_add_write(_u32 pid);
_vd poll_add_read(_u32 pid);
_vd poll_del_write(_u32 pid);
_vd poll_del_read(_u32 pid);


