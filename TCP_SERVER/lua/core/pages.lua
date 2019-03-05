--[[
Copyright (c) 2015-2016 GUN. All rights reserved
Auther     : miaomiao
Department : GUN
E-mail     : 545911533@qq.com
--]]
module(..., package.seeall);

function createErrorPage(x, status)
	local content = {}
	
	content[#content + 1] = "<html><body bgcolor=\"white\"><title>500 Internal Server Error</title><style>body"
	content[#content + 1] = "{nargin:auto;font-family:Tahoma, Geneva, sans-serif;}</style>"
	content[#content + 1] = "<h1 align=\"center\">500 Internal Server Error</h1>"
	content[#content + 1] = "<hr><p align=\"center\">Error Info:" .. status .. "</p></body></html>"
	content = table.concat(content)
	
	x.set["Content-type"] = "text\html"
	x.set["Content-Length"] = content:len()
	
	return content
end

html = 
{
	["404.html"] = 
	[[
	<html>
	<body bgcolor="white">
	<title>404 Not Found</title>
	<style>
	body
	{
		nargin:auto;
		font-family:Tahoma, Geneva, sans-serif;
	}
	</style>
	<h1 align="center">T_T 404 Not Found</h1>
	<hr><p align="center">Copyright (c) 2015-2016 <a href = "http://www.wayos.com"> GUN. </a> All rights reserved</p>
	<p align="center">Author:miaomiao </p>
	<p align="center">Departmendt:GUN </p>
	<p align="center">E-mail:545911533@qq.com</p>
	</body>
	</html>
	]]
	,
	["index.html"] = 
	[[<html>
	<body>
	<title>Welcome X-server 1.0</title>
	<style>
		body
		{
			nargin:auto;
			font-family:Tahoma, Geneva, sans-serif;
		}
	</style>
	<h1 align='center'>欢迎使用 X-server 1.0</h1>
	<hr><p align='center'>Copyright (c) 2015-2016 <a href = 'http://www.wayos.com'> GUN. </a> All rights reserved</p>
	<p align='center'>Author:miaomiao </p>
	<p align='center'>Departmendt:GUN </p>
	<p align='center'>E-mail:545911533@qq.com</p>
	</body>
	</html>]]
}