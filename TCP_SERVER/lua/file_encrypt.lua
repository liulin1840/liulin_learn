

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
		
		lmsg.gid = args.new_gid
		lmsg.filetype = args.types
		lmsg.old_gid = args.old_gid
		lmsg.new_gid = args.new_gid
		
		if opt == "add" then
			lmsg.cmd = fun.cmd.AZ_FILE_ADD
		end
		
		if opt == "getall" then
			lmsg.cmd = fun.cmd.AZ_FILE_GETALL
		end
		
		if opt == "del"  then
			lmsg.cmd = fun.cmd.AZ_FILE_DEL
			lmsg.old_gid = args.old_gid
			lmsg.new_gid = args.new_gid
			lmsg.old_filename = args.encryptExp
			lmsg.new_filename = args.encryptExp
		end
		
		if opt == "mod" then
			lmsg.cmd = fun.cmd.AZ_FILE_MOD
			--lmsg.new_filename = args.catalog or args.suffix or args.course
			--lmsg.old_filename = args.old_catalog or args.old_suffix or args.old_course
		end
		
		if opt ~= "del" then
			lmsg.new_filename = args.catalog or (args.suffix or args.course )
			lmsg.old_filename = args.old_catalog or (args.old_suffix or args.old_course) 
		end
			
		
		local ok,_data = fun.toserver(lmsg)
		
		if not ok then break end
		
		data = _data

	until true
	x.set["Content-type"] = conf.charset
	return data
end