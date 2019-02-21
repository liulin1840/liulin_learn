
local uci          = require "luci.model.uci"
local curs         =  uci.cursor()
curs:set_config("/tmp")

a = curs:get("audit","global","enable")
print(a)