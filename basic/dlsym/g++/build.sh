#############################################################
#   File Name: build.sh
#     Autohor: Hui Chen (c) 2020
#        Mail: chenhui13@baidu.com
# Create Time: 2020/04/22-10:29:09
#############################################################
#!/bin/sh 
g++ -fPIC -shared -g -o libadd.so add.c
#> nm libadd.so
#0000000000000600 T _Z3addii
#> readelf -s libadd.so
#    11: 0000000000000600    20 FUNC    GLOBAL DEFAULT    9 _Z3addii
#change demo.c //FUNC_ADD add_func = (FUNC_ADD)dlsym( handle, "_Z3addii" );
g++ -g -o demo demo.c -ldl
./demo
