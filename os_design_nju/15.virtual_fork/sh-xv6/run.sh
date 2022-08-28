#############################################################
#   File Name: run.sh
#     Autohor: Hui Chen (c) 2022
#        Mail: chenhui13@xxx.com
# Create Time: 2022/08/28-09:34:57
#############################################################
#!/bin/sh 
gcc -c -ffreestanding sh-xv6.c -g -O2
ld sh-xv6.o -o mysh
