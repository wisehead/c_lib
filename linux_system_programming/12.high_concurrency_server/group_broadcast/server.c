#include <stdio.h>
#include <netinet/in.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>

#define SERVER_PORT 8000
#define CLIENT_PORT 9000

#define MAXLINE 4096

#define GROUP "239.0.0.2"

int main (void)
{
	int sockfd,i;
	struct sockaddr_in serveraddr,clientaddr;
	char buf[MAXLINE];
	char ipstr[INET_ADDRSTRLEN];
	socklen_t clientlen;
	ssize_t len;
	struct ip_mreqn group;
	//create a socket
	sockfd=socket(AF_INET,SOCK_DGRAM,0);

	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;  //ipv4
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);    //local any ip
	serveraddr.sin_port=htons(SERVER_PORT);
	//bind
	bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));

	/*
	   int setsockopt(int sockfd, int level, int optname,                
	   const void *optval, socklen_t optlen);
	   */

	/* group address*/
	inet_pton(AF_INET, GROUP, &group.imr_multiaddr);
	/* local address*/
	inet_pton(AF_INET, "0.0.0.0", &group.imr_address);
	/*eth0 --> index, command: ip ad*/
	group.imr_ifindex = if_nametoindex("eth0");

	setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &group, sizeof(group));

	bzero(&clientaddr,sizeof(clientaddr));
	clientaddr.sin_family=AF_INET;  //ipv4
	/*construct client address ip+port*/
	inet_pton(AF_INET, GROUP, &clientaddr.sin_addr.s_addr);
	clientaddr.sin_port = htons(CLIENT_PORT);

	while(1){
		fgets(buf, sizeof(buf), stdin);
		printf("buf:%s\n", buf);
		sendto(sockfd,buf,strlen(buf),0,(struct sockaddr*)&clientaddr,sizeof(clientaddr));
	}

	close(sockfd);

	return 0;

}
