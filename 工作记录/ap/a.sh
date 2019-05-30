#!/bin/sh
#
for i in $(seq 1 10);
do
	echo "aaaaa"
	curl -d "ZK={ \"gw_id\": \"001234567851\", \"vendor_id\": \"ZK\", \"device_type\": \"TCA82\", \"product_type\": \"TCA82-B\", \"fm_name\": \"ZK-TCA82-B-4.3.0.4090-1806-stable\", \"soft_ver\": \"4.3.0.4090\", \"sys_load\": 10, \"status_interval\": 2, \"station_agingtime\": 10, \"timer_reboot\": 0, \"ipaddr_release\": 5, \"auth_server\": \"wifitest.zkgcloud.com\", \"auth_path\": \"\/wacs\/\", \"auth_port\": 8080, \"ac_server\": \"115.29.236.125\", \"ac_path\": \"\/wacs-mana\/\", \"ac_port\": 8090, \"ac_bak_hostname\": \"auth.zkgcloud.com\", \"ac_bak_path\": \"\/wacs-mana\/\", \"ac_bak_port\": 8090, \"ac_bak_sync_interval\": 30, \"acdisc_enable\": 1, \"auth_enable\": 1, \"portal_proxy\": 1, \"apple_auto_portal\": 1, \"escape_mode_enable\": 0, \"auth_protocol\": \"zk-portal\", \"sys_time\": 7994, \"sys_memory\": 62758912, \"sys_memfree\": 10620928, \"work_mode\": 0, \"lan_ip\": \"192.168.2.1\", \"wan_type\": 1, \"wan_mac\": \"00:12:34:56:78:51\", \"wan_ip\": \"10.60.162.4\", \"lan_mac\": \"00:12:34:56:78:51\", \"kick_threshold\": -100, \"band_select_enable\": 1, \"total_users\": 0, \"radio24g\": { \"channel\": 0, \"turn_on\": 1, \"freq_bandwidth\": 1, \"wireless_mode\": 3, \"tx_power\": 100 }, \"radio5g\": { \"channel\": 0, \"turn_on\": 1, \"freq_bandwidth\": 2, \"wireless_mode\": 0, \"tx_power\": 100 }, \"ssids\": [ { \"ssid\": \"WIFI-aaa1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-2\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-3\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 } ], \"ssids_5g\": [ { \"ssid\": \"WIFI-5G\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-2\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-3\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 } ], \"qos\": { \"enable\": 0, \"upload_bw\": 128, \"download_bw\": 51200, \"sta_download\": 25600 }, \"sta_scan\": { \"enable\": 0, \"building_id\": \"ZK-PROBE-001002\", \"group_id\": 5, \"interval\": 5, \"server_ip\": \"120.76.116.73\", \"server_port\": 8100 }, \"mac_white_black\": { \"mac_black_list_md5\": \"0\", \"mac_white_list_md5\": \"0\" } }#$" "http://10.60.162.1:8090/wacs-mana/status.ac" &
	curl -d "ZK={ \"gw_id\": \"001234567851\", \"vendor_id\": \"ZK\", \"device_type\": \"TCA82\", \"product_type\": \"TCA82-B\", \"fm_name\": \"ZK-TCA82-B-4.3.0.4090-1806-stable\", \"soft_ver\": \"4.3.0.4090\", \"sys_load\": 10, \"status_interval\": 2, \"station_agingtime\": 10, \"timer_reboot\": 0, \"ipaddr_release\": 5, \"auth_server\": \"wifitest.zkgcloud.com\", \"auth_path\": \"\/wacs\/\", \"auth_port\": 8080, \"ac_server\": \"115.29.236.125\", \"ac_path\": \"\/wacs-mana\/\", \"ac_port\": 8090, \"ac_bak_hostname\": \"auth.zkgcloud.com\", \"ac_bak_path\": \"\/wacs-mana\/\", \"ac_bak_port\": 8090, \"ac_bak_sync_interval\": 30, \"acdisc_enable\": 1, \"auth_enable\": 1, \"portal_proxy\": 1, \"apple_auto_portal\": 1, \"escape_mode_enable\": 0, \"auth_protocol\": \"zk-portal\", \"sys_time\": 7994, \"sys_memory\": 62758912, \"sys_memfree\": 10620928, \"work_mode\": 0, \"lan_ip\": \"192.168.2.1\", \"wan_type\": 1, \"wan_mac\": \"00:12:34:56:78:51\", \"wan_ip\": \"10.60.162.4\", \"lan_mac\": \"00:12:34:56:78:51\", \"kick_threshold\": -100, \"band_select_enable\": 1, \"total_users\": 0, \"radio24g\": { \"channel\": 0, \"turn_on\": 1, \"freq_bandwidth\": 1, \"wireless_mode\": 3, \"tx_power\": 100 }, \"radio5g\": { \"channel\": 0, \"turn_on\": 1, \"freq_bandwidth\": 2, \"wireless_mode\": 0, \"tx_power\": 100 }, \"ssids\": [ { \"ssid\": \"WIFI-aaa1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-2\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-3\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 } ], \"ssids_5g\": [ { \"ssid\": \"WIFI-5G\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-2\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-3\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 } ], \"qos\": { \"enable\": 0, \"upload_bw\": 128, \"download_bw\": 51200, \"sta_download\": 25600 }, \"sta_scan\": { \"enable\": 0, \"building_id\": \"ZK-PROBE-001002\", \"group_id\": 5, \"interval\": 5, \"server_ip\": \"120.76.116.73\", \"server_port\": 8100 }, \"mac_white_black\": { \"mac_black_list_md5\": \"0\", \"mac_white_list_md5\": \"0\" } }#$" "http://10.60.162.1:8090/wacs-mana/status.ac" &
	curl -d "ZK={ \"gw_id\": \"001234567851\", \"vendor_id\": \"ZK\", \"device_type\": \"TCA82\", \"product_type\": \"TCA82-B\", \"fm_name\": \"ZK-TCA82-B-4.3.0.4090-1806-stable\", \"soft_ver\": \"4.3.0.4090\", \"sys_load\": 10, \"status_interval\": 2, \"station_agingtime\": 10, \"timer_reboot\": 0, \"ipaddr_release\": 5, \"auth_server\": \"wifitest.zkgcloud.com\", \"auth_path\": \"\/wacs\/\", \"auth_port\": 8080, \"ac_server\": \"115.29.236.125\", \"ac_path\": \"\/wacs-mana\/\", \"ac_port\": 8090, \"ac_bak_hostname\": \"auth.zkgcloud.com\", \"ac_bak_path\": \"\/wacs-mana\/\", \"ac_bak_port\": 8090, \"ac_bak_sync_interval\": 30, \"acdisc_enable\": 1, \"auth_enable\": 1, \"portal_proxy\": 1, \"apple_auto_portal\": 1, \"escape_mode_enable\": 0, \"auth_protocol\": \"zk-portal\", \"sys_time\": 7994, \"sys_memory\": 62758912, \"sys_memfree\": 10620928, \"work_mode\": 0, \"lan_ip\": \"192.168.2.1\", \"wan_type\": 1, \"wan_mac\": \"00:12:34:56:78:51\", \"wan_ip\": \"10.60.162.4\", \"lan_mac\": \"00:12:34:56:78:51\", \"kick_threshold\": -100, \"band_select_enable\": 1, \"total_users\": 0, \"radio24g\": { \"channel\": 0, \"turn_on\": 1, \"freq_bandwidth\": 1, \"wireless_mode\": 3, \"tx_power\": 100 }, \"radio5g\": { \"channel\": 0, \"turn_on\": 1, \"freq_bandwidth\": 2, \"wireless_mode\": 0, \"tx_power\": 100 }, \"ssids\": [ { \"ssid\": \"WIFI-aaa1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-2\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-3\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 } ], \"ssids_5g\": [ { \"ssid\": \"WIFI-5G\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-2\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-3\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 } ], \"qos\": { \"enable\": 0, \"upload_bw\": 128, \"download_bw\": 51200, \"sta_download\": 25600 }, \"sta_scan\": { \"enable\": 0, \"building_id\": \"ZK-PROBE-001002\", \"group_id\": 5, \"interval\": 5, \"server_ip\": \"120.76.116.73\", \"server_port\": 8100 }, \"mac_white_black\": { \"mac_black_list_md5\": \"0\", \"mac_white_list_md5\": \"0\" } }#$" "http://10.60.162.1:8090/wacs-mana/status.ac" &
	curl -d "ZK={ \"gw_id\": \"001234567851\", \"vendor_id\": \"ZK\", \"device_type\": \"TCA82\", \"product_type\": \"TCA82-B\", \"fm_name\": \"ZK-TCA82-B-4.3.0.4090-1806-stable\", \"soft_ver\": \"4.3.0.4090\", \"sys_load\": 10, \"status_interval\": 2, \"station_agingtime\": 10, \"timer_reboot\": 0, \"ipaddr_release\": 5, \"auth_server\": \"wifitest.zkgcloud.com\", \"auth_path\": \"\/wacs\/\", \"auth_port\": 8080, \"ac_server\": \"115.29.236.125\", \"ac_path\": \"\/wacs-mana\/\", \"ac_port\": 8090, \"ac_bak_hostname\": \"auth.zkgcloud.com\", \"ac_bak_path\": \"\/wacs-mana\/\", \"ac_bak_port\": 8090, \"ac_bak_sync_interval\": 30, \"acdisc_enable\": 1, \"auth_enable\": 1, \"portal_proxy\": 1, \"apple_auto_portal\": 1, \"escape_mode_enable\": 0, \"auth_protocol\": \"zk-portal\", \"sys_time\": 7994, \"sys_memory\": 62758912, \"sys_memfree\": 10620928, \"work_mode\": 0, \"lan_ip\": \"192.168.2.1\", \"wan_type\": 1, \"wan_mac\": \"00:12:34:56:78:51\", \"wan_ip\": \"10.60.162.4\", \"lan_mac\": \"00:12:34:56:78:51\", \"kick_threshold\": -100, \"band_select_enable\": 1, \"total_users\": 0, \"radio24g\": { \"channel\": 0, \"turn_on\": 1, \"freq_bandwidth\": 1, \"wireless_mode\": 3, \"tx_power\": 100 }, \"radio5g\": { \"channel\": 0, \"turn_on\": 1, \"freq_bandwidth\": 2, \"wireless_mode\": 0, \"tx_power\": 100 }, \"ssids\": [ { \"ssid\": \"WIFI-aaa1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-2\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-3\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 } ], \"ssids_5g\": [ { \"ssid\": \"WIFI-5G\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-2\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-3\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 } ], \"qos\": { \"enable\": 0, \"upload_bw\": 128, \"download_bw\": 51200, \"sta_download\": 25600 }, \"sta_scan\": { \"enable\": 0, \"building_id\": \"ZK-PROBE-001002\", \"group_id\": 5, \"interval\": 5, \"server_ip\": \"120.76.116.73\", \"server_port\": 8100 }, \"mac_white_black\": { \"mac_black_list_md5\": \"0\", \"mac_white_list_md5\": \"0\" } }#$" "http://10.60.162.1:8090/wacs-mana/status.ac" &
	curl -d "ZK={ \"gw_id\": \"001234567851\", \"vendor_id\": \"ZK\", \"device_type\": \"TCA82\", \"product_type\": \"TCA82-B\", \"fm_name\": \"ZK-TCA82-B-4.3.0.4090-1806-stable\", \"soft_ver\": \"4.3.0.4090\", \"sys_load\": 10, \"status_interval\": 2, \"station_agingtime\": 10, \"timer_reboot\": 0, \"ipaddr_release\": 5, \"auth_server\": \"wifitest.zkgcloud.com\", \"auth_path\": \"\/wacs\/\", \"auth_port\": 8080, \"ac_server\": \"115.29.236.125\", \"ac_path\": \"\/wacs-mana\/\", \"ac_port\": 8090, \"ac_bak_hostname\": \"auth.zkgcloud.com\", \"ac_bak_path\": \"\/wacs-mana\/\", \"ac_bak_port\": 8090, \"ac_bak_sync_interval\": 30, \"acdisc_enable\": 1, \"auth_enable\": 1, \"portal_proxy\": 1, \"apple_auto_portal\": 1, \"escape_mode_enable\": 0, \"auth_protocol\": \"zk-portal\", \"sys_time\": 7994, \"sys_memory\": 62758912, \"sys_memfree\": 10620928, \"work_mode\": 0, \"lan_ip\": \"192.168.2.1\", \"wan_type\": 1, \"wan_mac\": \"00:12:34:56:78:51\", \"wan_ip\": \"10.60.162.4\", \"lan_mac\": \"00:12:34:56:78:51\", \"kick_threshold\": -100, \"band_select_enable\": 1, \"total_users\": 0, \"radio24g\": { \"channel\": 0, \"turn_on\": 1, \"freq_bandwidth\": 1, \"wireless_mode\": 3, \"tx_power\": 100 }, \"radio5g\": { \"channel\": 0, \"turn_on\": 1, \"freq_bandwidth\": 2, \"wireless_mode\": 0, \"tx_power\": 100 }, \"ssids\": [ { \"ssid\": \"WIFI-aaa1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-2\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-3\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 } ], \"ssids_5g\": [ { \"ssid\": \"WIFI-5G\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-1\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-2\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 }, { \"ssid\": \"WIFI-5G-3\", \"vlanEnable\": 0, \"vlanId\": 0, \"bssid\": \"00:12:34:56:78:52\", \"auth_mode\": 0, \"encryption_type\": 0, \"password\": \"\", \"coding_type\": 0, \"hide_ssid\": 0, \"max_users\": -1, \"download_speed\": -1, \"upload_speed\": -1, \"station_count\": 0, \"station_list\": [ ], \"auth_enable\": 1 } ], \"qos\": { \"enable\": 0, \"upload_bw\": 128, \"download_bw\": 51200, \"sta_download\": 25600 }, \"sta_scan\": { \"enable\": 0, \"building_id\": \"ZK-PROBE-001002\", \"group_id\": 5, \"interval\": 5, \"server_ip\": \"120.76.116.73\", \"server_port\": 8100 }, \"mac_white_black\": { \"mac_black_list_md5\": \"0\", \"mac_white_list_md5\": \"0\" } }#$" "http://10.60.162.1:8090/wacs-mana/status.ac" &


	var parameters = {
            "basic": {
                "auth_enable": 0,
                "apple_auto_portal": 0,
                "work_mode": 0,
                "ipaddr_release": 5,
                "timer_reboot": 0,
                "auth_server": "auth.zkgcloud.com",
                "auth_port": 8080,
                "wifi_interference_detect": 1,
                "kick_threshold": -85,
                "band_select_enable": 0,
                "acdisc_enable": 1,
                "web_login_usr": "admin",
                "web_login_pwd": "admin",
                "status_interval": 2,
                "station_agingtime": 10,
                "inwall_vlan": 0,
                "escape_mode_enable": 0
            },
            "radio24g": {
                "turn_on": 1,
                "channel": 0,
                "freq_bandwidth": 1,
                "wireless_mode": 0,
                "tx_power": 20
            },
            "url_blacklist": [],
            "url_whitelist": [],
            "mac_blacklist": [],
            "qos": {
                "enable": 0,
                "upload_bw": 128,
                "download_bw": 51200
            },
            "ssids": [{
                "encryption_type": 1,
                "ssid": "WiFi",
                "max_users": -1,
                "download_speed": -1,
                "auth_mode": 1,
                "vlanId": 0,
                "password": "12345678",
                "coding_type": 0,
                "vlan_id": 1,
                "vlanEnable": 0,
                "auth_enable": "1",
                "hide_ssid": 0,
                "ssid_code": "WiFi",
                "upload_speed": -1
            }]
        }
done


ZK={ "gw_id": "001234567851", "vendor_id": "ZK", "device_type": "TCA82", "product_type": "TCA82-B", "fm_name": "ZK-TCA82-B-4.3.0.4090-1806-stable", "soft_ver": "4.3.0.4090", "sys_load": 10, "status_interval": 2, "station_agingtime": 10, "timer_reboot": 0, "ipaddr_release": 5, "auth_server": "wifitest.zkgcloud.com", "auth_path": "\/wacs\/", "auth_port": 8080, "ac_server": "115.29.236.125", "ac_path": "\/wacs-mana\/", "ac_port": 8090, "ac_bak_hostname": "auth.zkgcloud.com", "ac_bak_path": "\/wacs-mana\/", "ac_bak_port": 8090, "ac_bak_sync_interval": 30, "acdisc_enable": 1, "auth_enable": 1, "portal_proxy": 1, "apple_auto_portal": 1, "escape_mode_enable": 0, "auth_protocol": "zk-portal", "sys_time": 7994, "sys_memory": 62758912, "sys_memfree": 10620928, "work_mode": 0, "lan_ip": "192.168.2.1", "wan_type": 1, "wan_mac": "00:12:34:56:78:51", "wan_ip": "10.60.162.4", "lan_mac": "00:12:34:56:78:51", "kick_threshold": -100, "band_select_enable": 1, "total_users": 0, "radio24g": { "channel": 0, "turn_on": 1, "freq_bandwidth": 1, "wireless_mode": 3, "tx_power": 100 }, "radio5g": { "channel": 0, "turn_on": 1, "freq_bandwidth": 2, "wireless_mode": 0, "tx_power": 100 }, "ssids": [ { "ssid": "WIFI-aaa1", "vlanEnable": 0, "vlanId": 0, "bssid": "00:12:34:56:78:52", "auth_mode": 0, "encryption_type": 0, "password": "", "coding_type": 0, "hide_ssid": 0, "max_users": -1, "download_speed": -1, "upload_speed": -1, "station_count": 0, "station_list": [ ], "auth_enable": 1 }, { "ssid": "WIFI-1", "vlanEnable": 0, "vlanId": 0, "bssid": "00:12:34:56:78:52", "auth_mode": 0, "encryption_type": 0, "password": "", "coding_type": 0, "hide_ssid": 0, "max_users": -1, "download_speed": -1, "upload_speed": -1, "station_count": 0, "station_list": [ ], "auth_enable": 1 }, { "ssid": "WIFI-2", "vlanEnable": 0, "vlanId": 0, "bssid": "00:12:34:56:78:52", "auth_mode": 0, "encryption_type": 0, "password": "", "coding_type": 0, "hide_ssid": 0, "max_users": -1, "download_speed": -1, "upload_speed": -1, "station_count": 0, "station_list": [ ], "auth_enable": 1 }, { "ssid": "WIFI-3", "vlanEnable": 0, "vlanId": 0, "bssid": "00:12:34:56:78:52", "auth_mode": 0, "encryption_type": 0, "password": "", "coding_type": 0, "hide_ssid": 0, "max_users": -1, "download_speed": -1, "upload_speed": -1, "station_count": 0, "station_list": [ ], "auth_enable": 1 } ], "ssids_5g": [ { "ssid": "WIFI-5G", "vlanEnable": 0, "vlanId": 0, "bssid": "00:12:34:56:78:52", "auth_mode": 0, "encryption_type": 0, "password": "", "coding_type": 0, "hide_ssid": 0, "max_users": -1, "download_speed": -1, "upload_speed": -1, "station_count": 0, "station_list": [ ], "auth_enable": 1 }, { "ssid": "WIFI-5G-1", "vlanEnable": 0, "vlanId": 0, "bssid": "00:12:34:56:78:52", "auth_mode": 0, "encryption_type": 0, "password": "", "coding_type": 0, "hide_ssid": 0, "max_users": -1, "download_speed": -1, "upload_speed": -1, "station_count": 0, "station_list": [ ], "auth_enable": 1 }, { "ssid": "WIFI-5G-2", "vlanEnable": 0, "vlanId": 0, "bssid": "00:12:34:56:78:52", "auth_mode": 0, "encryption_type": 0, "password": "", "coding_type": 0, "hide_ssid": 0, "max_users": -1, "download_speed": -1, "upload_speed": -1, "station_count": 0, "station_list": [ ], "auth_enable": 1 }, { "ssid": "WIFI-5G-3", "vlanEnable": 0, "vlanId": 0, "bssid": "00:12:34:56:78:52", "auth_mode": 0, "encryption_type": 0, "password": "", "coding_type": 0, "hide_ssid": 0, "max_users": -1, "download_speed": -1, "upload_speed": -1, "station_count": 0, "station_list": [ ], "auth_enable": 1 } ], "qos": { "enable": 0, "upload_bw": 128, "download_bw": 51200, "sta_download": 25600 }, "sta_scan": { "enable": 0, "building_id": "ZK-PROBE-001002", "group_id": 5, "interval": 5, "server_ip": "120.76.116.73", "server_port": 8100 }, "mac_white_black": { "mac_black_list_md5": "0", "mac_white_list_md5": "0" } }#$