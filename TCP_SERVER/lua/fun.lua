

module(...,package.seeall)

local conf = require("conf")
local socket = require ("socket.core")
local cjson = require("cjson")

cmd = {
	["AZ_UNKNOW"] = "0"			--//δ֪����

	--//�û���Ϣ���
	,["AZ_USER_ADD"] = "100"		--//����û�
	,["AZ_USER_DEL"] = "101"			--//ɾ���û�
	,["AZ_USER_MOD"] = "102"		--//�޸��û�
	,["AZ_USER_GET"] = "103"		--//��ѯ�����û���Ϣ
	,["AZ_USER_GETALL"]	= "104"		--//��ѯ�����û���Ϣ
	
	
	--//����Ϣ���
	,["AZ_GROUP_ADD"] = "200"		--//����û���
	,["AZ_GROUP_DEL"] = "201"			--//ɾ���û���
	,["AZ_GROUP_MOD"] = "202"		--//�޸��û���
	,["AZ_GROUP_GET"] = "203"			--//��ѯ�����û�����Ϣ
	,["AZ_GROUP_GETALL"] = "204"		--//��ѯ�����û�����Ϣ
	,["AZ_GROUP_GETBASE"] = "205"
	
	--//�ļ�������Ϣ���
	,["AZ_FILE_ADD"] = "300"		--//����ļ����ܹ���
	,["AZ_FILE_DEL"] = "301"			--//ɾ���ļ����ܹ���
	,["AZ_FILE_MOD"] = "302"			--//�޸��ļ����ܹ���
	,["AZ_FILE_GET"] = "303"			--//��ȡĳ�û����ļ����ܹ���
	,["AZ_FILE_GETALL"]	= "304"		--//��ȡȫ���û����ļ����ܹ���
	,["AZ_FILE_GETAUTH"] = "305"
	,["AZ_FILE_MODAUTH"] = "306"
	
	--//CD & USB���ʿ������
	,["AZ_QQ_MSG_GETUSER"] = "400"		--//
	,["AZ_QQ_MSG_GETGROUP"] = "401"			--//
	,["AZ_QQ_MSG_GETQQ"] = "402"	
	
	--//�û���̬����(��Ϊͳ��)���
	,["AZ_DYNAMIC_DELALL"] = "500"--//ɾ��ĳ�û����ж�̬����
	,["AZ_DYNAMIC_GET"]	= "501"		--//��ȡĳ�û����ֶ�̬����
	,["AZ_DYNAMIC_GETALL"] = "502"		--//��ȡĳ�û����ж�̬
	
	--//��Ϊ��¼
	,["AZ_DETAIL_GETUSER"] = "503"		--��ȡ(��ѯ)ĳ���û�����Ϊ��¼
	,["AZ_DETAIL_GETGROUP"] = "504"		--��ȡ(��ѯ)ĳ���û������Ϊ��¼
	,["AZ_DETAIL_GETALL"] = "505"		--��ȡ(��ѯ)ȫ���û�����Ϊ��¼
	,["AZ_DETAIL_EXPORTUSER"] = "506"   --������ǰ��ѯ���û���Υ���¼
	,["AZ_DETAIL_EXPORTGROUP"] = "507"  --������ǰ��ѯ���û����Υ���¼
	,["AZ_DETAIL_EXPORTALL"] = "508"	--������ǰ��ѯ��ȫ���û���Υ���¼
	--//��Ϊͳ��
	,["AZ_CALC_GETUSER"] = "509"		--��ȡ(��ѯ)ĳ���û�����Ϊͳ��
	,["AZ_CALC_GETGROUP"] = "510"		--��ȡ(��ѯ)ĳ���û������Ϊͳ��
	,["AZ_CALC_GETALL"] = "511"			--��ȡ(��ѯ)ȫ���û�����Ϊͳ��
	,["AZ_CALC_GETINDEX"] = "512"		--��ȡ(��ѯ)ȫ���û�����Ϊͳ��
	,["AZ_CALC_GETUSER_JS"] = "513"		--��ȡĳ���û���������Ϊͳ��js
	,["AZ_CALC_GETGROUP_JS"] = "514"	--��ȡĳ���û���������û���Ϊͳ��js
	,["AZ_CALC_GETALL_JS"] = "515"		--��ȡȫ���û������Ϊͳ��js
	,["AZ_CALC_GETMONTH_JS"] = "516"	--��ȡ��ҳ�����js
	,["AZ_GET_USER_PROCESS"] = "517"
	
	--//�칫��Ϊ���
	,["AZ_WORK_ADD"] = "600"		--//���Ӱ칫��Ϊ����
	,["AZ_WORK_DEL"] = "601"			--//ɾ���칫��Ϊ����
	,["AZ_WORK_GET"] = "602"			--//��ȡĳ�û���칫��Ϊ����
	,["AZ_WORK_GETALL"]	= "603"		--//��ȡ���а칫��Ϊ����

	--//Υ����Ϊ���
	,["AZ_FORBID_ADD"] = "700"	--//����Υ����Ϊ����
	,["AZ_FORBID_MOD"] = "701"			--//�޸�Υ����Ϊ����
	,["AZ_FORBID_DEL"] = "702"			--//ɾ��Υ����Ϊ����
	,["AZ_FORBID_GET"] = "703"			--//��ȡĳ�û���Υ����Ϊ����
	,["AZ_FORBID_GETALL"] = "704"		--//��ȡ����Υ����Ϊ����

	--//Υ�澯�����
	,["AZ_WARNING_MOD"] = "800"	--//�޸�Υ�澯��

	--//ϵͳ�������
	,["AZ_SYSTEM_MOD"] = "900"	--//ϵͳ�޸��������
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

