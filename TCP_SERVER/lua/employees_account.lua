

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
		
		if opt == "getall"  then
			lmsg.cmd = fun.cmd.AZ_USER_GETALL
		end
		
		if opt == "del" then
			lmsg.cmd = fun.cmd.AZ_USER_DEL
		end
		
		if opt == "mod" then
			lmsg.cmd = fun.cmd.AZ_USER_MOD
		end 
		
		if opt == "add" then
			lmsg.cmd = fun.cmd.AZ_USER_ADD
		end
		
		if opt == "get" then
			lmsg.cmd = fun.cmd.AZ_USER_GET
		end
		
		lmsg.account = args.account
		lmsg.password = args.pwd
		lmsg.bind = args.bindComputer
		lmsg.uid = args.user_id
		lmsg.old_gid = args.old_gid or args.old_group_id
		lmsg.new_gid = args.new_gid or args.group_id
		lmsg.remark = args.name
		
		local ok,_data = fun.toserver(lmsg)
		
		if not ok then
			print ("opt failure")
		end
		data = _data
		
	until true
	x.set["Content-type"] = conf.charset
	return data
end