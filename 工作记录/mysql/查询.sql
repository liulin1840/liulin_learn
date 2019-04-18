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

repair tables ac_status

(3). 并发插入

MyISAM存储引擎有个系统变量 concurrent_insert，专门用来控制并发插入的行为，可以取 0 ， 1 ， 2。

0表示不允许并发插入，1表示表中间没有删除的行时可以在表末尾插入，2表示总是可以插入。

一般如果对并发要求比较高的情况下，可以设置为2，总是可以插入，然
后定期在数据库空闲时间对表进行optimize

CHECK TABLE t1;
