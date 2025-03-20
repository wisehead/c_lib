/*******************************************************************************
 *      File name: gethostbyname.c                                               
 *         Author: Hui Chen. (c) 2022                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 22/09/21-9月:09                                    
 * Modified Time: 22/09/21-9月:09                                    
 *******************************************************************************/
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

extern int h_errno;

int main(int argc, char *argv[]) {
	struct hostent *host;
	char str[128];
	host = gethostbyname(argv[1]);
	printf(" host->h_name:%s\n", host->h_name);

	while (*(host->h_aliases) != NULL)
		printf("host->h_aliase:%s\n", *host->h_aliases++);
	
	switch (host->h_addrtype) {
		case AF_INET:
			while (*(host->h_addr_list) != NULL)
				printf("host->h_addr_list:%s\n", inet_ntop(AF_INET, (*host->h_addr_list++), str, sizeof(str)));
			break;
		default:
			printf("unknown address type\n");
			break;
	}
	return 0;

}

