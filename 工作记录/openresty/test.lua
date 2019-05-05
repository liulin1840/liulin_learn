ab -n 5000 -c 100 127.0.0.1:8989/test_wac
ab -n 5000 -c 100 127.0.0.1:8989/test
nginx -p /root/work/ -c conf/nginx.conf -s reload
