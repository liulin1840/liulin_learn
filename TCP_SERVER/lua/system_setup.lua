

module(...,package.seeall)
local fun = require "fun"
local cjson = require "cjson"

function main(x)
	x.req.read_body()
	local args = x.req.get_post_args()
	local data = '{"0":"opt error"}'
	
	
	
	repeat
		if not args or not args.opt then break end
		
		local lmsg = {}
		
		
		if args.opt == "mod" then
			lmsg.cmd = fun.cmd.AZ_SYSTEM_MOD
		end
		
		if args.opt == "getall" then
			lmsg.cmd = fun.cmd.AZ_SYSTEM_GET_ALL
		end
		
		lmsg.jumpurl = args.getout
		lmsg.password = args.password
		lmsg.work_day = args.workdate
		lmsg.start_time = args.start_time
		lmsg.end_time = args.end_time
		lmsg.login_bind = args.encrypt
		lmsg.exception_account = args.exception_account
		lmsg.breaktime = args.worktime
	
		local ok,_data = fun.toserver(lmsg)
		
		if not ok then
			print("opt ######## " .. lmsg.cmd .. " ####### failure")
			--break
		end
		data = _data
	until true
	x.set["Content-type"] = conf.charset
	return data
end