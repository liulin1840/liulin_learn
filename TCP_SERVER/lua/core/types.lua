--[[
Copyright (c) 2015-2016 GUN. All rights reserved
Auther     : miaomiao
Department : GUN
E-mail     : 545911533@qq.com
--]]

local _M = { _VERSION = '0.01'}
local mt = { __index = _M }


mt.contentTypes = 
{
    ["html"] = "text/html",
    ["htm"] = "text/html",
    ["shtml"] = "text/html",
    ["css"] = "text/css", 
    ["xml"] = "text/xml", 
    ["gif"] = "image/gif",
    ["jpg"] = "image/jpeg",
	["jpeg"] = "image/jpeg",
    ["js"] = "application/javascript",
    ["atom"] = "application/atom+xml",
    ["rss"] = "application/rss+xml",

    ["mml"] = "text/mathml",
    ["txt"] = "text/plain",
	["data"] = "text/plain",
    ["jad"] = "text/vnd.sun.j2me.app-descriptor",
    ["wml"] = "text/vnd.wap.wml",
    ["htc"] = "text/x-component",

    ["png"] = "image/png",
    ["tif"] = "image/tiff",
    ["tiff"] = "image/tiff",
    ["wbmp"] = "image/vnd.wap.wbmp",
    ["ico"] = "image/x-icon",
    ["jng"] = "image/x-jng",
    ["bmp"] = "image/x-ms-bmp",
    ["svg"] = "image/svg+xml",
    ["svgz"] = "image/svg+xml",
    ["webp"] = "image/webp",
    
    ["woff"] = "application/font-woff",
    ["jar"] = "application/java-archive",
    ["war"] = "application/java-archive",
    ["ear"] = "application/java-archive",
    ["json"] = "application/json",
    ["hqx"] = "application/mac-binhex40",
    ["doc"] = "application/msword",
    ["pdf"] = "application/pdf",
    ["ps"] = "application/postscript",
    ["eps"] = "application/postscript",
    ["ai"] = "application/postscript",
    ["rtf"] = "application/rtf",
    ["m3u8"] = "application/vnd.apple.mpegurl",
	["xls"] = "application/vnd.ms-excel",
    ["eot"] = "application/vnd.ms-fontobject",
    ["ppt"] = "application/vnd.ms-powerpoint",
    ["wmlc"] = "application/vnd.wap.wmlc",  
    ["kml"] = "application/vnd.google-earth.kml+xml",
    ["kmz"] = "application/vnd.google-earth.kmz",
    ["7z"] = "application/x-7z-compressed",
    ["cco"] = "application/x-cocoa",
    ["jardiff"] = "application/x-java-archive-diff",
    ["jnlp"] = "application/x-java-jnlp-file",
	["default"] = "application/octet-stream"
}

return mt

