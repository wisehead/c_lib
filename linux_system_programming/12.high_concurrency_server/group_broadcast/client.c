#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/if.h>

#define SERVER_PORT 8000
#define CLIENT_PORT 9000

#define MAXLINE 4096

#define GROUP "239.0.0.2"

int main(int argc,char *argv[])
{
	int confd,len;
	char ipstr[]="127.0.0.1";
	char buf[MAXLINE];
	struct sockaddr_in serveraddr, localaddr;
	struct ip_mreqn group;
	if (argc<2){
		printf("./client str\n");
		exit(1);
	}
	//1.create a socket
	confd=socket(AF_INET,SOCK_DGRAM,0);

	//2.initializes the server address
	bzero(&localaddr,sizeof(localaddr));
	localaddr.sin_family=AF_INET;
	//ipadress
	//127.0.0.1 doesn't work
	//inet_pton(AF_INET, "127.00.0.1", &localaddr.sin_addr.s_addr);
	inet_pton(AF_INET, "0.0.0.0", &localaddr.sin_addr.s_addr);
	localaddr.sin_port=htons(CLIENT_PORT);

	bind(confd, (struct sockaddr*)&localaddr, sizeof(localaddr));

	/* group address*/
	inet_pton(AF_INET, GROUP, &group.imr_multiaddr);
	/* local address*/
	//127.0.0.1 doesn't work
	//inet_pton(AF_INET, ipstr, &group.imr_address);
	inet_pton(AF_INET, "0.0.0.0", &group.imr_address);
	/*eth0 --> index, command: ip ad*/
	group.imr_ifindex = if_nametoindex("eth0");

	/*set client, add it to multi-cast group*/
	setsockopt(confd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group));

	while(1) {
		len=recvfrom(confd,buf,sizeof(buf),0,NULL,0);
		printf("xxxxx buf:%s\n", buf);
		write(STDOUT_FILENO,buf,len);
	}
	//5.close socket
	close(confd);
	return 0;

}
