module(..., package.seeall);
local conf = require("conf")
function findCgi(fileName)
	local ok = false
	local cgi
	
	repeat
		if not conf.locationByLuaScript[fileName] then
			break 
		end
		
		cgi = conf.locationByLuaScript[fileName]
		ok = not ok
	until true
	
	return ok, cgi
end