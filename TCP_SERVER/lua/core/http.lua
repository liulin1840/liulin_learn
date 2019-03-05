--[[
Copyright (c) 2015-2016 GUN. All rights reserved
Auther     : miaomiao
Department : GUN
E-mail     : 545911533@qq.com
--]]
module(..., package.seeall);

function combine(x,content)
	local http = {}
	-- TODO:后续再开发keep-alive
	x.set["Connection"] = "close"
	
	if x.redirectControl.flag then
		http[#http + 1] = "HTTP/1.1 302 Moved Permanently\r\n"				
		http[#http + 1] = "Location: " .. x.redirectControl.location .. "\r\n"
		for k,v in pairs(x.set) do
			if v then
				http[#http + 1] = k .. ": " .. v .. "\r\n"
			end
		end
		http[#http + 1] = "\r\n"
	else	
		http[#http + 1] = "HTTP/1.1 200 OK\r\n"
						
		for k,v in pairs(x.set) do
			if v then
				http[#http + 1] = k .. ": " .. v .. "\r\n"
			end
		end
		http[#http + 1] = "\r\n"
		http[#http + 1] = content
	end
	
	http = table.concat(http);
		
	return true, http, (x.set["Connection"] == "close" and {0} or {1})[1]	
end