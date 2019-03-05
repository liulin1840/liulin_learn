module(...,package.seeall)
local pub = require("public")

local info = {
	req = {},
	header = {}
}

-------------------------------------------------------------------------------

local function _mergeParms(argsStr, args)
	repeat
		local res, argsTab = pub.split(argsStr, '&')
		if not res then break end
		
		for k, v in pairs(argsTab) do
			repeat
				local sign = v:find('=')
				if not sign then break end
				local parmName = v:sub(1, sign -1)
				local parmValue = v:sub(sign + 1, -1)
				if not args[parmName] then
					args[parmName] = parmValue
				else
					local arrVal = args[parmName]
					args[parmName] = {}
					if type(arrVal) == "table" then
						for i = 1, #arrVal do
							table.insert(args[parmName], arrVal[i])
						end
					else
						table.insert(args[parmName], arrVal)
					end
					table.insert(args[parmName], parmValue)
				end
			until true
		end
	until true
	return args
end

local function _mergeParmsByGet(uri, args)
	if uri:match('?') then
		repeat
			local eof = uri:find('?')
			local argsStr = uri:sub(eof+1, -1)
			uri = uri:sub(1, eof -1)
		
			args = _mergeParms(argsStr, args)
			
		until true
	end
	
	return uri, args
end

local function _read_body_part(content)
	local ok = false
	local file
	repeat
		local start = content:find("\r\n\r\n")
		if not start then break end
		file = content:sub(start + 4, -1)

		ok = not ok
	until true
	return ok, file
end

local function _mergeParmsByPost(content)
	local args = {}
	
	repeat
		local headers = info.header["content_type"]()
		if not headers then
			break
		end
		local boundary = headers:match("%s*boundary=(.-)$")
		if boundary then
			local filename = content:match("%s*filename=\"([^\"]+)\"")
			local ok, file = _read_body_part(content)
			if not ok then break end
			if not file then break end
			local eof = file:find("--" .. boundary .. "--")
			if eof then
				file = file:sub(1, eof - 3)
			end
			args.filename = filename
			args.file = file
		else
			local res, argsTab = pub.split(content, '&')
			if not res then break end
			for k, v in pairs(argsTab) do
				repeat
					local sign = v:find('=')
					if not sign then break end
					local parmName = v:sub(1, sign -1)
					local parmValue = v:sub(sign + 1, -1)
					if not args[parmName] then
						args[parmName] = parmValue
					else
						local arrVal = args[parmName]
						args[parmName] = {}
						if type(arrVal) == "table" then
							for i = 1, #arrVal do
								table.insert(args[parmName], arrVal[i])
							end
						else
							table.insert(args[parmName], arrVal)
						end
						table.insert(args[parmName], parmValue)
					end
				until true
			end
		end
	until true
	return args
end

local function read_body() 
	local content = info.content 		
	local args = _mergeParmsByPost(content)
	if not info.req then info.req = {} end
	
	info.req.get_post_args = function() return args end
end	

local function _get_cookie()
	
end

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------



function phrase(data)
	--数据检查
	local ok = false
	
	--local info = {}
	
	repeat 
		
		if not phraseRequestLine(data) then break end
		if not phraseRequestHeader(data) then break end
		---[[
		if type(info.header.content_length) == 'function' and tonumber(info.header.content_length()) > 0 then
			phraseRequestBody(data)
			info.req.read_body = read_body
		end
		--]]
		ok = not ok
	until true
	
	return ok, info
end



function phraseRequestLine(data)
	local ok = false

	repeat 
		local eof = data:find("\r\n")
		if not eof then break end
		local line = string.sub(data, 1, eof)
		local firstSlash = line:find('/')
		local methed = string.gsub(line:sub(1,firstSlash - 1), ' ', '')		
		local lastSlash = line:find('HTTP')
		local uri = string.gsub(line:sub(firstSlash, lastSlash - 1), ' ', '')
		local args = {}
		if methed == "GET" then
			uri, args = _mergeParmsByGet(uri, args)
		end
		local version = line:sub(lastSlash, -1)

		info.req = 
		{
			get_uri = function() return uri end ,
			get_version = function() return version end ,
			get_methed = function() return methed end ,
			get_uri_args = function() return args end
		}
	
		ok = not ok	
	until true
	
	return ok
end

function phraseRequestHeader(data)
	local ok = false

	repeat
		local start = data:find("\r\n")
		if not start then break end
		local eof = data:find("\r\n\r\n")
		if not eof then break end
		local header = data:sub(start + 2, eof + 3)
	
		local headerStart = header:find("\r\n")
		for i = 1, string.len(header) do
	
			local offset = header:find("\r\n")
			local line = header:sub(1, offset)
			header = header:sub(offset + 2, -1)
						
			local firstColon = line:find(':')
			if not firstColon then break end
			local parmName = line:sub(1, firstColon - 1)
			if parmName:match('-') then
				parmName = parmName:gsub('-', '_')
			end
			parmName = parmName:gsub(' ', ''):lower()
			
			local parmValue = line:sub(firstColon + 1, -1)
			local valStart = parmValue:find("%w") or parmValue:find("%c")
			parmValue = parmValue:sub(2, -1)
		--	parmValue = parmValue:gsub("\r\n", '')
			
		--	if parmValue:match(",") then
		--		parmValue = pub.split(parmValue, ",")
		--	end
			
			if not info.header then info.header = {} end
			info.header[parmName] = function() return parmValue end	
		end
		ok = not ok
	until true
	
	return ok
end

function phraseRequestBody(data)
	local ok = false
	local content
	repeat
		local start = data:find('\r\n\r\n')
		if not start then break end
		content = data:sub(start + 4, -1)

		info.content = content
		ok = not ok
	until true
	
	return ok, content
end


























