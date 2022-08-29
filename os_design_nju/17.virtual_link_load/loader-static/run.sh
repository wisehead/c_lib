#############################################################
#   File Name: run.sh
#     Autohor: Hui Chen (c) 2022
#        Mail: chenhui13@xxx.com
# Create Time: 2022/08/29-10:14:00
#############################################################
#!/bin/sh 
gcc -static dfs-fork.c -o dfs
gcc -static minimal.S -c && ld -o minimal minimal.o
gcc -static env.c -o env
gcc loader-static.c -o loader
