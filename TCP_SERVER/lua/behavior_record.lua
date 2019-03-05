


--行为记录页面cgi

module(...,package.seeall)
local fun = require "fun"
local cjson = require "cjson"

function main(x)
	x.req.read_body()
	local args = x.req.get_post_args()
	local opt = args.opt;
	local data = "{\"0\":\"opt error\"}"
	
	
	
	
repeat
		if not args or not args.opt then break end
		
		local lmsg = {}
		
		if opt == "getall"  then
			lmsg.cmd = fun.cmd.AZ_DETAIL_GETALL
		end
		
		if opt == "getgroup" then 
			lmsg.cmd = fun.cmd.AZ_DETAIL_GETGROUP
		end
		
		if opt == "export" then
			lmsg.cmd = fun.cmd.AZ_DETAIL_EXPORTALL
		end
		
		lmsg.uid = args.uid
		lmsg.gid = args.gid
		lmsg.act_type = args.act_type
		lmsg.start_time = args.start_time
		lmsg.end_time = args.end_time
		lmsg.page_flag = args.page_flag
		
		local ok,_data = fun.toserver(lmsg)
		
		if ok then
			print("opt success ######## ")
		end
		data = _data
until true
	x.set["Content-type"] = conf.charset
	return data
end