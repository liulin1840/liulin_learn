--[[
Copyright (c) 2015-2016 GUN. All rights reserved
Auther     : miaomiao
Department : GUN
E-mail     : 545911533@qq.com
--]]
module(..., package.seeall);

local pub = require("public")
local conf = require("conf")
local cgi = require("cgi")
local pages = require("pages")
local types = require("types")

function getDataFromStaticFile(fileName)
	local ok = false
	local data = nil
	
	repeat 
		if fileName == "/" then fileName = conf.index end
		local fullFileName = conf.staticFileRoot .. fileName
		
		if not pub.fileExist(fullFileName) then 
			break
		end
		
		ok, data = pub.fileRead(fullFileName)
		
		if not ok then
			break
		end		

	until true
	
	return ok, data
end

function alloc(x)

	local ok = false
	local data
	local cgiName
	local entry
	local status
	local pass
	
	repeat 
		local fileName = x.req.get_uri()
		
		if fileName == '/' then
			x:redirect(conf.index or '/index.html')
			ok = true 
			break 
		end
		
		if not conf.accessBy then
			pass = true
		else
			local script = conf.accessBy:sub(1, conf.accessBy:find(".lua") -1)
			ok, script = pcall(require, script)
			
			if not ok then
				data = pages.createErrorPage(x, script)
				x.status = 500
				ok = true
				break
			else
				ok, pass = pcall(script.main, x)
				if not ok then
					data = pages.createErrorPage(x, pass)
					x.status = 500
					ok = true
					break
				end
			end
		end
		
		if not pass then
			x.status = 403
			ok = ok and true or not ok
		else
			if x.redirectControl.flag then
				ok = ok and true or not ok
			end
		end
		
				
		ok, cgiName = cgi.findCgi(fileName) 
		
		if ok then
			local script = cgiName:sub(1,cgiName:find(".lua")-1)
			ok, entry = pcall(require, script)
			
			if not ok then
				data = pages.createErrorPage(x, entry)
				x.status = 500
				ok = ok and true or not ok
				break
			else
				ok, data = pcall(entry.main, x)
				if not ok then
					data = pages.createErrorPage(x, data)
					x.status = 500
					ok = ok and true or not ok
					break
				else
					if not x.set["Content-type"] then
						x.set["Content-type"] = getContentType(x.req.get_uri())
					end
					x.set["Content-Length"] = data:len()
				end
			end
		else				
			ok, data = getDataFromStaticFile(fileName)
						
			if not ok then 
				x.status = 404
				ok = ok and true or not ok
			else
				x.status = 200
				if not x.set["Content-type"] then
						x.set["Content-type"] = getContentType(x.req.get_uri())
				end
				x.set["Content-Length"] = data:len()
			end
		end		
	
	until true

	return ok, data 
end

function getContentType(fileName)	
	
	local suffix = ''
	
	if fileName
	then
		suffix = string.match(fileName, "%.(%w+)[^%.]*$")
	else
		suffix = 'html'
	end

	if(suffix)
	then
		if types.contentTypes[suffix] 
		then
			return types.contentTypes[suffix]
		else
			return types.contentTypes["default"]
		end
	else
		return types.contentTypes["default"]
	end
end