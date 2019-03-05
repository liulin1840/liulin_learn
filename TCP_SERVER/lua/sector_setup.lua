

module(...,package.seeall)
local fun = require "fun"
local cjson = require "cjson"

function main(x)
	x.req.read_body()
	local args = x.req.get_post_args()
	local opt = args.opt;
	
	
	
	local data = '{"0":"opt error"}'

	repeat
		if not args or not args.opt then break end
		
		local lmsg = {}
		
		if opt == "add" then 			--添加账号
			
			lmsg["cmd"] = fun.cmd.AZ_GROUP_ADD
		end
		
		if opt == "getall" then		--获取所有信息
			lmsg["cmd"] = fun.cmd.AZ_GROUP_GETALL
		end
		
		if opt == "del" then
			lmsg["cmd"] = fun.cmd.AZ_GROUP_DEL
		end
		
		if opt == "mod" then
			print("mod")
			lmsg["cmd"] = fun.cmd.AZ_GROUP_MOD
		end
		
		if opt == "get" then
			lmsg["cmd"] = fun.cmd.AZ_GROUP_GET
		end
		
		lmsg["group_name"] = args.name
		lmsg["gid"] = args.group_id
		lmsg["record"] = args.record
		lmsg["control"] = args.control
		lmsg["usb_auth"] = args.usbUse
		lmsg["cd_auth"] = args.CDUse
		     
		local ok,rdata = fun.toserver(lmsg)
	
		if not ok then
			print ("opt failure")
		end
		
		data = rdata
	until true
	
	x.set["Content-type"] = conf.charset
	return data
end