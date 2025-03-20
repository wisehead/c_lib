/*******************************************************************************
 *      File name: server.c                                               
 *         Author: Hui Chen. (c) 2022                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 22/09/20-9月:09                                    
 * Modified Time: 22/09/20-9月:09                                    
 *******************************************************************************/
/* server.c */

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include "wrap.h"

#define MAXLINE 80 
#define SERV_PORT 8000

int main(int argc, char *argv[]) {
	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];/* FD_SETSIZE 默认为 1024 */
	ssize_t n;
	fd_set rset, allset;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN]; /* #define INET_ADDRSTRLEN 16 */
	socklen_t cliaddr_len;
	struct sockaddr_in cliaddr, servaddr;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	/* 默认最大128 */
	Listen(listenfd, 20);
	/* 初始化 */
	maxfd = listenfd;
	maxi = -1; /* client[]的下标 */

	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1; /* 用-1初始化client[] */

	FD_ZERO(&allset);
	FD_SET(listenfd, &allset); /* 构造select监控文件描述符集 */

	for ( ; ;) {
		rset = allset;
		/* 每次循环时都从新设置select监控信号集 */ 
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);

		if (nready < 0)
			perr_exit("select error");

		if (FD_ISSET(listenfd, &rset)) { /* new client connection */
			cliaddr_len = sizeof(cliaddr);
			connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
			printf("received from %s at PORT %d\n",
					inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), 
					ntohs(cliaddr.sin_port));

			for (i = 0; i < FD_SETSIZE; i++) {
				if (client[i] < 0) {
					client[i] = connfd;
					/* 保存accept返回的文件描述符到client[]里 */
					break;
				}
			}

			/* 达到select能监控的文件个数上限 1024 */ 
			if (i == FD_SETSIZE) {
				fputs("too many clients\n", stderr);
				exit(1);
			}
			FD_SET(connfd, &allset);
			/* 添加一个新的文件描述符到监控信号集里 */ 
			if (connfd > maxfd)
				maxfd = connfd;
			/* select第一个参数需要 */ 
			if (i > maxi)
				maxi = i;
			if (--nready == 0) 
				continue;
			/*处理完的就绪文件描述符 */ 
		}

		for (i = 0; i <= maxi; i++) {
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {
					/* 当client关闭链接时，服务器端也关闭对应链接 */ 
					Close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} else { 
					int j;
					for (j = 0; j < n; j++) 
						buf[j] = toupper(buf[j]);
					Write(sockfd, buf, n);
				}
				if (--nready == 0) 
					break;
			} 
		}
	}
	close(listenfd);
	return 0;
}
