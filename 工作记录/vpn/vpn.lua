
local utils = require "luci.lib.Utils"
local zkLog = require "luci.lib.ZKLog"
local uci = require "luci.model.uci"
local nixio  = require "nixio"
local fs     = require "nixio.fs"

module("luci.controller.admin.vpn", package.seeall)


-- 定义方法入口
function index()
	-- 默认入口
	entry({"admin", "vpn"}, alias("admin", "vpn", "pptpserver"), _, 50).index = true

	entry({"admin", "vpn", "pptpserver"}, call("server_config_page"), _, 1)
	entry({"admin", "vpn", "pptpclient"}, call("client_config_page"), _, 2)
	entry({"admin", "vpn", "getserverconfig"}, call("get_server_config"), _, 3)
	entry({"admin", "vpn", "saveserverconfig"}, call("save_server_config"), _, 4)
	entry({"admin", "vpn", "getserveruserlist"}, call("get_server_user_list"), _, 5)
	entry({"admin", "vpn", "getserveruserinfo"}, call("get_server_user_info"), _, 6)
	entry({"admin", "vpn", "saveserveruserinfo"}, call("save_server_user_info"), _, 7)
	entry({"admin", "vpn", "deleteserveruserinfo"}, call("delete_server_user_info"), _, 8)
	entry({"admin", "vpn", "getClientConfig"}, call("get_client_config"), _, 9)
	entry({"admin", "vpn", "saveClientConfig"}, call("save_client_config"), _, 10)
	entry({"admin", "vpn", "getClientState"}, call("get_client_state"), _, 11)
	entry({"admin", "vpn", "getOnlineuserList"}, call("get_online"), _, 12)
	entry({"admin", "vpn", "compulsoryDownline"}, call("compulsoryDownline"), _, 13)

	--ipsec 页面
	entry({"admin", "vpn", "ipsec_manage"}, call("ipsec_config_page"), _, 14)
	entry({"admin", "vpn", "ipsecvpn_manage"}, call("ipsecvpn_page"), _, 14)

	--ipsec状态按钮
	entry({"admin", "vpn", "save_ipsec_status"}, call("save_ipsec_status"), _, 32)    		  
	entry({"admin", "vpn", "get_ipsec_status"}, call("get_ipsec_status"), _, 33) 
	 
	--ipsec 隧道
	entry({"admin", "vpn", "save_tunnel"}, call("save_tunnel"), _, 19)        
	entry({"admin", "vpn", "get_tunnel_list"}, call("get_tunnel_list"), _, 20)
	entry({"admin", "vpn", "get_tunnel_info"}, call("get_tunnel_info"), _, 21)          
	entry({"admin", "vpn", "delete_tunnel"}, call("delete_tunnel"), _, 22)   

	--ipsec上传证书
	-- entry({"admin", "vpn", "upload_cacert"}, call("upload_cacert"), _, 34)    		  --根证书
	-- entry({"admin", "vpn", "upload_mycert"}, call("upload_mycert"), _, 35)    		  --本设备证书
	-- entry({"admin", "vpn", "upload_mykey"}, call("upload_mykey"), _, 36)    		  --本设备私钥

	entry({"admin", "vpn", "cacert_manage"}, call("cacert_manage_page"), _, 23)       --证书页面
	entry({"admin", "vpn", "create_ca"}, call("ca_create"), _, 37) 
	entry({"admin", "vpn", "load_all_cas"}, call("get_ca_list"), _, 36)
	entry({"admin", "vpn", "load_all_certs"}, call("get_certs_list"), _, 36) 
	entry({"admin", "vpn", "create_certs"}, call("certs_create"), _, 37) 
	entry({"admin", "vpn", "operation_cacerts"}, call("operation_cacerts"), _, 37) 

	entry({"admin", "vpn", "get_sa_info"}, call("get_sa_info"), _, 38)                 --获取隧道信息

	--strongswan 配置
	entry({"admin", "vpn", "save_ipsec_tunnel"}, call("save_ipsec_tunnel"), _, 40)        
	entry({"admin", "vpn", "get_ipsec_tunnel_list"}, call("get_ipsec_tunnel_list"), _, 41)
	entry({"admin", "vpn", "get_ipsec_tunnel_info"}, call("get_ipsec_tunnel_info"), _, 42)          
	entry({"admin", "vpn", "delete_ipsec_tunnel"}, call("delete_ipsec_tunnel"), _, 43) 
	entry({"admin", "vpn", "relink_ipsec_tunnel"}, call("relink_ipsec_tunnel"), _, 43)   
end

------------------------------------------------------ipsec/ike开始----------------------------------------------------------
function ipsec_config_page(...)
	luci.template.render("vpn/ipsec_config", {navmenu = "vpn_manage", submenu = "vpn_ipsec"})
end

-- 
function ipsecvpn_page(...)
	
	local action = luci.http.formvalue("act")

	if action ~= nil and action == "newipsec" then
		luci.template.render("vpn/ipsec_new", {navmenu = "vpn_manage", submenu = "ipsecvpn_ipsec",message="aaa"})
	else
		luci.template.render("vpn/strongswan_config", {navmenu = "vpn_manage", submenu = "ipsecvpn_ipsec"})
	end
end



function ltn12_popen(command)

	local fdi, fdo = nixio.pipe()
	local pid = nixio.fork()

	if pid > 0 then
		fdo:close()
		local close
		return function()
			local buffer = fdi:read(2048)
			local wpid, stat = nixio.waitpid(pid, "nohang")
			if not close and wpid and stat == "exited" then
				close = true
			end

			if buffer and #buffer > 0 then
				return buffer
			elseif close then
				fdi:close()
				return nil
			end
		end
	elseif pid == 0 then
		nixio.dup(fdo, nixio.stdout)
		fdi:close()
		fdo:close()
		nixio.exec("/bin/sh", "-c", command)
	end
end


local function download_file(path, file_name)
	local sys = require "luci.sys"
	local fs  = require "nixio.fs"

	local read_cmd  = "cat "..path

	local reader = ltn12_popen(read_cmd)

	local download_file_name = 'attachment; filename="'..file_name..'"';
	luci.http.header('Content-Disposition', download_file_name)
	luci.http.prepare_content("application/octet-stream")
	luci.ltn12.pump.all(reader, luci.http.write)
end


function operation_cacerts(...)
	local code,message

	luci.http.prepare_content("application/json")
	local name         = luci.http.formvalue("name")
	local action         = luci.http.formvalue("act")
	if action == "delete_ca" then
		luci.http.prepare_content("application/json")
		code,message = utils.delete_uci_config("cacerts",name)
		local command = "rm /etc/ipsec.d/cacerts/" .. name .. ".cert.pem  >/dev/null 2>&1"
		utils.fork_exec(command)
		luci.http.write_json({ code = code, message = message})

	elseif action == "delete_cert" then
		luci.http.prepare_content("application/json")
		code,message = utils.delete_uci_config("cacerts",name)
		local command = "rm /etc/ipsec.d/certs/" .. name .. ".cert.pem  >/dev/null 2>&1"
		utils.fork_exec(command)
		luci.http.write_json({ code = code, message = message})

	elseif action == "export_ca" then
		local ca_name = name ..".cert.pem"
		local ca_path = "/etc/ipsec.d/cacerts/" ..ca_name
		download_file(ca_path, ca_name)

	elseif action == "export_key" then
		local key_name = name ..".key"
		local _path = "/etc/ipsec.d/private/" .. key_name
		download_file(_path, key_name)


	elseif action == "export_cert" then
		local cert_name = name ..".cert.pem"
		local _path = "/etc/ipsec.d/certs/" ..cert_name
		download_file(_path, cert_name)
	end
	
end


function get_certs_list( ... )
	luci.http.prepare_content("application/json")
	local result,count = utils.get_uci_config("cacerts","cert")

	for k,v in pairs(result) do
		local server_path = "/etc/ipsec.d/certs/" .. v[".name"] ..".cert.pem"
		local info = io.popen("pki --print --in  %q |head -4 " % server_path , "r")
		local command_result = info:read("*all")
		issuer = command_result:match('issuer:%s+%p+([^\n]*)%p+')
		subject = command_result:match('subject:%s+%p+([^\n]*)%p+')
		start, expire = command_result:match('validity:%s+%S+%s+%S+%s+([^,]*).*\n%s+%S+%s+%S+%s+([^,]*)')

		info:close()
		v["issuer"] = issuer
		v["subject"] = subject
		v["start"] = start
		v["expire"] = expire

	end

	luci.http.write_json({ code = 0, rows = result,total = count})
end

function get_ca_list( ... )
	luci.http.prepare_content("application/json")
	local result,count = utils.get_uci_config("cacerts","ca")

	--local utils = require "luci.lib.Utils"
	local result,count = utils.get_uci_config("cacerts","ca")
	for k,v in pairs(result) do
		local ca_path = "/etc/ipsec.d/cacerts/" .. v[".name"] ..".cert.pem"
		local info = io.popen("pki --print --in  %q |head -4 " % ca_path , "r")
		local command_result = info:read("*all")
		issuer = command_result:match('issuer:%s+%p+([^\n]*)%p+')
		subject = command_result:match('subject:%s+%p+([^\n]*)%p+')
		start, expire = command_result:match('validity:%s+%S+%s+%S+%s+([^,]*).*\n%s+%S+%s+%S+%s+([^,]*)')

		info:close()
		v["issuer"] = issuer
		v["subject"] = subject
		v["start"] = start
		v["expire"] = expire

	end

	luci.http.write_json({ code = 0, rows = result,total = count})
end

function certs_create(...)

	luci.http.prepare_content("application/json")

	local descr           = luci.http.formvalue("descr")                       
	local method          = luci.http.formvalue("method") 
	
	local keylen          = luci.http.formvalue("keylen")             
	local lifetime        = luci.http.formvalue("lifetime")  
	local caref        		= luci.http.formvalue("caref")  

	local dn_commonname   = luci.http.formvalue("dn_commonname")
	local dn_country      = luci.http.formvalue("dn_country")   
	local dn_state        = luci.http.formvalue("dn_state")  
	local dn_city         = luci.http.formvalue("dn_city")  
	local dn_organization = luci.http.formvalue("dn_organization")
	local dn_organizationalunit = luci.http.formvalue("dn_organizationalunit") 
	
	local cur = uci.cursor()

	local exist_name = utils.is_name_exist("cacerts","cert", descr)
	if exist_name ~= nil then
		luci.http.write_json({ code =1 , message = "名称已经存在,请重新输入!" })
		return
	end
	local section_name = "cert_" .. descr
	cur:section("cacerts","cert", section_name)

	if (utils.isNotEmpty(descr) ) then
		cur:set("cacerts", section_name, "name", descr)
		cur:set("cacerts", section_name, "method", method)
		if method == "import" then
			-- 将数据存放在文件中
			local cert_data          = luci.http.formvalue("cert") 
			local key_data          = luci.http.formvalue("key")

			local cert_path = "/etc/ipsec.d/certs/" .. section_name ..".cert.pem"
			local cert_key_path = "/etc/ipsec.d/private/" .. section_name ..".key"

			utils.write_file(cert_path, cert_data)
			utils.write_file(cert_key_path, key_data)

			cur:save("cacerts")
			cur:commit("cacerts")
		else
			cur:set("cacerts", section_name, "caref", caref) 
			cur:set("cacerts", section_name, "keylen", keylen) 
			cur:set("cacerts", section_name, "lifetime", lifetime) 
			cur:set("cacerts", section_name, "dn_commonname", dn_commonname) 
			cur:set("cacerts", section_name, "dn_country", dn_country)                                                 
			cur:set("cacerts", section_name, "dn_state", dn_state) 
			cur:set("cacerts", section_name, "dn_city", dn_city) 
			cur:set("cacerts", section_name, "dn_organization", dn_organization)
			cur:set("cacerts", section_name, "dn_organizationalunit", dn_organizationalunit) 
			
			cur:save("cacerts")
			cur:commit("cacerts")
			utils.fork_exec("/usr/sbin/generate_CA.sh cert  %q >/dev/null 2>&1" % section_name)
		end
	end
	luci.http.write_json({ code =0 , message = "Save Success!" })

end

function ca_create(...)

	luci.http.prepare_content("application/json")
	local descr           = luci.http.formvalue("descr")                       
	local method          = luci.http.formvalue("method") 

	local keylen          = luci.http.formvalue("keylen")             
	local lifetime        = luci.http.formvalue("lifetime")  
	local dn_commonname   = luci.http.formvalue("dn_commonname")
	local dn_country      = luci.http.formvalue("dn_country")   
	local dn_state        = luci.http.formvalue("dn_state")  
	local dn_city         = luci.http.formvalue("dn_city")  
	local dn_organization = luci.http.formvalue("dn_organization")
	local dn_organizationalunit = luci.http.formvalue("dn_organizationalunit") 

	local cur = uci.cursor()

	local exist_name = utils.is_name_exist("cacerts","ca", descr)
	if exist_name ~= nil then
		luci.http.write_json({ code =1 , message = "名称已经存在,请重新输入!" })
		return
	end
	local section_name = "ca_".. descr
	cur:section("cacerts","ca", section_name)

	if (utils.isNotEmpty(descr) ) then
		cur:set("cacerts", section_name, "name", descr)
		cur:set("cacerts", section_name, "method", method) 
		if method == "existing" then
			-- 将数据存放在文件中
			local cert_data          = luci.http.formvalue("cert") 
			local key_data          = luci.http.formvalue("key")

			local ca_path = "/etc/ipsec.d/cacerts/" .. section_name ..".cert.pem"
			local ca_key_path = "/etc/ipsec.d/private/" .. section_name ..".key"

			utils.write_file(ca_path, cert_data)
			utils.write_file(ca_key_path, key_data)

			cur:save("cacerts")
			cur:commit("cacerts")
		else
			cur:set("cacerts", section_name, "keylen", keylen) 
			cur:set("cacerts", section_name, "lifetime", lifetime) 
			cur:set("cacerts", section_name, "dn_commonname", dn_commonname) 
			cur:set("cacerts", section_name, "dn_country", dn_country)                                                 
			cur:set("cacerts", section_name, "dn_state", dn_state) 
			cur:set("cacerts", section_name, "dn_city", dn_city) 
			cur:set("cacerts", section_name, "dn_organization", dn_organization)
			cur:set("cacerts", section_name, "dn_organizationalunit", dn_organizationalunit) 
			
			cur:save("cacerts")
			cur:commit("cacerts")
			utils.fork_exec("/usr/sbin/generate_CA.sh ca %q >/dev/null 2>&1" % section_name)
		end
	end
	luci.http.write_json({ code =0 , message = "Save Success!" })
end



function cacert_manage_page(...)
	local action = luci.http.formvalue("act")
	
	if action ~= nil and action == "newca" then
		luci.template.render("vpn/ca", {navmenu = "vpn_manage", submenu = "vpn_cacert"})
	elseif action ~= nil and action == "newcert" then
		luci.template.render("vpn/cacret", {navmenu = "vpn_manage", submenu = "vpn_cacert"})
	else
		luci.template.render("vpn/cacert_manage", {navmenu = "vpn_manage", submenu = "vpn_cacert"})
	end
end
---------------------------------------ipsec_隧道信息-------------------------------------------------
-- /**
--  * @description 保存ipsec启用/禁用状态
--  * @Author      liulin
--  * @DateTime    2017-09-06 星期三
--  */
function save_ipsec_status(...)
	luci.http.prepare_content("application/json")
	local ipsec_status = luci.http.formvalue("ipsec_status")
	
	local cur = uci.cursor()
	
	cur:section("racoon", "enable_status", "ipsec_status")
	cur:set("racoon", "ipsec_status", "ipsec_enable_status", ipsec_status)
	cur:save("racoon")
	cur:commit("racoon") 
	-- 启用增加防火墙的规则
	if(ipsec_status == 'enable') then
		os.execute("sed -i -e '/## zk-ipsec/d' /etc/firewall.user")
		os.execute("echo \"ipsec_firewall.sh ## zk-ipsec\" >> /etc/firewall.user") 
		utils.fork_exec("/etc/init.d/racoon start")        
	else    
		utils.fork_exec("/etc/init.d/racoon stop")
	end
	luci.http.write_json({code = 0, message = "保存成功！"})
end

-- /**
--  * @description 获取ipsec启用/禁用状态
--  * @Author      liulin
--  * @DateTime    2017-09-06 星期三
--  */
function get_ipsec_status( ... )
	luci.http.prepare_content("application/json")	

	local cur    = uci.cursor()
	local status = cur:get("racoon", "ipsec_status", "ipsec_enable_status")

	luci.http.write_json({ code = 0, status = status })
end
-- /**
--  * @description 保存ipsec策略到配置文件 racoon
--  * @Author      liulin
--  * @DateTime    2017-09-06 星期三
--  */
function save_tunnel(...)
	luci.http.prepare_content("application/json")

	local tunnel_name           = luci.http.formvalue("tunnel_name")                       -- 1 隧道名称
	local enabled               = luci.http.formvalue("enabled")                           -- 2 启用安全策略
	local network_mode          = luci.http.formvalue("network_mode")                      -- 3 本地类型
	local local_subnet_ip       = luci.http.formvalue("local_subnet_ip")                   -- 4 本地ip地址
	local local_subnet_mask     = luci.http.formvalue("local_subnet_mask")                 -- 5 本地子网掩码
	local remote_subnet_ip      = luci.http.formvalue("remote_subnet_ip")                  -- 6 对端ip地址
	local remote_subnet_mask    = luci.http.formvalue("remote_subnet_mask")                -- 7 对端子网掩码
	local remote                = luci.http.formvalue("remote")                            -- 8 对端网关	
	local exchange_mode         = luci.http.formvalue("exchange_mode")                     -- 9 协商模式
	local pre_shared_key        = luci.http.formvalue("pre_shared_key")                    -- 10 预共享密钥
	local authentication_method = luci.http.formvalue("authentication_method")             -- 11 认证模式 

	
	-----------------------------以下是加密相关数据----------------------------------------------------------------------
	
	local ph1_encryption_algorithm     = luci.http.formvalue("ph1_encryption_algorithm") --第一阶段加密算法
	local ph1_hash_algorithm           = luci.http.formvalue("ph1_hash_algorithm")       --第一阶段验证算法
	local dh_group                     = luci.http.formvalue("dh_group")                 --第一阶段DH组
	local ph1_lifetime                 = luci.http.formvalue("ph1_lifetime")             --第一阶段密钥生命周期
	local dpd_switch                   = luci.http.formvalue("dpd_switch")               --DPD检测开启
	local dpd_delay                    = luci.http.formvalue("dpd_delay")                --DPD检测周期
	
	local ph2_encryption_algorithm     = luci.http.formvalue("ph2_encryption_algorithm")     --第二阶段加密算法
	local ph2_authentication_algorithm = luci.http.formvalue("ph2_authentication_algorithm")--第二阶段验证算法
	local pfs_group                    = luci.http.formvalue("pfs_group")                    -- 第二阶段PFS群组
	local ph2_lifetime                 = luci.http.formvalue("ph2_lifetime")                 -- 第二阶段密钥生命周期
	local old_tunnel_name              = luci.http.formvalue("old_tunnel_name")       

	------------------------------以下是两端标识--------------------------------------------------------------                                  
	local my_id_type                   = luci.http.formvalue("my_id_type")  
	local peer_id_type                 = luci.http.formvalue("peer_id_type")  
	local my_id                        = luci.http.formvalue("my_id")  
	local peer_id                      = luci.http.formvalue("peer_id")  


	local out_code = 0
	local out_message = "保存失败"
	
	local cur = uci.cursor()
	
	if old_tunnel_name ~= tunnel_name then
		local exist_name = utils.is_name_exist("racoon","zk_tunnel",tunnel_name)
		if exist_name ~= nil then
			luci.http.write_json({ code =1 , message = "名称已经存在,请重新输入!" })
			return
		end
	end
	
	cur:delete("racoon",old_tunnel_name)
	cur:section("racoon","zk_tunnel",tunnel_name)
	
	if (utils.isNotEmpty(tunnel_name) ) then
	
		cur:set("racoon", tunnel_name, "name", tunnel_name)                                                   
		cur:set("racoon", tunnel_name, "enabled", enabled)     
		cur:set("racoon", tunnel_name, "remote", remote)		
		cur:set("racoon", tunnel_name, "network_mode", network_mode)
		cur:set("racoon", tunnel_name, "local_subnet_ip", local_subnet_ip) 
		cur:set("racoon", tunnel_name, "local_subnet_mask", local_subnet_mask)	
		cur:set("racoon", tunnel_name, "remote_subnet_ip", remote_subnet_ip) 
		cur:set("racoon", tunnel_name, "remote_subnet_mask", remote_subnet_mask)

		--两端的标识
		cur:set("racoon", tunnel_name, "my_id_type", my_id_type)
		cur:set("racoon", tunnel_name, "my_id", my_id)
		cur:set("racoon", tunnel_name, "peer_id_type", peer_id_type)
		cur:set("racoon", tunnel_name, "peer_id", peer_id)


		if (remote == "anonymous")then
			exchange_mode = "aggressive,main"
		end

		cur:set("racoon", tunnel_name, "exchange_mode", exchange_mode)
		cur:set("racoon", tunnel_name, "pre_shared_key", pre_shared_key)
		cur:set("racoon", tunnel_name, "authentication_method", authentication_method)

		cur:set("racoon", tunnel_name, "ph1_encryption_algorithm", ph1_encryption_algorithm) 
		cur:set("racoon", tunnel_name, "ph1_hash_algorithm", ph1_hash_algorithm) 
		cur:set("racoon", tunnel_name, "dh_group", dh_group) 
		cur:set("racoon", tunnel_name, "ph1_lifetime", ph1_lifetime) 
		cur:set("racoon", tunnel_name, "dpd_switch", dpd_switch) 
		cur:set("racoon", tunnel_name, "dpd_delay", dpd_delay) 

		cur:set("racoon", tunnel_name, "ph2_encryption_algorithm", ph2_encryption_algorithm) 
		cur:set("racoon", tunnel_name, "ph2_authentication_algorithm", ph2_authentication_algorithm) 
		cur:set("racoon", tunnel_name, "pfs_group", pfs_group) 
		cur:set("racoon", tunnel_name, "ph2_lifetime", ph2_lifetime) 	              
		
		-- p1_proposal
		local p1_proposal_name = "p1_proposal_"..tunnel_name
		cur:delete("racoon",p1_proposal_name)
		cur:section("racoon","p1_proposal",p1_proposal_name)

		cur:set("racoon",p1_proposal_name,"name",p1_proposal_name)
		cur:set("racoon", p1_proposal_name, "lifetime", ph1_lifetime) 
		cur:set("racoon", p1_proposal_name, "enc_alg", ph1_encryption_algorithm) 
		cur:set("racoon", p1_proposal_name, "hash_alg", ph1_hash_algorithm) 
		cur:set("racoon", p1_proposal_name, "auth_method", authentication_method) 
		cur:set("racoon", p1_proposal_name, "dh_group", dh_group) 
		
		-- p2_proposal
		local p2_proposal_name = "p2_proposal_"..tunnel_name
		cur:delete("racoon",p2_proposal_name)
		cur:section("racoon","p2_proposal",p2_proposal_name)

		cur:set("racoon",p2_proposal_name,"name",p2_proposal_name)
		cur:set("racoon", p2_proposal_name, "pfs_group", pfs_group) 
		cur:set("racoon", p2_proposal_name, "ph2_lifetime", ph2_lifetime) 
		cur:set("racoon", p2_proposal_name, "enc_alg", ph2_encryption_algorithm) 
		cur:set("racoon", p2_proposal_name, "auth_alg", ph2_authentication_algorithm) 


		--SAinfo
		local old_sainfo_name  = "sainfo_"..old_tunnel_name
		cur:delete("racoon",old_sainfo_name)
		
		local sainfo_name      = "sainfo_"..tunnel_name	
		cur:section("racoon","sainfo",sainfo_name)
		cur:set("racoon",sainfo_name,"name",sainfo_name)
		cur:set("racoon", sainfo_name, "local_net", local_subnet_ip.."/"..local_subnet_mask) 
		cur:set("racoon", sainfo_name, "remote_net", remote_subnet_ip.."/"..remote_subnet_mask) 
		cur:set("racoon", sainfo_name, "p2_proposal", p2_proposal_name) --第二阶段的名称
		
		--tunnel

		tunnel_name = "tunnel_"..tunnel_name

		cur:delete("racoon","tunnel_"..old_tunnel_name)
		cur:section("racoon","tunnel",tunnel_name)
		
		cur:set("racoon",tunnel_name,"name",tunnel_name)
		cur:set("racoon",tunnel_name,"enabled",enabled)
		cur:set("racoon",tunnel_name,"remote",remote)
		cur:set("racoon",tunnel_name,"exchange_mode",exchange_mode)
		
		if(utils.isNotEmpty(dpd_delay)) then
			cur:set("racoon",tunnel_name,"dpd_delay",dpd_delay)
		end
		
		if(utils.isNotEmpty(pre_shared_key)) then
			cur:set("racoon",tunnel_name,"pre_shared_key",pre_shared_key)
		else
			cur:delete("racoon",tunnel_name,"pre_shared_key")
		end
	
		-- 类型非空且是fqdn,主模式无法连同,切换为积极模式
		if(utils.isNotEmpty(my_id_type) and my_id_type == "fqdn" and utils.isNotEmpty(my_id)) then
			cur:set("racoon",tunnel_name,"my_id_type",my_id_type)
			cur:set("racoon",tunnel_name,"my_id",my_id)
		else
			cur:delete("racoon",tunnel_name,"my_id_type")
			cur:delete("racoon",tunnel_name,"my_id")
		end

		if(utils.isNotEmpty(peer_id_type) and peer_id_type == "fqdn" and utils.isNotEmpty(peer_id)) then
			cur:set("racoon",tunnel_name,"peer_id_type",peer_id_type)
			cur:set("racoon",tunnel_name,"peer_id",peer_id)
		else
			cur:delete("racoon",tunnel_name,"peer_id_type")
			cur:delete("racoon",tunnel_name,"peer_id")
		end


		if(utils.isNotEmpty(authentication_method) and authentication_method == "rsasig") then
			cur:set("racoon",tunnel_name,"certificate","openwrt")
		end
		
		local p1_proposal_list  = {}
		local sainfo_list       = {}
		table.insert(p1_proposal_list,p1_proposal_name)
		table.insert(sainfo_list,sainfo_name)

		if (not utils.table_is_empty(p1_proposal_list)) then 
			cur:set_list("racoon", tunnel_name, "p1_proposal", p1_proposal_list)
		end

		if (not utils.table_is_empty(sainfo_list)) then 
			cur:set_list("racoon", tunnel_name, "sainfo", sainfo_list)
		end

		cur:set("racoon", "ipsec_status", "ipsec_enable_status", "enable")
		cur:save("racoon")
		cur:commit("racoon")
		os.execute("sed -i -e '/## zk-ipsec/d' /etc/firewall.user")
		os.execute("echo \"ipsec_firewall.sh ## zk-ipsec\" >> /etc/firewall.user") 
		utils.fork_exec("/etc/init.d/racoon start")   

		out_message = "保存成功！"
	
	else
		out_message = "配置项数据错误"
		out_code = 0
	end

	luci.http.write_json({ code = out_code, message = out_message,result = result,auth_mode = auth_mode})
end
-- /**
--  * @description 从配置文件 racoon 获取列表数据
--  * @Author      liulin
--  * @DateTime    2017-09-06 星期三
--  */
function get_tunnel_list( ... )
	luci.http.prepare_content("application/json")

	local result,count = utils.get_uci_config("racoon","zk_tunnel")

	luci.http.write_json({ code = 0, rows = result,total = count})
end
-- /**
--  * @description 从配置文件 racoon 获取单条数据
--  * @Author      liulin
--  * @DateTime    2017-09-06 星期三
--  */
function get_tunnel_info( ... )
	luci.http.prepare_content("application/json")

	local name         = luci.http.formvalue('tunnel_name')   
	local result,count = utils.get_uci_config("racoon","zk_tunnel",name)
	
	luci.http.write_json({ code = 0, rows = result,total = count})
end
-- /**
--  * @description 从配置文件 racoon 删除数据
--  * @Author      liulin
--  * @DateTime    2017-09-06 星期三
--  */
function delete_tunnel( ... )
	luci.http.prepare_content("application/json")
	
	local name         = luci.http.formvalue("tunnel_name")
	local code,message = utils.delete_uci_config("racoon",name)
	utils.delete_uci_config("racoon","tunnel_"..name)
	utils.delete_uci_config("racoon","sainfo_"..name)
	utils.delete_uci_config("racoon","p1_proposal_"..name)
	utils.delete_uci_config("racoon","p2_proposal_"..name)
	utils.fork_exec("/etc/init.d/racoon start") 
	
	luci.http.write_json({ code = code, message = message})
end
-----------------------------------------------ipsec第二阶段结束--------------------------------------------------------
-- /**
--  * 上传根证书
--  * @Author   liulin
--  * @DateTime 2017-10-11
--  */
-- function upload_cacert( ... )
-- 	local licence_path = '/etc/racoon/certs/cacert.pem' 
-- 	local file = io.open(licence_path, "rb")
-- 	if file then file:close()
-- 	else
-- 		os.execute("mkdir -p %q" % "/etc/racoon/certs/" )
-- 	end
-- 	local fp
-- 	luci.http.setfilehandler(
-- 		function(meta, chunk, eof)
-- 		  if not fp then
-- 			if meta and meta.name == "image" then
-- 			  fp = io.open(licence_path, "w")
-- 			end
-- 		  end
-- 		  if chunk then
-- 			fp:write(chunk)
-- 		  end
-- 		  if eof then
-- 			fp:close()
-- 		  end
-- 		end
-- 	)
-- 	local fs     = require "nixio.fs"
-- 	if luci.http.formvalue("image") and fs.access("/etc/racoon/certs/cacert.pem") then 
-- 		luci.template.render("vpn/cacert_manage", {navmenu = "vpn_manage", submenu = "vpn_cacert",upload_status ="true"})
-- 	else
-- 		luci.template.render("vpn/cacert_manage", {navmenu = "vpn_manage", submenu = "vpn_cacert",upload_status ="false"})
-- 	end
-- end
-- /**
--  * 上传本设备证书
--  * @Author   liulin
--  * @DateTime 2017-10-11
--  */
-- function upload_mycert( ... )
-- 	local licence_path = '/etc/racoon/certs/mycert.pem' 
-- 	--判断文件的存在性
-- 	local file = io.open(licence_path, "rb")
-- 	if file then file:close()
-- 	else
-- 		os.execute("mkdir -p %q" % "/etc/racoon/certs/" )
-- 	end

-- 	local fp
-- 	luci.http.setfilehandler(
-- 		function(meta, chunk, eof)
-- 		  if not fp then
-- 			if meta and meta.name == "image" then
-- 			  fp = io.open(licence_path, "w")
-- 			end
-- 		  end
-- 		  if chunk then
-- 			fp:write(chunk)
-- 		  end
-- 		  if eof then
-- 			fp:close()
-- 		  end
-- 		end
-- 	)
-- 	local fs     = require "nixio.fs"
-- 	if luci.http.formvalue("image") and fs.access("/etc/racoon/certs/mycert.pem") then 
-- 		luci.template.render("vpn/cacert_manage", {navmenu = "vpn_manage", submenu = "vpn_cacert",upload_status1 ="true"})
-- 	else
-- 		luci.template.render("vpn/cacert_manage", {navmenu = "vpn_manage", submenu = "vpn_cacert",upload_status1 ="false"})
-- 	end
-- end
-- /**
--  * 上传本设备秘钥
--  * @Author   liulin
--  * @DateTime 2017-10-11
--  */
-- function upload_mykey( ... )
-- 	local licence_path = '/etc/racoon/certs/mykey.pem' 
-- 	local file = io.open(licence_path, "rb")
-- 	if file then file:close()
-- 	else
-- 		os.execute("mkdir -p %q" % "/etc/racoon/certs/" )
-- 	end
-- 	local fp
-- 	luci.http.setfilehandler(
-- 		function(meta, chunk, eof)
-- 		  if not fp then
-- 			if meta and meta.name == "image" then
-- 			  fp = io.open(licence_path, "w")
-- 			end
-- 		  end
-- 		  if chunk then
-- 			fp:write(chunk)
-- 		  end
-- 		  if eof then
-- 			fp:close()
-- 		  end
-- 		end
-- 	)
-- 	local fs     = require "nixio.fs"
-- 	if luci.http.formvalue("image") and fs.access("/etc/racoon/certs/mykey.pem") then 
-- 		luci.template.render("vpn/cacert_manage", {navmenu = "vpn_manage", submenu = "vpn_cacert",upload_status2 ="true"})
-- 	else
-- 		luci.template.render("vpn/cacert_manage", {navmenu = "vpn_manage", submenu = "vpn_cacert",upload_status2 ="false"})
-- 	end
-- end
-- /**
--  * 网络测试命令处理，返回命令结果
--  * @Author   liulin
--  * @DateTime 2017-10-25
--  * 
function diag_command(cmd)
	if (cmd ~=nil and cmd ~='') then
		luci.http.prepare_content("text/plain")

		local util = io.popen(cmd)
		local command_result = util:read("*all")
		util:close()

		return command_result
	end

	return
end
-- /**
--  * 获取隧道信息
--  * @Author   liulin
--  * @DateTime 2017-10-25
--  * 
function get_sa_info( ... )

	local sa_fd     = io.popen("setkey -D")
	local sa_object = {}
	local cur       = uci.cursor()
	local fd_setkey = io.open("/etc/config/setkey","w")
	io.close(fd_setkey)
	
	for line in sa_fd:lines() do

		--ip 隧道的方向
		if(string.match(line,"^%d+.%d+.%d+.%d+")) then
			sa_object.ip = line 
		end
		-- 模式
		if(string.find(line,"mode")) then
			local  _,end_mode = string.find(line,"mode")
			local mode        = string.sub(line,end_mode + 2,end_mode + 7)
			sa_object.mode    = mode  or "*"
		end
		-- 状态
		if(string.find(line,"state")) then
			local  _,end_state = string.find(line,"state")
			local state        = string.sub(line,end_state + 2,-1)
			sa_object.state    = state  or "*"
		end

		-- 创建时间
		if(string.find(line,"created")) then
			local _,end_created = string.find(line,"created")
			local created       = string.sub(line,end_created + 2,end_created + 22)
			sa_object.created   = created  or "*"
		end

		-- 数据包
		if(string.find(line,"current")) then
			local _,end_current = string.find(line,"current")
			local start_hard,_  = string.find(line,")") 
			local current       = string.sub(line,end_current + 2,start_hard)
			sa_object.current   = current or "*"
		end

		-- 结束标记
		if(string.find(line,"refcnt")) then
			local name = cur:add("setkey","sa_object")
			--zkLog.log(sa_object) 显示针对异常处理为* 
			cur:set("setkey",name,"ip",sa_object.ip or "*")
			cur:set("setkey",name,"mode",sa_object.mode or "*")
			cur:set("setkey",name,"state",sa_object.state or "*")
			cur:set("setkey",name,"current",sa_object.current or "*")
			cur:set("setkey",name,"created",sa_object.created or "*")

			sa_object = {}
		end
	end
	cur:commit("setkey")
	cur:save("setkey")

	local result,count = utils.get_uci_config("setkey","sa_object")
	luci.http.write_json({ code = 0, rows = result,total = count})
end



------------------------------------------------------ipsec/ike结束-----------------------------------------------------------
local function _get_user_info(id, name)
	local curs=uci.cursor()
	local ifce={}
	curs:foreach("pptpd", "login", function(s) 
		if (id ~= nil) then
			if (s[".name"] == id) then
				ifce = s
				return ifce
			end
		elseif (name ~= nil) then
			if (s["username"] == name) then
				table.insert(ifce, s)
				return ifce
			end
		else
   			ifce[s[".index"]] = s 
   		end
   	end)
	return ifce
end

function server_config_page(...)
	luci.template.render("vpn/server_config", {navmenu = "vpn_manage", submenu = "pptpserver"})
end

function client_config_page(...)
	luci.template.render("vpn/client_config", {navmenu = "vpn_manage", submenu = "pptpclient"})
end

function get_server_config(...)
	luci.http.prepare_content("application/json")

	local cur = uci.cursor()

	local result = {}
	if cur:get("pptpd", "pptpd") then
        result = {
            enabled = cur:get("pptpd", "pptpd", "enabled"),
            clientip = cur:get("pptpd", "pptpd", "remoteip"),
            nat = cur:get("pptpd", "pptpd", "nat"),
            internet = cur:get("pptpd", "pptpd", "internet"),
            dns = cur:get("pptpd", "pptpd", "dns")
        }
    end

	luci.http.write_json({ code = 0, data = result })
end

function save_server_config(...)
	luci.http.prepare_content("application/json")

	local useState = luci.http.formvalue('useState')
	local ipFrom = luci.http.formvalue('ipFrom')
	local ipTo = tonumber(luci.http.formvalue('ipTo'))
	local minTo = tonumber(luci.http.formvalue('minTo'))
	local ipLocal = luci.http.formvalue('localip')
	local useNAT = luci.http.formvalue('useNAT')
	local permitinter = luci.http.formvalue('permitinter')
	local dns_address = luci.http.formvalue('dns_address')
	local outMessage = ""
	local outCode = 1
	if (utils.isNotEmpty(useState) and (useState == 'disabled' or (utils.isNotEmpty(ipFrom) 
		and ipTo > minTo and ipTo < 255))) then
		local cur = uci.cursor()

		if (useState == "disabled") then
			cur:set("pptpd", "pptpd", "enabled", "0")
			cur:delete("pptpd", "pptpd", "remoteip")
			cur:delete("pptpd", "pptpd", "localip")
		else
			cur:set("pptpd", "pptpd", "enabled", "1")
			cur:set("pptpd", "pptpd", "remoteip", ipFrom.. "-" .. ipTo)
			cur:set("pptpd", "pptpd", "localip", ipLocal)
		end
		if (dns_address ~= "") then
			cur:set_list("pptpd", "pptpd", "dns", dns_address)
		end
		cur:set("pptpd", "pptpd", "internet", permitinter)
		cur:set("pptpd", "pptpd", "nat", useNAT)
		
		cur:save("pptpd")
    	cur:commit("pptpd")
    	luci.sys.exec("sleep 1; /etc/init.d/pptpd restart")
		outMessage = "保存成功"
		outCode = 0
	else
		outMessage = "配置项数据错误"
	end

	luci.http.write_json({ code = outCode, message = outMessage })
end

function get_server_user_list(...)
	luci.http.prepare_content("application/json")

	local result = _get_user_info()
	local page = tonumber(luci.http.formvalue("page"))  --第几页
	local rows = tonumber(luci.http.formvalue("rows"))  --每页几条
	local total= 0

	local page_table = {}
	for key,val in ipairs(result) do
		if(key >= (page - 1)*rows+1 and key <= rows*page) then 
			table.insert(page_table,result[key])
		end
		total = total + 1 
	end

	luci.http.write_json({ code = 0, rows = page_table,total = total})
end

function get_server_user_info(...)
	luci.http.prepare_content("application/json")

	local userId = luci.http.formvalue("userid")
	local result = _get_user_info(userId)

	luci.http.write_json({ code = 0, data = result})
end

function save_server_user_info(...)
	luci.http.prepare_content("application/json")

	local userId = luci.http.formvalue("userid")
	local userName = luci.http.formvalue("username")
	local userPwd = luci.http.formvalue("userpwd")

	local outMessage = ""
	local outCode = 1
	if (utils.isNotEmpty(userName) and utils.isNotEmpty(userPwd)) then
		local _tempUser = _get_user_info(nil, userName)

		if (#_tempUser == 0 or _tempUser[1][".name"] == userId) then
			local cur = uci.cursor()

			if (userId == "") then
				userId = cur:add("pptpd", "login")
			end
			cur:set("pptpd", userId, "username", userName)
			cur:set("pptpd", userId, "password", userPwd)

			cur:save("pptpd")
	    	cur:commit("pptpd")
			outMessage = "保存成功" 
			luci.sys.exec("sleep 1; /etc/init.d/pptpd restart")
			outCode = 0
		else
			outMessage = "用户名已经存在，请重新输入"
		end
	else
		outMessage = "配置项数据错误"
	end

	luci.http.write_json({ code = outCode, message = outMessage })
end

function delete_server_user_info(...)
	luci.http.prepare_content("application/json")

	local userId = luci.http.formvalue("userid")

	if (utils.isNotEmpty(userId)) then
		local cur = uci.cursor()

		cur:delete("pptpd", userId)

		cur:save("pptpd")
    	cur:commit("pptpd")
		outMessage = "保存成功"
		luci.sys.exec("sleep 1; /etc/init.d/pptpd restart")
		outCode = 0
	else
		outMessage = "参数错误"
	end

	luci.http.write_json({ code = 0, message = "删除成功" })
end

function get_client_config(...)
	luci.http.prepare_content("application/json")
	local curs=uci.cursor()
	local _disabled = curs:get("network", "vpn", "disabled")
	local _username = curs:get("network", "vpn", "username")
	local _password = curs:get("network", "vpn", "password")
	local _server = curs:get("network", "vpn", "server")
	local _remote_net = curs:get("network", "vpn", "remotenet")
	local _remote_mask = curs:get("network", "vpn", "remotemask")
	local _defaultroute =curs:get("network", "vpn", "defaultroute")
	local _peerdns = curs:get("network", "vpn", "peerdns")

	local _useState = "disabled"
	if (_disabled == "0") then
		_useState = "enable"
	else
		_useState = "disabled"
	end

	local result = {
		useState = _useState, 
		serverIp = _server, 
		username = _username, 
		userpwd  = _password,
		remote_net =  _remote_net ,
		remote_mask =  _remote_mask,
		defaultroute =  _defaultroute ,
		peerdns =  _peerdns
	}

	luci.http.write_json({ code = 0, data = result })
end

local function _get_firewall_noteid(_type)
    local curs=uci.cursor()
    local _id = false
    curs:foreach("firewall", "zone", function(s) 
        if (s["name"] == _type) then
            _id = s[".name"]
        end
    end)
    return _id
end

function save_client_config(...)
	luci.http.prepare_content("application/json")

	local useState = luci.http.formvalue('useState')
	local serverIp = luci.http.formvalue('serverIp')
	local username = luci.http.formvalue('username')
	local userpwd = luci.http.formvalue('userpwd')
	local serdns = luci.http.formvalue('serdns')
	--local defaultroute = luci.http.formvalue('defaultroute')
	local remote_net = luci.http.formvalue('remote_net')
	local remote_mask = luci.http.formvalue('remote_mask')
	local auto = 0
	if useState == 'disabled' then
		disabled = 1
	else
		disabled = 0
	end	
	local defaultroute = "0"
	local outMessage = ""
	local outCode = 1
	local section_name = "vpn"
	local curs = uci.cursor()
	if (utils.isNotEmpty(useState) and (useState == 'disabled' or (utils.isNotEmpty(serverIp) 
		and utils.isNotEmpty(username) and utils.isNotEmpty(userpwd)))) then	

		luci.sys.exec("uci set network.vpn=interface")
		-- luci.sys.exec("uci set network.vpn.ifname='pptp-vpn'")
		luci.sys.exec("uci set network.vpn.proto='pptp'")
		-- luci.sys.exec("uci set network.vpn.mtu='1400'")
		-- luci.sys.exec("uci set network.vpn.metric='1'")
		luci.sys.exec("uci set network.vpn.keepalive='10 10'")
		luci.sys.exec("uci set network.vpn.ipv6='0'")
		luci.sys.exec("uci set network.vpn.username='" .. username .. "'")
		luci.sys.exec("uci set network.vpn.password='" .. userpwd .. "'")
		luci.sys.exec("uci set network.vpn.server='" .. serverIp .. "'")
		luci.sys.exec("uci set network.vpn.defaultroute='" .. defaultroute .. "'")
		luci.sys.exec("uci set network.vpn.peerdns='" .. serdns .. "'")
		luci.sys.exec("uci set network.vpn.disabled='" .. disabled .. "'")
		luci.sys.exec("uci set network.vpn.auto='1'")

		luci.sys.exec("uci set network.vpn.remotenet='" .. remote_net .. "'")
		luci.sys.exec("uci set network.vpn.remotemask='" .. remote_mask .. "'")

		luci.sys.exec("uci commit network")	
		
		local id  = _get_firewall_noteid("wan")

		local _data_list = curs:get_list("firewall", id, "network")
   		local _save_list = {}
	    for key, value in pairs(_data_list) do
	    	if(value ~= "vpn") then  	        
	            table.insert(_save_list, value)	
	        end        
	    end
		if useState == 'enable' then
			table.insert(_save_list, "vpn")
		end	
		curs:set_list("firewall", id, "network", _save_list)

		curs:save("firewall")
   	 	curs:commit("firewall")

		--utils.fork_exec("/etc/init.d/network reload; sleep 1;")
		--utils.fork_exec("/etc/init.d/firewall restart; sleep 1;/etc/init.d/dnsmasq restart")

		outMessage = "保存成功"
		outCode = 0
	else
		outMessage = "配置项不能为空"
	end

	luci.http.write_json({ code = outCode, message = outMessage })

        -- apply save
	if useState == 'disabled' then
        luci.sys.call("env -i /sbin/ifdown vpn >/dev/null 2>/dev/null")
    else
        luci.sys.call("env -i /sbin/ifup vpn >/dev/null 2>/dev/null")
    end
end

function get_client_state(...)
	luci.http.prepare_content("application/json")
	local result
	local fs     = require "nixio.fs"
	if not(fs.access("/tmp/log/vpn_status.log")) then
		result = {
			linkState = "disconnected", 
			duration = 0
		}
	else
		local uptimestamp, ifname ,local_ip,remore_ip,ipparam, state
		for e in io.lines("/tmp/log/vpn_status.log") do
			uptimestamp, ifname ,local_ip,remore_ip,ipparam, state = e:match("^(.*)%s(.*)%s(.*)%s(.*)%s(.*)%s(.*)")
		end
		local now_tiime = os.time()
		result = {
			linkState = state, 
			duration = now_tiime - uptimestamp
		}
	end
	luci.http.write_json({ code = 0, data = result })
end


function get_online(...)

	luci.http.prepare_content("application/json")
	local result = {}
	local page = tonumber(luci.http.formvalue("page"))  --第几页
	local rows = tonumber(luci.http.formvalue("rows"))  --每页几条
	local total= 0

	local vpn_client_dat = "/var/run/pptpd_client.dat"
	if fs.access(vpn_client_dat, "r") then
		local file = io.open(vpn_client_dat ,"r");
	  	for line in file:lines() do
			local status, pid, timestamp, device, user, cip, gwip, vip = line:match(
	        		"^ *(.+) +(.+) (.+) (.+) +(.+) (.+) (.+) +(.+)"
	        	)
			local idx = tonumber(pid)
			local now_tiime = os.time()
			if idx and status == "up" then
			   result[#result + 1] = {
				['PID'] = pid,
				['device'] = device,
				['user'] = user,
				['duration'] = now_tiime - timestamp,
				['status'] = status,
				['vpn_gw'] = gwip,
				['vpn_ip'] = vip,
				['clent_ip'] = cip
			   }
			end
		end
	end

	local page_table = {}
	for key,val in ipairs(result) do
		if(key >= (page - 1)*rows+1 and key <= rows*page) then 
			table.insert(page_table,result[key])
		end
		total = total + 1 
	end
	luci.http.write_json({ code = 0, rows = page_table,total = total })
end

function compulsoryDownline(...)
	luci.http.prepare_content("application/json")
	local PID = luci.http.formvalue("PID")
	local returnMessage = "强制下线成功"

	luci.http.write_json({ code = 0, message = returnMessage })
end	
---------------------------------------------------strongswan--vpn------------------------------------------------------------
function save_ipsec_tunnel(...)
	luci.http.prepare_content("application/json")
	local tunnel_name                  = luci.http.formvalue("tunnel_name")                       -- 1 隧道名称
	local enabled                      = luci.http.formvalue("enabled")                           -- 2 启用安全策略
	local local_subnet_ip              = luci.http.formvalue("local_subnet_ip")                   -- 4 本地ip地址
	local local_subnet_mask            = luci.http.formvalue("local_subnet_mask")                 -- 5 本地子网掩码
	local remote_subnet_ip             = luci.http.formvalue("remote_subnet_ip")                  -- 6 对端ip地址
	local remote_subnet_mask           = luci.http.formvalue("remote_subnet_mask")                -- 7 对端子网掩码
	local remote                       = luci.http.formvalue("remote")                            -- 8 对端网关	
	local exchange_mode                = luci.http.formvalue("exchange_mode")                     -- 9 协商模式
	local pre_shared_key               = luci.http.formvalue("pre_shared_key")                    -- 10 预共享密钥
	local authentication_method        = luci.http.formvalue("authentication_method")             -- 11 认证模式 
	local local_identifier             = luci.http.formvalue("local_identifier")                  -- 12 本地标识 
	local remote_identifier            = luci.http.formvalue("remote_identifier")                 -- 13 对端标识 

	local peerid_type                  = luci.http.formvalue("peerid_type")                       -- 14 对端类型
	local myid_type                    = luci.http.formvalue("myid_type")                         -- 15 本地类型 
	local ph1_user                     = luci.http.formvalue("ph1_user")                          -- 16 扩展认证用户 
	local ph1_pass                     = luci.http.formvalue("ph1_pass")                          -- 17 扩展认证密码
	local certref                      = luci.http.formvalue("certref")                           -- 18 my证书
	local caref                        = luci.http.formvalue("caref")                             -- 19 peer 证书 
	local ph1_key_length               = luci.http.formvalue("ph1_key_length")                    -- 20 秘钥长度
	local ipsecvpn_mode                = luci.http.formvalue("ipsecvpn_mode")                     -- 21 ikev1 模式选择
 	local dpd_switch_action            = luci.http.formvalue("dpd_switch_action")                 -- 22 dpd_switch_action

	-----------------------------以下是加密相关数据----------------------------------------------------------------------
	local key_exchange                 = luci.http.formvalue("key_exchange") --ike version
	local ph1_encryption_algorithm     = luci.http.formvalue("ph1_encryption_algorithm")         --第一阶段加密算法
	local ph1_hash_algorithm           = luci.http.formvalue("ph1_hash_algorithm")               --第一阶段验证算法
	local dh_group                     = luci.http.formvalue("dh_group")                          --第一阶段DH组
	local ph1_lifetime                 = luci.http.formvalue("ph1_lifetime")                      --第一阶段密钥生命周期
	local dpd_switch                   = luci.http.formvalue("dpd_switch")                        --DPD检测开启
	local dpd_delay                    = luci.http.formvalue("dpd_delay")                         --DPD检测周期
	local dpd_switch_action            = luci.http.formvalue("dpd_switch_action")                  -- dpd 行为
	local ph2_encryption_algorithm     = luci.http.formvalue("ph2_encryption_algorithm")          --第二阶段加密算法
	local ph2_authentication_algorithm = luci.http.formvalue("ph2_authentication_algorithm")      --第二阶段验证算法
	local pfs_group                    = luci.http.formvalue("pfs_group")                         -- 第二阶段PFS群组
	local ph2_lifetime                 = luci.http.formvalue("ph2_lifetime")                      -- 第二阶段密钥生命周期
	local old_tunnel_name              = luci.http.formvalue("old_tunnel_name")     
	local ph2_key_length               = luci.http.formvalue("ph2_key_length")                    -- 20 秘钥长度    
	local sub_arry                     = luci.http.formvalue("sub_arry")  or  nil                     -- 多子网  
	local user_arry                    = luci.http.formvalue("user_arry") or  nil                       -- 多用户

	local xauth_mode                    = luci.http.formvalue("xauth_mode")                        
	local virtual_ip                    = luci.http.formvalue("virtual_ip")           
	local virtual_ip_mask               = luci.http.formvalue("virtual_ip_mask")    
	local auto_negotiation              = luci.http.formvalue("auto_negotiation")                -- 自动协商          
	

	local out_code = 0
	local out_message = "保存失败"
	local cur = uci.cursor()

	if old_tunnel_name ~= tunnel_name then
		local exist_name = utils.is_name_exist("update","zk_tunnel",tunnel_name)
		if exist_name ~= nil then
			luci.http.write_json({ code =1 , message = "名称已经存在,请重新输入!" })
			return
		end
	end

	cur:delete("update",old_tunnel_name)
	cur:section("update","zk_tunnel",tunnel_name)
	if (utils.isNotEmpty(tunnel_name) ) then
		cur:set("update", tunnel_name, "name", tunnel_name)                                                   
		cur:set("update", tunnel_name, "enabled", enabled)     
		cur:set("update", tunnel_name, "remote", remote)		
		cur:set("update", tunnel_name, "local_subnet_ip", local_subnet_ip) 
		cur:set("update", tunnel_name, "local_subnet_mask", local_subnet_mask)	
		cur:set("update", tunnel_name, "remote_subnet_ip", remote_subnet_ip) 
		cur:set("update", tunnel_name, "remote_subnet_mask", remote_subnet_mask)
		cur:set("update", tunnel_name, "remote_subnet_ip", remote_subnet_ip) 
		cur:set("update", tunnel_name, "remote_subnet_mask", remote_subnet_mask)
		cur:set("update", tunnel_name, "local_identifier", local_identifier)
		cur:set("update", tunnel_name, "remote_identifier", remote_identifier)

		cur:set("update", tunnel_name, "key_exchange", key_exchange)
		cur:set("update", tunnel_name, "pre_shared_key", pre_shared_key)
		cur:set("update", tunnel_name, "authentication_method", authentication_method)
		cur:set("update", tunnel_name, "ph1_encryption_algorithm", ph1_encryption_algorithm) 
		cur:set("update", tunnel_name, "ph1_hash_algorithm", ph1_hash_algorithm) 
		cur:set("update", tunnel_name, "dh_group", dh_group) 
		cur:set("update", tunnel_name, "ph1_lifetime", ph1_lifetime) 
		cur:set("update", tunnel_name, "dpd_switch", dpd_switch) 
		cur:set("update", tunnel_name, "dpd_delay", dpd_delay) 
		cur:set("update", tunnel_name, "ph2_encryption_algorithm", ph2_encryption_algorithm) 
		cur:set("update", tunnel_name, "ph2_authentication_algorithm", ph2_authentication_algorithm) 
		cur:set("update", tunnel_name, "pfs_group", pfs_group) 
		cur:set("update", tunnel_name, "ph2_lifetime", ph2_lifetime) 

		--新增参数
		cur:set("update", tunnel_name, "peerid_type", peerid_type) 
		cur:set("update", tunnel_name, "myid_type", myid_type) 
		cur:set("update", tunnel_name, "ph1_user", ph1_user) 
		cur:set("update", tunnel_name, "ph1_pass", ph1_pass) 
		cur:set("update", tunnel_name, "certref", certref) 
		cur:set("update", tunnel_name, "caref", caref) 
		cur:set("update", tunnel_name, "ph1_key_length", ph1_key_length)      
		cur:set("update", tunnel_name, "ipsecvpn_mode", ipsecvpn_mode)                        -- 主模式/积极模式 ikev1 模式下
		cur:set("update", tunnel_name, "dpd_switch_action",dpd_switch_action) 
		cur:set("update", tunnel_name, "ph2_key_length",ph2_key_length) 
		cur:set("update", tunnel_name, "sub_arry",sub_arry) 
		cur:set("update", tunnel_name, "user_arry",user_arry) 

		cur:set("update", tunnel_name, "xauth_mode",xauth_mode) 
		cur:set("update", tunnel_name, "virtual_ip",virtual_ip) 
		cur:set("update", tunnel_name, "virtual_ip_mask",virtual_ip_mask)     
		cur:set("update", tunnel_name, "auto_negotiation",auto_negotiation)     

		cur:save("update")
		cur:commit("update")

		-- p1_proposal
		local p1_proposal_name = "p1_proposal_"..tunnel_name
		cur:delete("ipsec",p1_proposal_name)
		cur:section("ipsec","p1_proposal",p1_proposal_name)

		cur:set("ipsec",p1_proposal_name,"name",p1_proposal_name)
		
		cur:set("ipsec", p1_proposal_name, "encryption_algorithm", ph1_encryption_algorithm..ph1_key_length) -- 第一阶段加密
		cur:set("ipsec", p1_proposal_name, "hash_algorithm", ph1_hash_algorithm)             -- 第一阶段验证算法
		cur:set("ipsec", p1_proposal_name, "dh_group", dh_group)                             -- 第一阶段DH组
		--cur:set("ipsec", p1_proposal_name, "ph1_key_length", ph1_key_length)                 -- 第一阶段key length


		-- p2_proposal 
		local p2_proposal_name = "p2_proposal_"..tunnel_name
		cur:delete("ipsec",p2_proposal_name)
		cur:section("ipsec","p2_proposal",p2_proposal_name)

		cur:set("ipsec",p2_proposal_name,"name",p2_proposal_name) 
		if pfs_group ~= "0" then
			cur:set("ipsec", p2_proposal_name, "pfs_group", pfs_group)                                   -- 第二阶段PFS群组
		end
		                        -- 第二阶段周期
		cur:set("ipsec", p2_proposal_name, "encryption_algorithm", ph2_encryption_algorithm..ph2_key_length)         -- 加密
		cur:set("ipsec", p2_proposal_name, "authentication_algorithm", ph2_authentication_algorithm) -- 验证 

		--tunnel 
		local old_sainfo_name  = "tunnel_"..old_tunnel_name
		cur:delete("ipsec",old_sainfo_name)
		local sainfo_name      = "tunnel_"..tunnel_name	
		cur:section("ipsec","tunnel",sainfo_name)

		cur:set("ipsec", ainfo_name,"name",sainfo_name)
		cur:set("ipsec", sainfo_name, "local_subnet", local_subnet_ip.."/"..local_subnet_mask) 
		--cur:set("ipsec", sainfo_name, "remote_subnet", remote_subnet_ip.."/"..remote_subnet_mask) 

	

		--user list 用户名/密码也用list 表示
		local p1_user_list  = {}
		table.insert(p1_user_list,ph1_user.."/"..ph1_pass)
		if(user_arry ~= nil) then
			local user_tab = utils.split(user_arry,",")
			for i,v in ipairs(user_tab) do
				table.insert(p1_user_list,user_tab[i])
			end
		end
		cur:set_list("ipsec", sainfo_name, "user", p1_user_list)


		cur:set("ipsec", sainfo_name, "crypto_proposal", p2_proposal_name)              --第二阶段的名称
		cur:set("ipsec", sainfo_name, "key_exchange", key_exchange)                     --ike版本
		cur:set("ipsec", sainfo_name, "mode", "start")                                  --ikev2
		cur:set("ipsec", sainfo_name, "force_crypto_proposal", "1")                     --ikev2
		cur:set("ipsec", sainfo_name,"dpd_delay",dpd_delay)                             --dpd 检查时间
		cur:set("ipsec", sainfo_name,"dpd_action",dpd_switch_action) 				    --dpd 行为
		cur:set("ipsec", sainfo_name,"dpd_switch",dpd_switch)                           --dpd 开关

		cur:set("ipsec", sainfo_name, "ikelifetime", ph1_lifetime.."s")                     -- 第一阶段密钥生命周期
		cur:set("ipsec", sainfo_name, "lifetime", ph2_lifetime.."s")     

		--cur:set("ipsec", sainfo_name, "ph1_user", ph1_user) 
		--cur:set("ipsec", sainfo_name, "ph1_pass", ph1_pass)
		cur:set("ipsec", sainfo_name, "aggressive", ipsecvpn_mode)  
		cur:set("ipsec", sainfo_name, "cert", certref) 
		cur:set("ipsec", sainfo_name, "ca", caref) 

		cur:set("ipsec", sainfo_name, "xauth_mode",xauth_mode) 
		if xauth_mode == "server" then
			cur:set("ipsec", sainfo_name, "virtual_addr_pool",remote_subnet_ip.."/"..remote_subnet_mask)
		else
			--remote sub list  对端子网用list 表示
			local p1_remote_list  = {}
			table.insert(p1_remote_list,remote_subnet_ip.."/"..remote_subnet_mask)
			
			if(sub_arry ~= nil) then
				local remote_sub_tab = utils.split(sub_arry,",")
				for i,v in ipairs(remote_sub_tab) do
					table.insert(p1_remote_list,remote_sub_tab[i])
				end
			end
			cur:set_list("ipsec", sainfo_name, "remote_subnet", p1_remote_list)
		end
		cur:set("ipsec", sainfo_name, "auto_negotiation",auto_negotiation) 
		 
		--remote
		tunnel_name = "remote_"..tunnel_name

		cur:delete("ipsec","remote_"..old_tunnel_name)
		cur:section("ipsec","remote",tunnel_name)

		cur:set("ipsec",tunnel_name,"name",tunnel_name)
		cur:set("ipsec",tunnel_name,"enabled",enabled)
		
		if(remote == "0.0.0.0") then remote = "any" end
		cur:set("ipsec",tunnel_name,"gateway",remote)
		cur:set("ipsec", tunnel_name, "authentication_method", authentication_method) 
		cur:set("ipsec", tunnel_name, "local_identifier", local_identifier) 
		cur:set("ipsec", tunnel_name, "remote_identifier", remote_identifier) 
		cur:set("ipsec", tunnel_name, "peerid_type", peerid_type) 
		cur:set("ipsec", tunnel_name, "myid_type", myid_type) 

		if(utils.isNotEmpty(pre_shared_key)) then
			cur:set("ipsec",tunnel_name,"pre_shared_key",pre_shared_key)
		else
			cur:delete("ipsec",tunnel_name,"pre_shared_key")
		end
		local p1_proposal_list  = {}
		local sainfo_list       = {}
		table.insert(p1_proposal_list,p1_proposal_name)
		table.insert(sainfo_list,sainfo_name)

		if (not utils.table_is_empty(p1_proposal_list)) then 
			cur:set_list("ipsec", tunnel_name, "crypto_proposal", p1_proposal_list)
		end
		if (not utils.table_is_empty(sainfo_list)) then 
			cur:set_list("ipsec", tunnel_name, "tunnel", sainfo_list)
		end

		cur:save("ipsec")
		cur:commit("ipsec")
		os.execute("ipsec stop >/dev/null 2>&1;sleep 1;/etc/init.d/ipsec restart >/dev/null 2>&1;ipsec start >/dev/null 2>&1")   
		out_message = "保存成功！"
	else
		out_message = "配置项数据错误"
		out_code = 0
	end
	luci.http.write_json({ code = out_code, message = out_message,result = result})
end

function get_ipsec_tunnel_list( ... )
	luci.http.prepare_content("application/json")
	local result,count = utils.get_uci_config("update","zk_tunnel")

	for k,v in pairs(result) do
		local tunnel_name = "tunnel_" .. v[".name"]

		local cmd = "ipsec status " .. tunnel_name
		local tunnel = io.popen(cmd , "r")
		if tunnel then
			while true do
				local file = tunnel:read("*l")
				if not file then
					break
				elseif file:match("IPsec SA established") or file:match("INSTALLED") then
					status="CONNECTED"
					break
				elseif file:match("CONNECTING")  then
					status="CONNECTING"
					break
				elseif file:match("ROUTED")  then
					status="ON-DEMAND"
					break
				else
					status="DISCONNECTED"
				end
			end
			tunnel:close()
		end
		v["status"] = status
	end

	luci.http.write_json({ code = 0, rows = result,total = count})
end
-- /**
--  * @description 从配置文件 ipsec 获取单条数据
--  * @Author      liulin
--  * @DateTime    2019年3月23日
--  */
function get_ipsec_tunnel_info( ... )
	luci.http.prepare_content("application/json")
	local name         = luci.http.formvalue('tunnel_name')   
	local result,count = utils.get_uci_config("update","zk_tunnel",name)
	luci.http.write_json({ code = 0, rows = result,total = count})
end
-- /**
--  * @description 从配置文件 ipsec 删除数据
--  * @Author      liulin
--  * @DateTime    2019年3月23日
--  */
function delete_ipsec_tunnel( ... )
	luci.http.prepare_content("application/json")
	local name         = luci.http.formvalue("tunnel_name")
	local code,message = utils.delete_uci_config("update",name)
	utils.delete_uci_config("ipsec","tunnel_"..name)
	utils.delete_uci_config("ipsec","remote_"..name)
	utils.delete_uci_config("ipsec","p1_proposal_"..name)
	utils.delete_uci_config("ipsec","p2_proposal_"..name)
	utils.fork_exec("/etc/init.d/ipsec start") 
	luci.http.write_json({ code = code, message = message})
end

-- /**
--  * 重连
--  * @Author   liulin
--  * @DateTime 2019-04-25T11:20:16+0800
--  * @param    {...[type]}               [description]
--  * @return   {[type]}                  [description]
--  */
function relink_ipsec_tunnel( ... )
	luci.http.prepare_content("application/json")
	local name         = luci.http.formvalue("tunnel_name")
	local tunnel_name  = "tunnel_"..name
	local relink_cmd   = "ipsec up "..tunnel_name
	utils.fork_exec(relink_cmd) 
	--zkLog.log(relink_cmd)
	luci.http.write_json({ code = 0, message = "relink ok!"})
end
