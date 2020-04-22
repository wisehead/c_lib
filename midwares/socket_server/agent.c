/*******************************************************************************
 *      File Name: agent.c                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 2016/09/30-16:03                                    
 *	Modified Time: 2016/09/30-16:03                                    
 *******************************************************************************/
#include <stdio.h>
#include <netinet/in.h>    // for sockaddr_in  
#include <sys/types.h>    // for socket  
#include <sys/socket.h>    // for socket  
#include <stdio.h>        // for printf  
#include <stdlib.h>        // for exit  
#include <string.h>        // for bzero  
#include <unistd.h>  
#include <fcntl.h>
#include <errno.h>
#include <signal.h>       // for signal
#include <time.h>
#include "ids_config.h"         //for IDS server ip config 
#include "ids_log.h"            //for IDS logger

#define HELLO_WORLD_SERVER_PORT    6666   
#define LENGTH_OF_LISTEN_QUEUE 20  
#define BUFFER_SIZE 1024  
#define FILE_NAME_MAX_SIZE 512  
#define MAX_WAITING_COUNT 86400 
#define IDS_SERVER_NUM 4
#define IP_ADDR_LEN 16
#define RECONNECT_TIME 30
#define MAX_RETRY_COUNT 30
#define MAX_PREFERRED_RETRY_COUNT 10

/*
char g_ids_ip_tables[IDS_SERVER_NUM][IP_ADDR_LEN] = 
        {"10.46.206.59",  //cq01-dba-davinci-srv33.cq01
         "10.26.18.30",   //tc-dba-davinci-srv49.tc
         "10.26.18.31"    //tc-dba-davinci-srv50.tc
        };
*/
int g_new_server_socket;
int g_fp;
int g_reconnect_flag = 0;
char g_date[100];
int g_retry_count = 0;

// signal handler function
static void node_exit(int32_t arg)
{
    log_msg(LEVEL_INFO, "SIGNAL is %d.\n", arg);
    if (arg == SIGPIPE)
    {
        g_reconnect_flag = 1;
    }
    else
    {
        exit(1);
    }
}

  
int main(int argc, char **argv)  
{  
    signal(SIGTERM, node_exit);
    signal(SIGINT, node_exit);
    signal(SIGPIPE, node_exit);

    //init the log handle
    g_log_handle = log_init("./conf/ids_agent.log", 400000000);

    int input_num = 0;
    int input_flag = 0;
    if (argc == 2)
    {
        input_num = atoi(argv[1]);
        if (input_num != 1 && input_num != 2 && input_num != 3)
        {
            log_msg(LEVEL_INFO, "please input the IDS server num.\n");
            exit(1);
        }
        input_num--;
        input_flag = 1;
    }
    /* Load the configuration parameters for connection to mysql. */
    if (!load_connect_config())
    {
        exit(1);
    }

    struct timeval timeout = {15, 0};

    bzero(g_date, 100);  
    time_t rawtime;
    struct tm * timeinfo;
    char time_buf[128]; 

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    bzero(time_buf, 128);   
    if (time_buf && timeinfo)
    {
        strftime(time_buf, sizeof(time_buf), "%Y%m%d", timeinfo);
    }
    snprintf(g_date, 99, "%s", time_buf);  
    log_msg(LEVEL_INFO, "g_date is:%s\n", g_date); 

    srand(time(NULL));
    char last_ip[16];
    int first_flag = 1;
    char* ip_addr = NULL;

    struct sockaddr_in client_addr;  
    int client_socket = 0;  

    while(1)
    {
        bzero(&client_addr, sizeof(client_addr)); //把一段内存区的内容全部设置为0  
        client_addr.sin_family = AF_INET;    //internet协议族  
        client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址  
        client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口  
        //创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket  
        client_socket = socket(AF_INET, SOCK_STREAM, 0);  
        if (client_socket < 0)  
        {  
            log_msg(LEVEL_INFO, "Create Socket Failed!\n");  
            exit(1);  
        }  
        //set the timeout value
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, 
                (char *)&timeout, sizeof(struct timeval));
        //把客户机的socket和客户机的socket地址结构联系起来  
        if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))  
        {  
            log_msg(LEVEL_INFO, "Client Bind Port Failed!\n");   
            exit(1);  
        }  
      
        //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口  
        struct sockaddr_in server_addr;  
        bzero(&server_addr, sizeof(server_addr));  
        server_addr.sin_family = AF_INET;  
        //const char* ip_addr = "10.46.92.31";
        
        unsigned int rand_ids_num = 0;
        if (input_flag)
        {
            rand_ids_num = input_num;
            input_flag = 0;
            ip_addr = g_ids_servers[rand_ids_num];
        }
        else if (strcmp(g_preferred_ids_server, "") != 0 
                && (g_retry_count < MAX_PREFERRED_RETRY_COUNT))
        {
            ip_addr = g_preferred_ids_server; 
        }
        else
        {
            rand_ids_num = rand();
            log_msg(LEVEL_INFO, "rand_ids_num is:%u\n", rand_ids_num);
            //rand_ids_num %= IDS_SERVER_NUM;
            rand_ids_num %= g_ids_server_count;
            log_msg(LEVEL_INFO, "rand_ids_num is:%u\n", rand_ids_num);
            ip_addr = g_ids_servers[rand_ids_num];
        }
        
        //ip_addr = g_ids_servers[rand_ids_num];
        log_msg(LEVEL_INFO, "Connecting to IDS server:%s\n", ip_addr);
        if(inet_aton(ip_addr, &server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数  
        {  
            log_msg(LEVEL_INFO, "Server IP Address Error!\n");  
            exit(1);  
        }  
        //server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);  
        server_addr.sin_port = htons(g_ids_server_port); //chenhui 
        socklen_t server_addr_length = sizeof(server_addr);  
        //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接  
        if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)  
        {  
            log_msg(LEVEL_INFO, "Can Not Connect To %s!\n", ip_addr);  
            sleep(RECONNECT_TIME);
            if (g_retry_count++ < MAX_RETRY_COUNT)
            {
                continue;
            }
            exit(1);  
        }  

        g_retry_count = 0;
        socklen_t length = sizeof(client_addr);  
  
        char buffer[BUFFER_SIZE];  
        bzero(buffer, BUFFER_SIZE);  
        length = recv(client_socket, buffer, BUFFER_SIZE, 0);  
        if (length < 0)  
        {  
            log_msg(LEVEL_INFO, "Server Recieve Data Failed!\n");  
            break;  
            //goto Exit;
        }  
        char file_name[FILE_NAME_MAX_SIZE + 1];  
        bzero(file_name, FILE_NAME_MAX_SIZE + 1);  
        snprintf(file_name, FILE_NAME_MAX_SIZE, "%s", buffer);
        log_msg(LEVEL_INFO, "%s\n", file_name);  

        int a = 0;
        int b = 0;
        int c = 0;
        long int t = 0;
        int i = 0;
        int nsl = 0;
        char buf[BUFFER_SIZE];
        int rc = 0;

        a = open(file_name, O_RDONLY);
        if (a == 0 || errno != 0)
        {
            log_msg(LEVEL_INFO, "open errno is:%d\n", errno);
            //B_chenhui
            if (client_socket)
            {
                close(client_socket);  
            }
            sleep(60);
            errno = 0;
            continue;
            //break;
            //E_chenhui
            //goto Exit;
        }
        g_fp = a;

        rc = lseek(a, 0, SEEK_SET);  
        if (rc == -1)
        {
            log_msg(LEVEL_INFO, "lseek errno is:%d\n", errno);
            if (errno == 9)
            {
                if (client_socket)
                {
                    close(client_socket);  
                }
                close(a);
                errno = 0;
                continue;
            }
            break;
            //goto Exit;
        }

        while (1)
        {
            if (g_reconnect_flag)
            {
                //close(a);
                log_msg(LEVEL_INFO, "g_reconnect_flag is set.%d\n", __LINE__);
                g_reconnect_flag = 0;
                break;
            }
            //c = read(a, buf, 3);
            c = read(a, buf, BUFFER_SIZE);
            if (c == -1)
            {
                log_msg(LEVEL_INFO, "read errno is:%d\n", errno);
                break;
            }
            //log_msg(LEVEL_INFO, "read bytes: %d\n", c);

            t += c; 
            if (c == 0) 
            {
                nsl++;  
                if (nsl > MAX_WAITING_COUNT)    
                {
                    break;  
                }
                if (nsl % 6 == 0)
                {
                    time_t rawtime;
                    struct tm * timeinfo;
                    char time_buf[128];
                    char temp_date[128];
                        
                    time(&rawtime); 
                    timeinfo = localtime(&rawtime);
                        
                    bzero(time_buf, 128);  
                    strftime(time_buf, sizeof(time_buf), "%Y%m%d", timeinfo);

                    // check the date every 60 seconds, to see whether we need the log rotation.
                    if (strncmp(g_date, time_buf, strlen(g_date)) != 0)
                    {
                        snprintf(g_date, 99, "%s", time_buf);
                        log_msg(LEVEL_INFO, 
                                "The End of Today, waiting for downloading the new log.\n");
                        log_msg(LEVEL_INFO, "old_date is:%s\n", g_date);
                        log_msg(LEVEL_INFO, "current date is:%s\n", time_buf);
                        //break;
                        // exit the ids_agent process, supervisor will restart the process
                        close(a);
                        //close the socket for listening
                        log_msg(LEVEL_INFO, "closing server socket.\n");
                        if (client_socket)
                        {
                            close(client_socket);  
                        }
                        return 0;  
                    }
                }
                log_msg(LEVEL_INFO, "sleeping 10 seconds.\n");
                sleep(10);  
                if (g_reconnect_flag)
                {
                    log_msg(LEVEL_INFO, "g_reconnect_flag is set.%d\n", __LINE__);
                    g_reconnect_flag = 0;
                    break;
                }
                close(a);
                log_msg(LEVEL_INFO, "lseek offset bytes: %ld\n", t);
                a = open(file_name, O_RDONLY);    
                if (a == 0 || errno != 0)
                {
                    log_msg(LEVEL_INFO, "open errno is:%d\n", errno);
                    break;
                }
                g_fp = a;
                rc = lseek(a, t, SEEK_SET);  
                if (rc == -1)
                {
                    log_msg(LEVEL_INFO, "lseek errno is:%ld\n", errno);
                    break;
                }
                if (g_reconnect_flag)
                {
                    //close(a);
                    log_msg(LEVEL_INFO, "g_reconnect_flag is set.%d\n", __LINE__);
                    g_reconnect_flag = 0;
                    break;
                }
            }
            else    
            {
                if (g_reconnect_flag)
                {
                    log_msg(LEVEL_INFO, "g_reconnect_flag is set.%d\n", __LINE__);
                    g_reconnect_flag = 0;
                    break;
                }

                if (send(client_socket, buf, c, 0) < 0)  
                {  
                    if (errno != 0)
                    {
                        log_msg(LEVEL_INFO, "send errno is:%d\n", errno);
                        errno = 0;
                    }
                    log_msg(LEVEL_INFO, "Send File:\t%s Failed\n", file_name);  

                    if (g_reconnect_flag)
                    {
                        log_msg(LEVEL_INFO, "g_reconnect_flag is set.%d\n", __LINE__);
                        g_reconnect_flag = 0;
                    }
                    break;  
                }  
                if (errno != 0)
                {
                    log_msg(LEVEL_INFO, "errno is:%d\n", errno);
                    errno = 0;
                }
                //bzero(buf, BUFFER_SIZE);  
            }
        }
        log_msg(LEVEL_INFO, "closing fp and socket.\n");
        close(a);
        close(client_socket);  
        sleep(RECONNECT_TIME);
        //goto retry;
    }  
Exit:
    //关闭监听用的socket  
    log_msg(LEVEL_INFO, "closing server socket.\n");
    if (client_socket)
    {
        close(client_socket);  
    }
    return 0;  
}  
