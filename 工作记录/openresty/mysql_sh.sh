#!/bin/bash

#判断是否是root用户
if [ $(id -u) != "0" ]; then
    echo " Not the root user! Try using sudo Command ! "
    exit 1
fi
#安装依赖包
yum -y groupinstall "Desktop Platform Development"
yum install  -y  gd-devel  libjpeg-devel  libpng-devel  libxml2-devel bzip2-devel libcurl-devel  libmcrypt  libmcrypt-devel  zlib-devel  net-snmp-devel openssl  cmake  ncurses-devel  git  gcc  libevent-devel

#自定义源码包名称
Package_Type=".tar.gz"
Package_Version="mysql-5.6.31"
Install_Package=$Package_Version$Package_Type
Setup_path="/root/"
#自定义用户和用户组
Group_Name="mysql"
User_Name="mysql"
#自定义RPM包
RPM="$User_Name"
#自定义MySql数据库保存路径
#####Mydata="/mydata/{data,bin-log}}"
#mkdir /mydata
#mkdir /mydata/data
#mkdir /mydata/bin-log
mkdir /mydata
mkdir /mydata/{data,bin-log}
DataDir="/mydata/data"
#自定义MySql配置文件路径
MySql_Conf="/etc/mysql"
#自定义MySql安装路径
Install_prefix="/usr/local/mysql"
#判断Rpm包是否安装
function RPM_Install(){
rpm -qa | egrep "$RPM" >>/dev/null
	if [ $? -eq 0 ]
	then
		echo "$RPM is install Yes."
	else 
		echo "$RPM is Not install."
	fi
}
RPM_Install
#判断用户和用户组
Add_Group_User(){
egrep "^$Group_Name" /etc/group >& /dev/null
if [ $? -ne 0 ]
then
    echo "mysql 用户组正在添加."
    groupadd $Group_Name
else
    echo " The $Group_Name user group already exists."
    echo "mysql 用户组已经添加."
fi
#判断mysql用户是否存在
egrep "^$User_Name" /etc/passwd >& /dev/null
if [ $? -ne 0 ]
then
    echo "mysql 用户正在添加."
    useradd -g $Group_Name $User_Name
else
    echo "mysql 用户已经添加."
    echo " The $User_Name user already exists."
fi
}
Add_Group_User
#判断源码包文件是否存在
if [ -e $Package_Name ]
then
        echo "$Package_Name The Package exists."
else
        echo "$Package_Name The package does not exist."
fi

#编译安装mysql
cd $Setup_path
tar -zxvf $Install_Package
cd $Package_Version
configure_opts=(
 -DCMAKE_INTALL_PREFIX=$Install_prefix
 -DSYSCONFDIR=$MySql_Conf 
 -DMYSQL_DATADIR=$DataDir
 -DWITH_INNOBASE_STORAGE_ENGINE=1 
 -DWITH_ARCHIVE_STORAAGE_ENGINE=1 
 -DWITH_READLINE=1 
 -DWITH_OPENSSL=system 
 -DWITH_ZLIB=system 
 -DWITH_LIBWRAp=0 
 -DMYSQL_UNIX_ADDR=/tmp/mysql.sock  
 -DDEFAULT_CHARSET=utf8
 -DDEFAULT_COLLATION=utf8_general_ci
)
cmake . ${configure_opts[@]}
#判断是否编译成功
if [[ $? -eq 0 ]]
then
	make && make install
else
	echo "编译失败，请重新编译" && exit 1
fi
#修改mysql组和数据目录
cd $Install_prefix
chown :mysql ./* -R
chown -R mysql:mysql $DataDir
chown -R mysql:mysql /mydata/bin-log
#初始化数据库
$Install_prefix/scripts/mysql_install_db --user=mysql --datadir=$DataDir
#判断是否初始化成功，目录不为空说明成功，目录为空则失败
if [ "$(ls -A $DataDir)" ]; then
echo "$DataDir is not Empty"
else
echo "$DataDir is Empty"
fi
#提供服务脚本
cp $Install_prefix/support-files/mysql.server /etc/rc.d/init.d/mysqld
chmod +x /etc/rc.d/init.d/mysqld
chkconfig --add mysqld
#提供配置文件
#mv /etc/my.cnf /etc/my.cnf.bak
#mv /etc/my.cnf.d /etc/my.cnf.d.bak
if [ -e /etc/my.cnf ]
then
	echo "File exists."
	mv /etc/my.cnf /etc/my.cnf.bak
else
	echo "File does not exist."
fi

#if [ -e /etc/my.cnf.d ]
#then
#        echo "File exists."
#        mv /etc/my.cnf.d /etc/my.cnf.d.bak
#else
#        echo "File does not exist."
#fi
mkdir $MySql_Conf
cp $Install_prefix/support-files/my-default.cnf  /etc/mysql/my.cnf
sed -i '/datadir/a\datadir = /mydata/data' /etc/mysql/my.cnf
sed -i '/log_bin/a\log_bin = /mydata/bin-log/mysql-bin' /etc/mysql/my.cnf
sed -i '$ a\innodb_file_per_table = 1' /etc/mysql/my.cnf
#修改环境变量
cat >/etc/profile.d/mysql.sh << EOF
export PATH=/usr/local/mysql/bin/:$PATH
EOF
source /etc/profile.d/mysql.sh
source /etc/profile
man -M /usr/local/mysql/man/
#启动MySql服务
#Centos6启动方式
#service mysqld start
#Centos7启动方式
#systemctl start mysqld.service
/etc/rc.d/init.d/mysqld start
#判断MySql是否启动
MySQL_running() {
Startup_file="/etc/rc.d/init.d/mysqld"
#通过查看进程判断是否正常启动
proc=`ps -ef | grep mysql | grep -v grep| wc -l`
#通过监听3306端口进行判断是否启动
port=`netstat -lntup|grep 3306|wc -l`
if [ $proc -eq 2 ] & [ $port -eq 1 ]
   then
        echo "MySQL is running."
		echo "`ss -tnlp | grep mysql`"
else
        echo "MySQL is no running."
#       $Startup_file start

fi
}