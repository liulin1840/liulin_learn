
sudo yum install yum-utils
sudo yum-config-manager --add-repo https://openresty.org/package/centos/openresty.repo
sudo yum install openresty
#如果你想安装命令行工具 resty
sudo yum install openresty-resty
#列出所有 openresty 仓库里头的软件包：
sudo yum --disablerepo="*" --enablerepo="openresty" list available