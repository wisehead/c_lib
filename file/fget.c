/*******************************************************************************
 *      File Name: fgets.c                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: chenhui13@baidu.com                                        
 *   Created Time: 2016/10/10-15:54                                    
 *	Modified Time: 2016/10/10-15:54                                    
 *******************************************************************************/
/*
#include <stdio.h>
#include <fcntl.h>
 
int main() 
{ 
    //char filename[] = "1.txt"; //文件名
    char filename[] = "c/davinci_srv.log.20161010"; //文件名
    FILE *fp; 
    char StrLine[1024];             //每行最大读取的字符数
    if((fp = fopen(filename,"r")) == NULL) //判断文件是否存在及可读
    { 
        printf("error!"); 
        return -1; 
    } 

    //fseek(fp, 10L, SEEK_SET);
    while (!feof(fp)) 
    { 
        fgets(StrLine,1024,fp);  //读取一行
        printf("%s\n", StrLine); //输出
    } 
    fclose(fp);                     //关闭文件
    return 0; 
}
*/
/*******************************************************************************
 *      File Name: reader.c                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: chenhui13@baidu.com                                        
 *   Created Time: 2016/10/10-15:28                                    
 *	Modified Time: 2016/10/10-15:28                                    
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

//#define MAX_WAITING_COUNT 86400
#define MAX_WAITING_COUNT 3
#define BUF_SIZE 1024
int main(int argc, char *argv[])
{
    FILE* a;
    FILE* b;
    int c;
    long int t = 0;
    int i;
    int nsl = 0;
    char buf[BUF_SIZE];
    int rc = 0;


    char   *line = (char*)malloc(BUF_SIZE* sizeof(char)); 
    size_t  len = 0;
    size_t  line_len = 0;
    ssize_t bytes_read;

    //const char* file_name = "c/davinci_srv.log.20161010";
    char file_name[BUF_SIZE];
    memset(file_name, 0, BUF_SIZE);
    if (argc != 2)
    {
        printf("to few arguments.\n");
    }
    strncpy(file_name, argv[1], strlen(argv[1]));

    a = fopen(file_name, "r");
    b = fopen("xxx.txt", "w");

    //lseek(a, 0, SEEK_SET);
    fseek(a, 0, SEEK_SET);
    while (1)
    {
        //c = read(a, buf, BUF_SIZE);
        fgets(buf, BUF_SIZE, a);
        c = strlen(buf);
        if(feof(a))
        {
            printf("reach the EOF!! c is:%d.\n",c);
            c = 0;
        }
        /*
        if ((c = getline(&line, &len, a)) == -1)  
        {
            printf("getline failed. errno: %d\n", errno);
            break; 
        }
        */

        //printf("read bytes: %d\n", c);

        t += c;
        //t += len;
        if (c == 0)
        {
            printf("fseek offset is:%ld\n",t);
            nsl++;
            if (nsl > MAX_WAITING_COUNT)
                break;
            sleep(10);
            fclose(a);
            a = fopen(file_name, "r");
            if (a == 0)
            {
                printf("open errno is:%d\n",errno);
                break;
            }
            rc = fseek(a, t, SEEK_SET);
            if (rc == -1)
            {
                printf("lseek errno is:%d\n",errno);
                break;
            }
        }
        else
        {
            //printf("write fseek offset is:%ld\n",t);
            rc = fwrite(buf, sizeof(char),c,b);
            //int write_length = fwrite(buffer,sizeof(char),length,fp);
            if (rc == -1)
            {
                printf("write errno is:%d\n",errno);
                break;
            }
        }
    }
    fclose(a);
    fclose(b);
}

