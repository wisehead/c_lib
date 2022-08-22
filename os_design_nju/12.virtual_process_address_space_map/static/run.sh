#############################################################
#   File Name: run.sh
#     Autohor: Hui Chen (c) 2022
#        Mail: chenhui13@xxx.com
# Create Time: 2022/08/22-09:12:25
#############################################################
#!/bin/sh 
gcc -static main.c
gdb a.out
--starti
--info inferiors
cat /proc/24143/maps >main.addressmap.txt
pmap 24143 >pmap.txt

readelf -l a.out
