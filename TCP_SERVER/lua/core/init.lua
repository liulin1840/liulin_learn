--[[
Copyright (c) 2015-2016 GUN. All rights reserved
Auther     : miaomiao
Department : GUN
E-mail     : 545911533@qq.com
--]]
module(..., package.seeall);
local pub = require("public")

----------------------------------------------------------------------

local function socket()

end

local function base64()

end

local function md5()

end

---------------------------------------------------------------------
-----------------------------interface-------------------------------
---------------------------------------------------------------------

function initModule(x)
	local ok = false
	
	repeat
		x.set = 
		{
			["Server"] 			= "X-server/1.0 by miaomiao",
			["Connection"] 		= x.header.connection and x.header.connection() or "close" ,
			["Accept-Ranges"] 	= "bytes",
			["Date"] 			= pub.httpFormatDate()		
		}
		x.redirectControl = {}
		x.status 	= 200
		x.redirect 	= redirect
		x.socket 	= socket
		x.base64 	= base64
		x.md5		= md5
		--x.cjson		= require("cjson")
		ok = not ok
	until true
	
	return ok
end

function redirect(self, uri)
	self.redirectControl.flag = true;
	self.redirectControl.location = uri;
	self.status = 302
end
