#############################################################
#   File Name: run.sh
#     Autohor: Hui Chen (c) 2022
#        Mail: alex.chenhui@gmail.com
# Create Time: 2022/08/17-20:41:38
#############################################################
#!/bin/sh 
gcc sum.c -g -lpthread -fsanitize=thread
