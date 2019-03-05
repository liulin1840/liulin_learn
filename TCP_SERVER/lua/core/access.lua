--[[
Copyright (c) 2016 GUN. All rights reserved
Auther     : miaomiao
Department : GUN
E-mail     : 545911533@qq.com
--]]
local alloc = require("alloc")
local phrase = require("phrase")
local init = require("init")
local pages = require("pages")
local prot = require("http")
local pub = require("public")


function serverErrorInfo(x,status)
	local http = {}
	local content = pages.createErrorPage(x, status)
		
	http[#http + 1] = "HTTP/1.1 500 ERROR\r\n"

	for k,v in pairs(x.set) do
		if v then
			http[#http + 1] = k .. ": " .. v .. "\r\n"
		end
	end
	http[#http + 1] = "\r\n"
	http[#http + 1] = content
	
	http = table.concat(http);
	
	return http
end

function simpleInit(x)
	x.set = 
		{
			["Server"] 			= "X-server/1.0 by miaomiao",
			["Connection"] 		= "close",
			["Accept-Ranges"] 	= "bytes",
			["Date"] 			= pub.httpFormatDate()		
		}
end

function access(data)
	
	print("get length:" .. data:len())
	print(data)
	local success = true
	local status, x, content, http, keepAlive
	repeat
		success, status, x = pcall(phrase.phrase,data)
		if not success then 
			print(status)  
			x = {}
			simpleInit(x)
			break 
		end
	
		success , status = pcall(init.initModule, x)
		if not success then print(status) break end
		
		success , status, content = pcall(alloc.alloc, x)
		if not success then print(status) break end
	
		success, status, http, keepAlive = pcall(prot.combine, x, content)
		if not success then print(status) break end
	until true
		
	if not success then 
		http =  serverErrorInfo(x,status)
		keepAlive = 0;
	end
	
	return http, http:len(), keepAlive
end
































