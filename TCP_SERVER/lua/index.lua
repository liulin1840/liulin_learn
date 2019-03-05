

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
		
		if opt == "getindex"  then
			lmsg.cmd = fun.cmd.AZ_CALC_GETINDEX
		end
		
		local ok,_data = fun.directserver(lmsg)
		
		if ok then
			print("opt success ######## ")
		end
		data = _data
until true
	x.set["Content-type"] = conf.charset
	return data
end