###
 # @Author       : Kexiang Zhang
 # @Date         : 2020-12-28 10:55:00
 # @LastEditors  : Kexiang Zhang
 # @LastEditTime : 2021-01-05 17:27:46
 # @FilePath     : /fcgi-openwrt-mt7620/start.sh
 # @Description  :
###
clear
kill -9 $(ps | grep /root/upload | grep -v grep | awk '{print $1}')
# tftp 192.168.1.103 -g -r upload
spawn-fcgi -a 127.0.0.1 -p 9002 -f /root/upload
/etc/init.d/nginx start
/etc/init.d/vsftpd start
