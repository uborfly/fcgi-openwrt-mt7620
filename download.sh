#! /bin/ash
###
 # @Author       : Kexiang Zhang
 # @Date         : 2020-12-28 10:08:20
 # @LastEditors  : Kexiang Zhang
 # @LastEditTime : 2021-01-05 17:26:40
 # @FilePath     : /fcgi-openwrt-mt7620/download.sh
 # @Description  :
###
tftp 192.168.1.103 -g -r libfcgi.so.0
tftp 192.168.1.103 -g -r libfcgi.so.0.0.0
tftp 192.168.1.103 -g -r libjson-c.so.2
tftp 192.168.1.103 -g -r libjson-c.so.2.0.2
tftp 192.168.1.103 -g -r libblkid.so.1.1.0

mv lib* /usr/lib

tftp 192.168.1.103 -g -r nginx.conf
mv nginx.conf /etc/nginx/

tftp 192.168.1.103 -g -r wireless
mv wireless /etc/config/

tftp 192.168.1.103 -g -r network
mv network /etc/config/

tftp 192.168.1.103 -g -r vsftpd.conf
mv vsftpd.conf /etc/

tftp 192.168.1.103 -g -r start.sh
chmod 777 start.sh

clear
kill -9 $(ps | grep /root/upload | grep -v grep | awk '{print $1}')
tftp 192.168.1.103 -g -r upload
chmod 777 upload
spawn-fcgi -a 127.0.0.1 -p 9002 -f /root/upload
