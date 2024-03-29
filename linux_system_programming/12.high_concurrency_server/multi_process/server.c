#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#define SERVER_PORT 8000
#define MAXLINE 4096 


int main(void)
{
	struct sockaddr_in serveraddr,clientaddr;
	int sockfd,addrlen,confd,len,i;
	char ipstr[128];
	char buf[MAXLINE];
	pid_t pid;
	//1.socket
	sockfd=socket(AF_INET,SOCK_STREAM,0);

	//2.bind
	bzero(&serveraddr,sizeof(serveraddr)); 
	//ipv4
	serveraddr.sin_family=AF_INET;
	//ipaddr
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	//port
	serveraddr.sin_port=htons(SERVER_PORT);
	bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));

	//3.listen
	listen(sockfd,128);

	//4.accept
	while(1){
		addrlen=sizeof(clientaddr);
		confd=accept(sockfd,(struct sockaddr *)&clientaddr,&addrlen);
		//printf client ipaddress & port
		inet_ntop(AF_INET,&clientaddr.sin_addr.s_addr,ipstr,sizeof(ipstr));
		printf("client ip %s\tport %d\n",
				inet_ntop(AF_INET,&clientaddr.sin_addr.s_addr,ipstr,sizeof(ipstr)),
				ntohs(clientaddr.sin_port));


		//multiprocess server
		pid=fork();
		if(pid==0){
			close(sockfd);
			while(1){
				len=read(confd,buf,sizeof(buf)); 
				i=0;
				while(i<len){
					buf[i]=toupper(buf[i]);
					i++;
				}
				printf("buf:%s, len:%d\n", buf, len);
				write(confd,buf,len);
				sleep(1);
			}
			close(confd);
			return 0;  
		}
		else if(pid>0){
			close(confd);

		}

		/*
		//interact with the client for data manipulation
		len=read(confd,buf,sizeof(buf)); 
		i=0;
		while(i<len){
			buf[i]=toupper(buf[i]);
			i++;
		}
		write(confd,buf,len);
		//5.process client requests
		close(confd);
		*/
	}
	close(sockfd);
	return 0;
}


