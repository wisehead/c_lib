/*******************************************************************************
 *      File Name: multi_thread_server.c                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: chenhui13@baidu.com                                        
 *   Created Time: 2016/09/29-15:32                                    
 *	Modified Time: 2016/09/29-15:32                                    
 *******************************************************************************/
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <pthread.h>
#include <sys/errno.h>    // for errno
#include "queue_list.h"   // for Queue
#include <signal.h>       // for signal

#define HELLO_WORLD_SERVER_PORT    6666 
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE  16384 
//#define BUFFER_SIZE  1024 
#define THREAD_MAX    5
#define PING_PONG_LINE_COUNT 100000

Queue g_sql_queue;
static pthread_mutex_t queue_mutex;
static pthread_mutex_t flag_ping_mutex;
int is_running = 0; 
int debug_on = 0;
long unsigned int g_sql_count = 0;
const char* file_name_ping = "ping.txt";
const char* file_name_pong = "pong.txt";
int g_flag_ping = 1;//ping == 1 means, socket is writing ping, else writing pong.
int g_enqueue_done_ping = 1;
int g_enqueue_done_pong = 0;
int g_writing_done_ping = 0;
int g_writing_done_pong = 1;
unsigned long int g_writing_count = 0;
int g_socket_is_active = 0;


static void node_exit(int32_t arg)
{
    printf("SIGNAL is %d.\n", arg);
    exit(1);
}

void * talk_to_client(void *data)
{
    int retry_count = 0;
    printf("talk_to_client Worker start\n");
    int new_server_socket = (int)data;
    char buffer[BUFFER_SIZE];
    char* sql_ptr = NULL;
    bzero(buffer, BUFFER_SIZE);
    strcpy(buffer,"Hello,World! from server!\n");
    strcat(buffer,"\n"); //C�����ַ�������
    //unsigned long int g_writing_count = 0;
    //����buffer�е��ַ�����new_server_socket,ʵ���Ǹ��ͻ���
    send(new_server_socket,buffer,BUFFER_SIZE,0);
    //const char* file_name = "recv.txt";
    FILE* fp = NULL;
    int length = 0;
    int rs = 1;
    is_running = 1;
    g_socket_is_active = 1;

retry:
    pthread_mutex_lock(&flag_ping_mutex); 
    if (g_flag_ping == 1)
    {
        if (g_enqueue_done_pong)
        {
            fp = fopen(file_name_ping,"w");  
            if(NULL == fp)  
            {  
                printf("File:\t%s Not Found\n", file_name_ping);  
            }  
            printf("------init writing ping -----\n");
            printf("g_sql_count: %lu.\n ",g_sql_count);
        }
    }
    else
    {
        if (g_enqueue_done_ping)
        {
            fp = fopen(file_name_pong,"w");  
            if(NULL == fp)  
            {  
                printf("File:\t%s Not Found\n", file_name_pong);  
            }  
            printf("------init writing pong -----\n");
            printf("g_sql_count: %lu.\n ",g_sql_count);
        }
    }
    pthread_mutex_unlock(&flag_ping_mutex); 
    if (!fp)
    {
        usleep(100);
        goto retry;
    }

    while (is_running)
    {
        if (g_writing_count > PING_PONG_LINE_COUNT)
        {
            printf("FILE:%s, line:%d\n",__FILE__,__LINE__);
            pthread_mutex_lock(&flag_ping_mutex); 
            //g_writing_done = 1;
            //if (g_writing_done && g_enqueue_done)
            if (g_flag_ping == 1)
            {
                if (g_enqueue_done_pong)
                {
                    g_flag_ping = 0;
                    g_writing_done_ping = 1;
                    g_enqueue_done_pong = 0;
                    if (fp)
                    {
                        fclose(fp);
                    }
                    fp = fopen(file_name_pong,"w");  
                    if(NULL == fp)  
                    {  
                        printf("File:\t%s Not Found\n", file_name_pong);  
                    }  
                    printf("------ writing pong -----\n");
                    printf("g_sql_count: %lu.\n ",g_sql_count);
                }
            }
            else
            {
                if (g_enqueue_done_ping)
                {
                    g_writing_done_pong = 1;
                    g_flag_ping = 1;
                    g_enqueue_done_ping = 0;
                    if (fp)
                    {
                        fclose(fp);
                    }
                    fp = fopen(file_name_ping,"w");  
                    if(NULL == fp)  
                    {  
                        printf("File:\t%s Not Found\n", file_name_ping);  
                    }  
                    printf("------ writing ping -----\n");
                    printf("g_sql_count: %lu.\n ",g_sql_count);
                }
            }
            //g_enqueue_done = 0;
            pthread_mutex_unlock(&flag_ping_mutex); 
            g_writing_count = 0;
        }
        
        bzero(buffer,BUFFER_SIZE);
        while(rs)
        {
            g_writing_count++;
            //���տͻ��˷���������Ϣ��buffer��
            length = recv(new_server_socket,buffer,BUFFER_SIZE,0);
            //printf("length:%d.\n", length);
            if (debug_on)
                printf("FILE:%s, line:%d\n",__FILE__,__LINE__);
            if(length < 0)
            {
                printf("length < 0\n");
                if (debug_on)
                    printf("FILE:%s, line:%d\n",__FILE__,__LINE__);
                // �����Ƿ�������ģʽ,���Ե�errnoΪEAGAINʱ,��ʾ��ǰ�������������ݿɶ�
                // ������͵����Ǹô��¼��Ѵ���.
                if(errno == EAGAIN)
                {
                    if (debug_on)
                        printf("FILE:%s, line:%d\n",__FILE__,__LINE__);
                    break;
                }
                else
                { 
                    if (debug_on)
                        printf("FILE:%s, line:%d\n",__FILE__,__LINE__);
                    goto Exit;
                }
           }
           else if(length == 0)
           {
                printf("length == 0\n");
                if (debug_on)
                    printf("FILE:%s, line:%d\n",__FILE__,__LINE__);
               // �����ʾ�Զ˵�socket�������ر�.
                goto Exit;
           }
           else
           { 
               //printf("length > 0\n");
               if (debug_on)
                   printf("FILE:%s, line:%d\n",__FILE__,__LINE__);
               retry_count = 0;

               int write_length = fwrite(buffer,sizeof(char),length,fp);  
               if (write_length<length)  
               {  
                   printf("Write Failed. ping_pong_flag is:%d.\n", g_flag_ping);  
                   //printf("File:\t%s Write Failed\n", file_name);  
                   break;  
               }  
               bzero(buffer,BUFFER_SIZE);      
               if(length == sizeof(buffer))
               {
                   if (debug_on)
                        printf("FILE:%s, line:%d\n",__FILE__,__LINE__);
                    rs = 1;   // ��Ҫ�ٴζ�ȡ
               }
               else
               {
                    rs = 0;
                   if (debug_on)
                        printf("FILE:%s, line:%d\n",__FILE__,__LINE__);
               }
           }
       }
        //to detect whether the socket is closed.
        rs = 1;
    }
Exit:
    g_writing_count = 0;
    pthread_mutex_lock(&flag_ping_mutex); 
    g_socket_is_active = 0;
    if (g_flag_ping == 1)
    {
        g_writing_done_ping = 1;
        /*
        if (g_enqueue_done_pong)
        {
            g_flag_ping = 0;
            g_writing_done_ping = 1;
            g_enqueue_done_pong = 0;
            printf("------ g_flag_ping changed:%d.\n",g_flag_ping);
        }
        */
    }
    else
    {
        g_writing_done_pong = 1;
        /*
        if (g_enqueue_done_ping)
        {
            g_flag_ping = 1;
            g_writing_done_pong = 1;
            g_enqueue_done_ping = 0;
            printf("------ g_flag_ping changed:%d.\n",g_flag_ping);
        }
        */
    }
    //printf("g_sql_count: %lu.\n ",g_sql_count);
    //printf("FILE:%s, line:%d\n",__FILE__,__LINE__);

    pthread_mutex_unlock(&flag_ping_mutex); 
    //�ر���ͻ��˵�����
    printf("talk_to_client Worker exit\n");
    printf("g_sql_count: %lu.\n ",g_sql_count);
    fclose(fp); 
    close(new_server_socket); 
    pthread_exit(NULL);
}

void * enqueue_func(void)
{
    char buffer[BUFFER_SIZE];
    char* sql_ptr = NULL;
    printf("enqueue Worker start\n");

    FILE* fp = NULL;
    char   *line = (char*)malloc(BUFFER_SIZE* sizeof(char)); 
    size_t  len = 0;
    size_t  line_len = 0;
    ssize_t bytes_read;
    
    unsigned long int writing_count;
    is_running = 1;
    while(is_running)
    {
        pthread_mutex_lock(&flag_ping_mutex); 
        if (g_flag_ping == 1)
        {
            if (g_writing_done_pong)
            {
                fp = fopen(file_name_pong,"r");  
                if(NULL == fp)  
                {  
                    printf("File:\t%s Not Found\n", file_name_pong);  
                }  
                printf("------ reading pong -----\n");
                g_writing_done_pong = 0;
            }
            else if (!g_socket_is_active && g_writing_done_ping)
            {
                g_flag_ping = 0;
                g_writing_done_ping = 0;
                fp = fopen(file_name_ping,"r");  
                if(NULL == fp)  
                {  
                    printf("File:\t%s Not Found\n", file_name_ping);  
                }  
                printf("------ reading ping -----\n");
                
            }
            else
            {
                pthread_mutex_unlock(&flag_ping_mutex); 
                usleep(10);
                continue;
            }
        }
        else
        {
            if (g_writing_done_ping)
            {
                fp = fopen(file_name_ping,"r");  
                if(NULL == fp)  
                {  
                    printf("File:\t%s Not Found\n", file_name_ping);  
                }  
                printf("------ reading ping -----\n");
                g_writing_done_ping = 0;
            }
            else if (!g_socket_is_active && g_writing_done_pong)
            {
                g_flag_ping = 1;
                g_writing_done_pong = 0;
                fp = fopen(file_name_pong,"r");  
                if(NULL == fp)  
                {  
                    printf("File:\t%s Not Found\n", file_name_pong);  
                }  
                printf("------ reading pong -----\n");
                
            }
            else
            {
                pthread_mutex_unlock(&flag_ping_mutex); 
                usleep(10);
                continue;
            }
        }
        //g_writing_done = 0;
        pthread_mutex_unlock(&flag_ping_mutex); 

        bzero(buffer, BUFFER_SIZE);  
        int file_block_length = 0;  
        while( (bytes_read = getline(&line, &len, fp)) != -1)  
        {  
            //printf("file_block_length = %d\n",file_block_length);  
            //line_len = (len >= 8191)? 8191:len;
            line_len = (bytes_read >= (BUFFER_SIZE - 1))? (BUFFER_SIZE -1):bytes_read;
            if (line_len > 0)
            {
                sql_ptr = (char*)malloc(line_len+1);
                sql_ptr[line_len] = '\0';
                strncpy(sql_ptr, line, line_len);
                if (debug_on)
                    printf("sql_ptr is:%s\n",sql_ptr);
                pthread_mutex_lock(&queue_mutex); 
                Enqueue(sql_ptr , g_sql_queue);
                g_sql_count++;
                if (g_sql_count % 1000 == 0)
                    printf("g_sql_count: %lu.\n ",g_sql_count);
                queue_node_count++;
                //printf("queue_node_count: %d.\n ",queue_node_count);
                pthread_mutex_unlock(&queue_mutex); 
            }
            //printf("length is:%d, new line is:%s", line_len,buffer);  
            bzero(buffer, BUFFER_SIZE);  
        }  
        fclose(fp);
        pthread_mutex_lock(&flag_ping_mutex); 
        if (g_flag_ping)
            g_enqueue_done_pong = 1;
        else
            g_enqueue_done_ping = 1;
        printf("FILE:%s, line:%d\n",__FILE__,__LINE__);
        pthread_mutex_unlock(&flag_ping_mutex); 
        usleep(10);
    }
    printf("g_sql_count: %lu.\n ",g_sql_count);
    //fprintf(stderr,"Worker %lu exit\n",wa->worker_id);
    printf("enqueue Worker exit\n");
    pthread_exit(NULL);
}

void * dispatch_func(void)
{
    char* sql_ptr = NULL;
    printf("dispatch Worker start\n");
    is_running = 1;
    while(is_running)
    {
        while (queue_node_count > 0)
        {
            pthread_mutex_lock(&queue_mutex); 
            if (!IsEmpty(g_sql_queue))
            {
                sql_ptr = FrontAndDequeue(g_sql_queue);
                queue_node_count--;
                if (sql_ptr)
                {
                    if (debug_on)
                        printf("SQL is poped: %s", sql_ptr);
                    else if (queue_node_count % 1000 == 0 && queue_node_count != 0)
                        //printf("%s", sql_ptr);
                        printf("SQL is poped: queue_node_count:%d\n", queue_node_count);
                    //printf("queue_node_count: %d.\n ",queue_node_count);
                    free(sql_ptr);
                }
            }
            pthread_mutex_unlock(&queue_mutex); 
        }
        usleep(10);
    }
    //printf("g_sql_count: %lu.\n ",g_sql_count);
    printf("dispatch Worker exit\n");
    pthread_exit(NULL);
}
int main(int argc, char **argv)
{
    signal(SIGTERM, node_exit);
    signal(SIGINT, node_exit);
    queue_node_count = 0;
    g_sql_queue = CreateQueue();
    printf( "Initialization complete.\n" );  
    pthread_mutex_init(&queue_mutex,NULL);
    pthread_mutex_init(&flag_ping_mutex,NULL);
    //����һ��socket��ַ�ṹserver_addr,���������internet��ַ, �˿�
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr)); //��һ���ڴ���������ȫ������Ϊ0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

    //��������internet����Э��(TCP)socket,��server_socket���������socket
    int server_socket = socket(AF_INET,SOCK_STREAM,0);
    if( server_socket < 0)
    {
        printf("Create Socket Failed!");
        exit(1);
    }
    
    //��socket��socket��ַ�ṹ��ϵ����
    if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT); 
        exit(1);
    }
    
    //server_socket���ڼ���
    if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) )
    {
        printf("Server Listen Failed!"); 
        exit(1);
    }
    pthread_t dispatch_thread;
    pthread_attr_t dispatch_thread_attr;
    pthread_attr_init(&dispatch_thread_attr);
    pthread_attr_setdetachstate(&dispatch_thread_attr,PTHREAD_CREATE_DETACHED);
    if( pthread_create(&dispatch_thread,&dispatch_thread_attr, dispatch_func, NULL) < 0 )
        printf("pthread_create Failed : %s\n",strerror(errno));
    
    
    pthread_t enqueue_thread;
    pthread_attr_t enqueue_thread_attr;
    pthread_attr_init(&enqueue_thread_attr);
    pthread_attr_setdetachstate(&enqueue_thread_attr,PTHREAD_CREATE_DETACHED);
    if( pthread_create(&enqueue_thread,&enqueue_thread_attr, enqueue_func, NULL) < 0 )
        printf("pthread_create Failed : %s\n",strerror(errno));

    int i;
    while(1) //��������Ҫһֱ����
    {

        //����ͻ��˵�socket��ַ�ṹclient_addr
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);

        //����һ����server_socket�����socket��һ������
        //���û����������,�͵ȴ�������������--����accept����������
        //accept��������һ���µ�socket,���socket(new_server_socket)����ͬ���ӵ��Ŀͻ���ͨ��
        //new_server_socket�����˷������Ϳͻ���֮���һ��ͨ��ͨ��
        //accept���������ӵ��Ŀͻ�����Ϣ��д���ͻ��˵�socket��ַ�ṹclient_addr��
        int new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);
        if ( new_server_socket < 0)
        {
            printf("Server Accept Failed!\n");
            break;
        }
        pthread_t child_thread;
        pthread_attr_t child_thread_attr;
        pthread_attr_init(&child_thread_attr);
        pthread_attr_setdetachstate(&child_thread_attr,PTHREAD_CREATE_DETACHED);
        if( pthread_create(&child_thread,&child_thread_attr,talk_to_client, (void *)new_server_socket) < 0 )
            printf("pthread_create Failed : %s\n",strerror(errno));
    }
    //�رռ����õ�socket
    close(server_socket);
    return 0;
}
