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
	["/department.data"] = "department.lua",				-- ��
	
	
	["/sector_setup.data"] = "sector_setup.lua", 			--Ա���˺� ��
	
	["/employees_account.data"] = "employees_account.lua", --�������� ��
	
	["/cipher_permissions.data"] = "cipher_permissions.lua",	--����Ȩ��
	
	["/office_behavior.data"] = "office_behavior.lua",			--�칫��Ϊɸѡ pageindex:1
	["/office_behavior_add.data"] = "office_behavior.lua",		--�칫��Ϊ���
	["/modifyEncrypt.data"] = "modifyEncrypt.lua",				--�칫��Ϊ����Ա��
	["/encryptObj.data"] = "encryptObj.lua",
	
	["/irregularities.data"] = "irregularities.lua",			--Υ����Ϊ opt:getall pageindex:1
	["/irregularities.data"] = "irregularities.lua", 			--Υ����Ϊ�޸� opt:mod irrText: type_up staff  behavior
	
	["/system_setup.data"] = "system_setup.lua",				--ϵͳ����
	
	["/chat_record.data"] = "chat_record.lua",					--�����¼
	
	["/file_encrypt.data"] = "file_encrypt.lua",				--�ļ�����
	
	["/behavior_statistical.data"] = "behavior_statistical.lua", --��Ϊͳ��
	
	["/behavior_record.data"] = "behavior_record.lua",			--��Ϊ��¼
	["/employee.data"] = "behavior_record.lua",						--��Ϊ��¼getall
	
	["/index.data"] = "index.lua",								--��ҳ
	
	["/proccess.data"] = "process.lua"							--jingcheng
}

