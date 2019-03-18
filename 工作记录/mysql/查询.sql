# 查询MySQL中标的引擎
SELECT table_name, table_type, engine FROM information_schema.tables  WHERE table_schema = 'wacs_db' ORDER BY table_name DESC; 

#看mysql支持哪些存储引擎:
mysql> show engines;

#看你的mysql当前默认的存储引擎:
mysql> show variables like '%storage_engine%';


DROP TABLE IF EXISTS `nas_device`;
CREATE TABLE `nas_device` (
  `device_id` int(10) NOT NULL AUTO_INCREMENT COMMENT '设备id主键',
  `device_sn` varchar(64) NOT NULL,
  `group_id` int(10) NOT NULL,
  `ip` varchar(64) DEFAULT NULL,
  `mac` varchar(64) DEFAULT NULL,
  `dev_type` varchar(128) DEFAULT NULL,
  `alias_name` varchar(128) DEFAULT NULL,
  `acname` varchar(128) DEFAULT NULL,
  `ssid` varchar(128) DEFAULT NULL,
  `description` varchar(256) DEFAULT NULL,
  `status` int(10) DEFAULT NULL,
  `active_time` datetime default NULL comment '激活时间',
  `note_time` datetime default NULL comment '最后更新时间',
  PRIMARY KEY (`device_id`)
) AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
ALTER TABLE `nas_device` ENGINE =MyISAM;

#执行sql脚本
mysql -u root -p123456 wacs_db </etc/config/ac_templet.sql


