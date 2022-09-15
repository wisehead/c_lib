/*******************************************************************************
 *      file name: envrion.c                                               
 *         author: hui chen. (c) 22                             
 *           mail: chenhui13@xxx.com                                        
 *   created time: 22/09/15-9月:09                                    
 * modified time: 22/09/15-9月:09                                    
 *******************************************************************************/
#include <stdio.h>
int main() {
	extern char **environ;
	int i;
	for (i = 0; environ[i] != NULL; i++) {
		printf("%s\n", environ[i]);
	}
	return 0;
}
