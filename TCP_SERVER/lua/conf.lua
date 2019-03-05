--[[
Copyright (c) 2015-2016 GUN. All rights reserved
Auther     : miaomiao
Department : GUN
E-mail     : 545911533@qq.com
--]]

module(..., package.seeall);

index = "/index.html"

staticFileRoot = "/data/MT7620/source/user/jhl_tools/water_wall_html"
accessBy = "entry.lua"

charset = "text/plain;charset=UTF-8"
locationByLuaScript = 
{
	["/SecurityCode.jpg"] = "code.lua",
	["/login.data"] = "login.lua",
	["/cjson.data"] = "json_test.lua",
	["/body.data"] = "body.lua",
	["/post.data"] = "post.lua",
	["/department.data"] = "department.lua",				-- √
	
	
	["/sector_setup.data"] = "sector_setup.lua", 			--员工账号 √
	
	["/employees_account.data"] = "employees_account.lua", --部门设置 √
	
	["/cipher_permissions.data"] = "cipher_permissions.lua",	--密文权限
	
	["/office_behavior.data"] = "office_behavior.lua",			--办公行为筛选 pageindex:1
	["/office_behavior_add.data"] = "office_behavior.lua",		--办公行为添加
	["/modifyEncrypt.data"] = "modifyEncrypt.lua",				--办公行为设置员工
	["/encryptObj.data"] = "encryptObj.lua",
	
	["/irregularities.data"] = "irregularities.lua",			--违规行为 opt:getall pageindex:1
	["/irregularities.data"] = "irregularities.lua", 			--违规行为修改 opt:mod irrText: type_up staff  behavior
	
	["/system_setup.data"] = "system_setup.lua",				--系统设置
	
	["/chat_record.data"] = "chat_record.lua",					--聊天记录
	
	["/file_encrypt.data"] = "file_encrypt.lua",				--文件加密
	
	["/behavior_statistical.data"] = "behavior_statistical.lua", --行为统计
	
	["/behavior_record.data"] = "behavior_record.lua",			--行为记录
	["/employee.data"] = "behavior_record.lua",						--行为记录getall
	
	["/index.data"] = "index.lua",								--首页
	
	["/proccess.data"] = "process.lua"							--jingcheng
}

