/*****************************************************
@Copyright (c) 2016 GUN. All rights reserved
@Auther    : miaomiao
@Department: GUN
@E-mail	   : 545911533@qq.com
******************************************************/
#include <x-lua.h>
#include <x-debug.h>
#include <x-memory.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdlib.h>

_vd lua_stack_dump(lua_State *L)
{
	_s32 i;
	_s32 top = lua_gettop(L);

	INFO("========== Lua C Stack Dump ===========\n");
	INFO("Elemnet:%d\n",top);
	
	for(i = 1; i <= top; ++i)
	{
		_s32 t = lua_type(L,i);

		switch(t)
		{
			case LUA_TSTRING:
			{
				INFO("---------------------------------------\n");
				INFO("Index:%d\nType:%s\nData:%-20.20s\n",i,lua_typename(L,t),lua_tostring(L,i));
				break;
			}
			case LUA_TBOOLEAN:
			{
				INFO("---------------------------------------\n");
				INFO("Index:%d\nType:%s\nData:%s\n",i,lua_typename(L,t),lua_toboolean(L,i) ? "true" : "false");
				break;
			}
			case LUA_TNUMBER:
			{
				INFO("---------------------------------------\n");
				INFO("Index:%d\nType:%s\nData:%d\n",i,lua_typename(L,t),(_s32)lua_tonumber(L,i));
				break;
			}
			default:
			{
				INFO("---------------------------------------\n");
				INFO("Index:%d\nType:%s\nData:Unknow\n",i,lua_typename(L,t));
				break;
			}
		}
	}

	INFO("=======================================\n");
}

_s32 lua_access(x_session_t* session)
{

	
	lua_State *L = lua_open();
	_s8* data = NULL;
	_s32 data_len = 0;
	_s32 keep_alive = 0;
	_s32 ret = -1;
	
	luaL_openlibs(L);

	if(luaL_dofile(L, "./lua/core/access.lua"))
	{
		snprintf(session->error_info, 127, "Load lua script \"access.lua\" failed!\n");
		session->state |= SESSION_STAT_LUA_ERROR;
		goto OUT;
	}

	lua_getglobal(L,"access");
	INFO("push length:%d\n",session->rcv_len);
	lua_pushlstring(L, session->rcv_buf, session->rcv_len);

	wys_free(session->rcv_buf);
	session->rcv_buf = NULL;
	session->rcv_buflen = 0;
	session->rcv_len = 0;
	
	if(lua_pcall(L,1,3,0) != 0)
	{
		snprintf(session->error_info, 127, "%s",lua_tostring(L,1));
		INFO("%s\n",lua_tostring(L,1));
		session->state |= SESSION_STAT_LUA_ERROR;
		goto OUT;
	}

	data = (_s8*)lua_tostring(L,1);	
	data_len = (_s32)lua_tonumber(L,2);
	keep_alive = (_s32)lua_tonumber(L,3);

	if(keep_alive)
	{
		session->state |= SESSION_STAT_KEEP_ALIVE;
	}

	session->snd_buf = (_s8*)wys_realloc(session->snd_buf, data_len);

	if(!session->snd_buf)
	{
		INFO("Out Of Memory!\n");
		goto ERR;
	}

	memcpy(session->snd_buf + session->snd_len, data, data_len);
	session->snd_len += data_len;
OUT:
	ret = 0;
ERR:
	lua_settop(L,0);
	return ret;
}
