//orphan process & init()


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(void)
{
  pid_t pid;
  int n=3;
  while(n--){
    pid=fork();
    if(pid==0)
      break;
  }
  if (pid==0){
    while(1){
      printf("child%d\n",getpid());
      sleep(3);
	  break;
    }
  }
  else if(pid>0){
    pid_t pid_c;
    while(1){
      //pid_c=wait(NULL);
	  pid_c = waitpid(0, NULL, WNOHANG);
	  //printf("parent:child:%d exits\n", pid_c);
	  printf("I am parent\n");
	  sleep(1);
      if(pid_c == -1)
        continue;
	  else
		  printf("Wait for child %d\n", pid_c);
    }
  }
  return 0;
}
