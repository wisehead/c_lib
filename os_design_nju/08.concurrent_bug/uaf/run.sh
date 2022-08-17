#############################################################
#   File Name: run.sh
#     Autohor: Hui Chen (c) 2022
#        Mail: alex.chenhui@gmail.com
# Create Time: 2022/08/17-20:24:32
#############################################################
#!/bin/sh 
gcc uaf.c -g -fsanitize=address
