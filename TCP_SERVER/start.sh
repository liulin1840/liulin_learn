#!/bin/sh

killall x-server
CurPath=`pwd`
LuaPath=$CurPath'/lua/share/?.lua;'
LuaPath+=$CurPath'/lua/?.lua;'
LuaPath+=$CurPath'/lua/core/?.lua;'
LuaCPath=$CurPath'/lua/lualib/?.so;'

export LUA_PATH=$LuaPath
echo $LuaPath
export LUA_CPATH=$LuaCPath
echo $LuaCPath

./x-server
