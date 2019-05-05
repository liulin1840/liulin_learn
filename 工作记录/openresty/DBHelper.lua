-- author 2016 yusong<3674297@qq.com>
-- sqlite 数据库封装

local M          = {};
local modelName  = ...;
_G[modelName]    = M;

-- 打开数据库连接
local function openConnection()
	local cur = uci.cursor();
	local db   
	local conn 
	local mysql = require "luasql.mysql"
	db          = mysql.mysql()
	conn        = db:connect('miaomiao','root','MIAOmiao1988++', '127.0.0.1', 3306)
    return db, conn
end

-- 关闭数据库连接
local function closeConnection(db, conn)
	conn:close();
    db:close();
end



--[[
	自定义SQL查询
	query("select * from xxx")
	@_sql		string 	完整SQL语句
	@return 	object 	记录数据，false为空
]]
function M.query(_sql)
	local db, conn = openConnection();
	if _sql == '' then
		return false, "sql is empty"
	end

	local cur, errorMsg = conn:execute(_sql)

    if cur == nil then
        closeConnection(db, conn)
        return false, errorMsg
    end
    local row = cur:fetch({}, "a")
    local result = {}
    while row do
    	table.insert(result, utils.copyTable(row))
    	row = cur:fetch(row, "a")
    end
    cur:close()
    closeConnection(db, conn)
    return result, errorMsg
end

return M;


