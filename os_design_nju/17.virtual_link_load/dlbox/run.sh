#############################################################
#   File Name: run.sh
#     Autohor: Hui Chen (c) 2022
#        Mail: chenhui13@xxx.com
# Create Time: 2022/08/29-14:43:42
#############################################################
#!/bin/sh 
gcc -g dlbox.c -o dlbox
./dlbox gcc libc.S
./dlbox gcc libhello.S 
./dlbox gcc main.S
./dlbox readdl libc.dl
./dlbox readdl main.dl
./dlbox readdl libhello.dl
./dlbox objdump *.dl >out.txt
./dlbox interp main.dl
