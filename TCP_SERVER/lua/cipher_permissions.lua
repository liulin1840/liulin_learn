

module(...,package.seeall)
local fun = require "fun"
local cjson = require "cjson"

-- 密文权限对应的cgi 只有一个操作mod


function main(x)
	x.req.read_body()
	local args = x.req.get_post_args()
	local opt = args.opt;
	local data = '{"0":"opt error"}'
	
	
	
	repeat
		if not args or not args.opt then break end
		
		local lmsg = {}
		
		if opt == "getall" then
			lmsg.cmd = fun.cmd.AZ_FILE_GETAUTH
		end
		if opt == "mod"  then
			lmsg.cmd = fun.cmd.AZ_FILE_MODAUTH
		
			local glen = 0
			
			for key,v in pairs(args) do
				print("key:" .. key .. "val: " .. v)
				if key ~= "opt" then
					lmsg[key] = v
					glen = glen + 1
				end
			end
			print("glen: " .. glen)
			lmsg.group_num = tostring(glen)
		end
		
		local ok,_data = fun.toserver(lmsg)
		
		if not ok then
			--res.msg = "数据获取失败"
			print("opt ######## " .. lmsg.cmd .. " ####### failure")
			--break
		end
		data = _data
	until true
	x.set["Content-type"] = conf.charset
	return data
end