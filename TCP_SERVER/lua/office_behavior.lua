

module(...,package.seeall)
local fun = require "fun"
local cjson = require "cjson"

function main(x)
	x.req.read_body()
	local args = x.req.get_post_args()
	local opt = args.opt;
	local data = '{"0":"opt error"}'
	
	
	--包括设置员工 移除 筛选 添加
	repeat
		if not args or not args.opt then break end
		
		local lmsg = {}
		
		if opt == "getall" then 			
			
			lmsg["cmd"] = fun.cmd.AZ_WORK_GETALL
		end
		
		if opt == "getact" then		
			lmsg["cmd"] = fun.cmd.AZ_WORK_GET
		end
		
		if opt == "del" then
			lmsg["cmd"] = fun.cmd.AZ_WORK_DEL
		end
		
		if opt == "add" then
			lmsg["cmd"] = fun.cmd.AZ_WORK_ADD
		end
		
		if opt == "mod" then 
			lmsg["cmd"] = fun.cmd.AZ_WORK_MOD
		end
		
		if opt == "ser" then
			lmsg.cmd = fun.cmd.AZ_WORK_GET
		end
		
		lmsg.gid = args.group_id
		lmsg.type = args.type
		lmsg.describe = args.describe
		lmsg.listcount = args.pageindex
		
		local ok,rdata = fun.toserver(lmsg)
	
		if not ok then
			print ("opt failure")
		end
		
		data = rdata
	until true
	
	x.set["Content-type"] = conf.charset
	return data
end