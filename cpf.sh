#!/bin/bash
###
 # @Author       : Kexiang Zhang
 # @Date         : 2020-12-28 10:12:34
 # @LastEditors  : Kexiang Zhang
 # @LastEditTime : 2020-12-28 14:22:33
 # @FilePath     : /fcgi-openwrt-mt7620/cpf.sh
 # @Description  :
###
set -e
clear
make clean
make
cp build/bin/upload /mnt/d/tftp/
cp download.sh /mnt/d/tftp/
cp start.sh /mnt/d/tftp/

echo "Build OK"
# service nginx start
# spawn-fcgi -a 127.0.0.1 -p 9002 -f ./upload -n
