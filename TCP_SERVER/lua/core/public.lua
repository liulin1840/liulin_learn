--[[
Copyright (c) 2015-2016 GUN. All rights reserved
Auther     : miaomiao
Department : GUN
E-mail     : 545911533@qq.com
--]]

module(..., package.seeall);

function split(str, delimiter)
	
	local ok = false
	local res = {}
	
	repeat
		if str == nil or str == '' or delimiter == nil 
		then
			break
		end
		
		for match in (str..delimiter):gmatch("(.-)"..delimiter) 
		do
			table.insert(res, match)
		end
		
		ok = not ok
	until true
	
	return ok, res	
end

function fileExist(fileName)
	local file = io.open(fileName, "rb")
	if file
	then
		file:close()
		return true
	else
		return false
	end
end

function fileRead(fileName)
	
	local file = io.open(fileName, "rb")	
	if file then
		local data = file:read("*a")
		file:close()		
		return true, data
	else
		return false, nil
	end
end

function httpFormatDate()
	return os.date("%a") .. ", " .. os.date("%d") .. " " .. os.date("%b") .. " " .. os.date("%Y") .. ", " .. os.date("%X") .. " GMT"
end