#############################################################
#   File Name: run.sh
#     Autohor: Hui Chen (c) 2022
#        Mail: chenhui13@xxx.com
# Create Time: 2022/08/22-09:19:10
#############################################################
#!/bin/sh 
gcc main.c
pmap 24705 >pmap.txt
cat /proc/24705/maps >proc.txt
