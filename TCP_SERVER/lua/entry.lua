--[[
Copyright (c) 2015-2016 GUN. All rights reserved
Auther     : miaomiao
Department : GUN
E-mail     : 545911533@qq.com
--]]
module(..., package.seeall);

function main(x)
	suffix = string.match(x.req.get_uri(), "%.(%w+)[^%.]*$")

	if suffix == "css" or suffix == "js" then
		x.set["Cache-Control"] = "max-age=3600"
	end
	
	return true
end


