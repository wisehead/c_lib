#############################################################
#   File Name: run.sh
#     Autohor: Hui Chen (c) 2022
#        Mail: chenhui13@xxx.com
# Create Time: 2022/08/18-08:43:59
#############################################################
#!/bin/sh 
gcc -c minimal.S
ld minimal.o
objdump -d minimal.o
