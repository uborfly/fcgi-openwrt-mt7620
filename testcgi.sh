#!/bin/bash
clear
make clean
make
cp build/bin/upload /mnt/e/vmbox_share/
# service nginx start
# spawn-fcgi -a 127.0.0.1 -p 9002 -f ./upload -n
