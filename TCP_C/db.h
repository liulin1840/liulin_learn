/*****************************************************
@Copyright (c) 2015-2016 GUN. All rights reserved
@Auther    : miaomiao
@Department: GUN
@E-mail	   : 545911533@qq.com
******************************************************/
#ifndef _DB_H
#define _DB_H "db.h"
#include "types.h"

#define BTOU64(bin) (*(_u64*)bin)
#define BTOS32(bin) (*(_s32*)bin)
#define BTOU32(bin) (*(_u32*)bin)
#define BTOS16(bin) (*(_s16*)bin)
#define BTOU16(bin) (*(_u16*)bin)
#define BTOS8(bin) (*(_s8*)bin)
#define BTOU8(bin) (*(_u8*)bin)

#define DB_KVN_MAX 1024
#define DB_SCAN_MAX 1024
#define DB_SCAN_SUPER_MAX 10240
#define DB_URL_TABLE_MAX 512

enum DB_RNO
{
	DB_NORET = 0,		//未知返回状态
	DB_ERR,				//数据库错误
	DB_OUT_MEM,			//内存不足
	DB_INVALID_PARMS,	//参数错误
	DB_CONNECT_ERR,		//数据库连接错误
	DB_NOT_FOUND,		//key不存在
	DB_SET_OK,			//set成功
	DB_GET_OK,			//get成功
	DB_DEL_OK,			//del成功
	DB_SCAN_OK,			//scan成功
	DB_NO_EXIST,		//key不存在
	DB_EXIST,			//key存在
	DB_MULTI_GET_OK,	//multi_get成功
	DB_MULTI_SET_OK,	//multi_get成功
	DB_MULTI_DEL_OK,	//multi_del成功
	DB_FLUSHDB_OK,		//flushdb成功
	DB_DBSIZE_OK,		//dbsize成功
	DB_RNO_MAX			//无效返回值
};

typedef struct
{
	_s8* key;
	_s32 key_len;
	_s8* val;
	_s32 val_len;
}db_kv;

typedef struct
{
	_s8* key;
	_s32 key_len;
}db_key;

typedef struct
{
	_u32 sts; //内存释放状态	
	_s8* buf; //源数据
}db_src;

typedef struct db_res
{
	_u16	kvn;				//最多支持1024条数据,将来有需求再按需求扩展
	_u16	rno;				//返回的消息号
	db_src	src;				//数据源,外部不得修改,也无需读取
	db_kv*	kvs;				//key-val 数组		
}DB;

DB*	dbset(_s8* key, _s32 key_len, _s8* val, _s32 val_len);
DB*	dbget(_s8* key, _s32 key_len);
DB* dbdel(_s8* key, _s32 key_len);
DB* dbscan(_s8* start,_s32 start_len, _s8* end, _s32 end_len, _s32 max);
DB* dbexists(_s8* key, _s32 key_len);
DB* dbmulti_set(db_kv* kvs, _s32 kvn);
DB* dbmulti_get(db_key* keys, _s32 key_num);
DB* dbmulti_del(db_key* keys, _s32 key_num);
DB* dbflush(void);
DB* dbsize(void);
_s32 dbinit(_s8* addr, _u16 port);
void dbfree(DB* db);
const _s8* dberr(_u16 dbrno);
_s32 dbcheck(void);
#endif //end of #ifndef

