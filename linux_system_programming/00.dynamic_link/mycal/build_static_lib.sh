#############################################################
#   File Name: build_static_lib.sh
#     Autohor: Hui Chen (c) 2025
#        Mail: alex.chenhui@gmail.com
# Create Time: 2025/04/30-13:55:55
#############################################################
#!/bin/sh 
gcc -c add.c sub.c mul.c dive.c
ar rcs libmycal.a add.o sub.o mul.o dive.o
