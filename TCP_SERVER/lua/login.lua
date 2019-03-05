module(..., package.seeall)

local io = require "io"

function main(x)

	x.req.read_body()
	local args = x.req.get_post_args()
	local cook = x.header.cookie()
	
repeat
	if cook == nil or args.checkcode == nil then
		break
	end
	
	cook = string.match(cook,'(%w+)')
	print(cook)
	
	print(args.checkcode)
	if cook == args.checkcode then
		return '{"ret":"1", "msg":"ok"}'
	end
until true
	return '{"ret":"0","msg":"error"}'
end