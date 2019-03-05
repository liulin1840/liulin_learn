

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
		
		if opt == "get" or opt == "getall" then
			lmsg.cmd = fun.cmd.AZ_FORBID_GET
		end
		if opt == "add" then
			lmsg.cmd = fun.cmd.AZ_FORBID_ADD
		end
		if opt == "mod" then
			lmsg.cmd = fun.cmd.AZ_FORBID_MOD
		end
		if opt == "del" then
			lmsg.cmd = fun.cmd.AZ_FORBID_DEL
		end
		if opt == "ser" then
			lmsg.cmd = fun.cmd.AZ_FORBID_GET
		end
		if opt == "scr" then
			lmsg.cmd = fun.cmd.AZ_FORBID_GET
		end
		if opt == "modwaring" then
			lmsg.cmd = fun.cmd.AZ_WARNING_MOD
			lmsg.warning_msg = args.warning_msg
			lmsg.warning_en = args.warning_en
			lmsg.error_msg = args.error_msg
			lmsg.error_en = args.error_en
			
		end
		
		if opt ~= "modwaring" then
			lmsg.old_name = (args.old_content or args.old_describle) or args.key
			lmsg.new_name = (args.content or args.describle) or args.key

			lmsg.old_gid = (args.old_group_id or args.group_id) or "0"
			lmsg.new_gid = args.group_id or "0"

			lmsg.act_type = args.type or args.type_up

			lmsg.is_forbid = args.behavior
			
			lmsg.search_flag = args.flag or "0"
		end
		
		local ok,_data = fun.toserver(lmsg)
		
		if not ok then
			print("opt ######## " .. lmsg.cmd .. " ####### failure")
		--	break
		end
		data = _data
	until true
	x.set["Content-type"] = conf.charset
	return data
end