#############################################################
#   File Name: build_static_app.sh
#     Autohor: Hui Chen (c) 2025
#        Mail: chenhui13@xxx.com
# Create Time: 2025/04/30-14:12:15
#############################################################
#!/bin/sh 
gcc main.c libmycal.a -static -o app2
