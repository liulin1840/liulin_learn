module(..., package.seeall);

local pub  = require("public")
local io = require "io"

function main(x)
	local imgs = 
	{
		"02t2","0a06","0ncu","3kqz","5b2a",
		"5p9g","5td9","6igm","6tc8","9q89",
		"c599","f361","j5uk","n8bj","q4w5",
		"qe09","u951","yiw0","z57d","z6b2"
	}
	
	math.randomseed(tostring(os.time()):reverse():sub(1,6))
	
	local index = math.random(1,20)
	local img = "/data/MT7620/source/user/jhl_tools/x-server/lua/code/" .. imgs[index] .. ".jpg"
	local ok, jpg = pub.fileRead(img)
	
	
	x.set["Set-Cookie"] = imgs[index]
	return 	jpg
end

