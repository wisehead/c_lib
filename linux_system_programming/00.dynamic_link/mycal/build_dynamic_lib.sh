#############################################################
#   File Name: build_dynamic_lib.sh
#     Autohor: Hui Chen (c) 2025
#        Mail: alex.chenhui@gmail.com
# Create Time: 2025/04/30-13:56:24
#############################################################
#!/bin/sh 
gcc -c -fPIC add.c sub.c mul.c dive.c
gcc -shared -Wl,-soname,libmycal.so.1 -o libmycal.so.1.10 add.o sub.o mul.o dive.o
