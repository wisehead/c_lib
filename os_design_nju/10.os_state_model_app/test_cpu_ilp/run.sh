#############################################################
#   File Name: run.sh
#     Autohor: Hui Chen (c) 2022
#        Mail: chenhui13@xxx.com
# Create Time: 2022/08/20-21:30:13
#############################################################
#!/bin/sh 
perf stat ./a.out
perf record ./a.out 1
perf report
