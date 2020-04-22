#############################################################
#   File Name: build.sh
#     Autohor: Hui Chen (c) 2020
#        Mail: chenhui13@baidu.com
# Create Time: 2020/04/22-10:29:09
#############################################################
#!/bin/sh 
gcc -fPIC -shared -o libadd.so file.c
gcc -o demo demo.c -ldl
./demo
