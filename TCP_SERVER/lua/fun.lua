

module(...,package.seeall)

local conf = require("conf")
local socket = require ("socket.core")
local cjson = require("cjson")

cmd = {
	["AZ_UNKNOW"] = "0"			--//未知请求

	--//用户信息相关
	,["AZ_USER_ADD"] = "100"		--//添加用户
	,["AZ_USER_DEL"] = "101"			--//删除用户
	,["AZ_USER_MOD"] = "102"		--//修改用户
	,["AZ_USER_GET"] = "103"		--//查询单个用户信息
	,["AZ_USER_GETALL"]	= "104"		--//查询所有用户信息
	
	
	--//组信息相关
	,["AZ_GROUP_ADD"] = "200"		--//添加用户组
	,["AZ_GROUP_DEL"] = "201"			--//删除用户组
	,["AZ_GROUP_MOD"] = "202"		--//修改用户组
	,["AZ_GROUP_GET"] = "203"			--//查询单个用户组信息
	,["AZ_GROUP_GETALL"] = "204"		--//查询所有用户组信息
	,["AZ_GROUP_GETBASE"] = "205"
	
	--//文件加密信息相关
	,["AZ_FILE_ADD"] = "300"		--//添加文件加密规则
	,["AZ_FILE_DEL"] = "301"			--//删除文件加密规则
	,["AZ_FILE_MOD"] = "302"			--//修改文件加密规则
	,["AZ_FILE_GET"] = "303"			--//获取某用户组文件加密规则
	,["AZ_FILE_GETALL"]	= "304"		--//获取全部用户组文件加密规则
	,["AZ_FILE_GETAUTH"] = "305"
	,["AZ_FILE_MODAUTH"] = "306"
	
	--//CD & USB访问控制相关
	,["AZ_QQ_MSG_GETUSER"] = "400"		--//
	,["AZ_QQ_MSG_GETGROUP"] = "401"			--//
	,["AZ_QQ_MSG_GETQQ"] = "402"	
	
	--//用户动态数据(行为统计)相关
	,["AZ_DYNAMIC_DELALL"] = "500"--//删除某用户所有动态数据
	,["AZ_DYNAMIC_GET"]	= "501"		--//获取某用户部分动态数据
	,["AZ_DYNAMIC_GETALL"] = "502"		--//获取某用户所有动态
	
	--//行为记录
	,["AZ_DETAIL_GETUSER"] = "503"		--获取(查询)某个用户的行为记录
	,["AZ_DETAIL_GETGROUP"] = "504"		--获取(查询)某个用户组的行为记录
	,["AZ_DETAIL_GETALL"] = "505"		--获取(查询)全部用户的行为记录
	,["AZ_DETAIL_EXPORTUSER"] = "506"   --导出当前查询的用户的违规记录
	,["AZ_DETAIL_EXPORTGROUP"] = "507"  --导出当前查询的用户组的违规记录
	,["AZ_DETAIL_EXPORTALL"] = "508"	--导出当前查询的全部用户的违规记录
	--//行为统计
	,["AZ_CALC_GETUSER"] = "509"		--获取(查询)某个用户的行为统计
	,["AZ_CALC_GETGROUP"] = "510"		--获取(查询)某个用户组的行为统计
	,["AZ_CALC_GETALL"] = "511"			--获取(查询)全部用户的行为统计
	,["AZ_CALC_GETINDEX"] = "512"		--获取(查询)全部用户的行为统计
	,["AZ_CALC_GETUSER_JS"] = "513"		--获取某个用户的所有行为统计js
	,["AZ_CALC_GETGROUP_JS"] = "514"	--获取某个用户组的所有用户行为统计js
	,["AZ_CALC_GETALL_JS"] = "515"		--获取全部用户组的行为统计js
	,["AZ_CALC_GETMONTH_JS"] = "516"	--获取首页所需的js
	,["AZ_GET_USER_PROCESS"] = "517"
	
	--//办公行为相关
	,["AZ_WORK_ADD"] = "600"		--//增加办公行为规则
	,["AZ_WORK_DEL"] = "601"			--//删除办公行为规则
	,["AZ_WORK_GET"] = "602"			--//获取某用户组办公行为规则
	,["AZ_WORK_GETALL"]	= "603"		--//获取所有办公行为规则

	--//违规行为相关
	,["AZ_FORBID_ADD"] = "700"	--//增加违规行为规则
	,["AZ_FORBID_MOD"] = "701"			--//修改违规行为规则
	,["AZ_FORBID_DEL"] = "702"			--//删除违规行为规则
	,["AZ_FORBID_GET"] = "703"			--//获取某用户组违规行为规则
	,["AZ_FORBID_GETALL"] = "704"		--//获取所有违规行为规则

	--//违规警告相关
	,["AZ_WARNING_MOD"] = "800"	--//修改违规警告

	--//系统设置相关
	,["AZ_SYSTEM_MOD"] = "900"	--//系统修改设置相关
	,["AZ_SYSTEM_GET_ALL"] = "901"
}


serhost = "192.168.2.125"
serport = 7676
timeout = 3

function toserver(msg)

	local host = serhost
	local port = serport
	local msg = "\n" .. cjson.encode(msg)
	local ok = false
	local data = '{"ret":"0","msg":"server data error"}'

	
	print("send msg:" .. msg)
	local sock = socket.tcp()
	
	sock:settimeout(timeout)
	sock:connect(host,port)
	
	
	sock:settimeout(timeout)
	sock:send(msg)
	print("send msg success")
	
	local rcvmsg,status = sock:receive("*a")
	
repeat
	if rcvmsg == nil then
		print("no msg recv")
		break
	end
	
	if rcvmsg ~= nil then
		print("rcvmsg success: " .. rcvmsg .. "\n ")
	end
	
	

	local okk,ddata = pcall(cjson.decode, rcvmsg)
	
	if okk then 
		data = rcvmsg
		ok = true
	end
until true

	sock:close()
	return ok,data
	
end


function directserver(msg)

	local host = conf.serhost
	local port = conf.serport
	local msg = "\n" .. cjson.encode(msg)
	local ok = false
	local data = '{"ret":"0","msg":"server data error"}'

	
	print("send msg:" .. msg)
	local sock = socket.tcp()
	
	sock:settimeout(2)
	sock:connect(host,port)
	
	
	sock:settimeout(timeout)
	sock:send(msg)
	print("send msg success")
	
	local rcvmsg,status = sock:receive("*a")
	
repeat
	if rcvmsg == nil then
		print("no msg recv")
		break
	end
	
	if rcvmsg ~= nil then
		print("rcvmsg success: " .. rcvmsg .. "\n ")
	end
	
	
	data = rcvmsg
	ok = true
	
until true

	sock:close()
	return ok,data
	
end

