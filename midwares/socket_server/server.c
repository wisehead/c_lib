/*******************************************************************************
 *      File Name: server.c                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: alex.chenhui@gmail.com                                        
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
#include "ids_queue_list.h"   // for Queue
#include <signal.h>       // for signal
#include "ids_helper.h"   // for the helper functions 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "ids_log.h" 

#define HELLO_WORLD_SERVER_PORT    6666 
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE  16384 
#define DAVINCI_BUFFER_SIZE 81920//for davinci big SQLs, 80K 
#define DAVINCI_BUFFER_SIZE_BIG 1048576 //for davinci bigger than 81920 , 1MB. 
#define THREAD_MAX   11 
#define MAX_WAITING_COUNT 86400
#define BUF_SIZE 1024
#define MAX_QUEUE_SIZE 1000000 
#define IP_ADDR_LEN 16
#define FILE_NAME_MAX_SIZE 512 

typedef struct{
    int new_socket;
    int worker_id;
}Work_Area;

Work_Area g_wa_array[THREAD_MAX] = {0};
int g_wa_index = 0;
Queue g_sql_queue;
static pthread_mutex_t s_queue_mutex;
int g_is_running = 0; 
int g_debug_on = 0;
long unsigned int g_sql_count = 0;
char g_file_name[FILE_NAME_MAX_SIZE + 1];  
int g_rotate_flag = 0;
int g_rotate_array[THREAD_MAX] = {0};
int g_socket_active_array[THREAD_MAX] = {0};
int g_connected_agent_count = 0;
//log_st* g_log_handle;//for ids log

// get the offset of the ip tables, which is from ids_server.conf.
// if the ip isn't in the ip tables, then -1 is returned.
static int get_index_of_agent_ip_tables(const char* ip)
{
    if (ip == NULL)
    {
        return -1;
    }
    log_msg(LEVEL_DEBUG, "ip address passed in is:%s\n", ip);
    int index = 0;
    for (index = 0; index < g_davinci_server_count && index < THREAD_MAX; index++)
    {
        log_msg(LEVEL_INFO, "xxx davinci server[%d]:%s\n", index, g_davinci_servers[index]);
        if (strncmp(g_davinci_servers[index], ip, strlen(ip)) == 0)
        {
            return index;
        }
    }
    return -1;
}

// signal handler function.
static void node_exit(int32_t arg)
{
    log_msg(LEVEL_INFO, "SIGNAL is %d.\n", arg);
    exit(1);
}

char g_cmd_buffer[BUFFER_SIZE];  
int process_exception_insert()
{
    //char buffer[BUFFER_SIZE];  
    char* buffer = (char*)malloc(sizeof(char)*BUFFER_SIZE);
    if (buffer == NULL)
    {
        exit(1);
    }
    bzero(buffer, BUFFER_SIZE);  
    char* option_ptr = strstr(g_cmd_buffer, "OPTION:");
    if (option_ptr == NULL)
    {
        //snprintf(buffer, BUFFER_SIZE, "%s", cmd_ptr);
        log_msg(LEVEL_INFO, "option is NULL.\n");
        return -1;
    }
    snprintf(buffer, BUFFER_SIZE, "%s", option_ptr+7);
    log_msg(LEVEL_INFO, "exception SQL: %s\n", buffer);
    insert_exception(buffer);

    return 0;
}

int process_update_config()
{
    log_msg(LEVEL_INFO, "[IDS]process update config.\n");
    // Reload the config rules, the new level will take effect.
    sql_load_config();
    return 0;
}

void process_cmd(int console_socket)
{
    char buffer[BUFFER_SIZE];  
    bzero(buffer, BUFFER_SIZE);  
    // reveive the data from IDS Agent, and put into buffer
    bzero(g_cmd_buffer, BUFFER_SIZE);  
    int length = 0;  
    char cmd = 0;
    while (1)  
    {  
        length = recv(console_socket, g_cmd_buffer, BUFFER_SIZE, 0);
        if(length <= 0)  
        {  
            //log_msg(LEVEL_INFO, "Recieve Data From Server localhost Failed!\n");  
            break;  
        }  
        
        log_msg(LEVEL_INFO, " received command: %s\n", g_cmd_buffer);
        char* cmd_ptr = strstr(g_cmd_buffer, "COMMAND:-");
        //log_msg(LEVEL_INFO, " cmd_ptr: %s\n", cmd_ptr);
        if (cmd_ptr == NULL)
        {
            log_msg(LEVEL_INFO, "Wrong command!\n");
            break;
        }
        // point to the end of "COMMAND:"
        //cmd_ptr += 9; 
        //log_msg(LEVEL_INFO, "CMD is:%c\n", cmd_ptr);
        cmd = cmd_ptr[9];
        log_msg(LEVEL_INFO, "CMD is:%c\n", cmd);
        switch (cmd)
        {
            case 'I': 
                process_exception_insert();
                break;
            case 'U': 
                process_update_config();
                break;
            default:
                log_msg(LEVEL_INFO, "wrong cmd!\n");
                break;
        }

        bzero(buffer, BUFFER_SIZE);      
        bzero(g_cmd_buffer, BUFFER_SIZE);      
    }  
}

// a new thread is spawned if a new connection from client arrives.
// this thread is used for downloading davinci logs from ids agent constantly, 
// and writes the data to a local file.
void * talk_to_client(void *data)
{
    Work_Area* wa = (Work_Area*)data;
    int index = wa->worker_id;
    int new_server_socket = wa->new_socket;
    g_socket_active_array[index] = 1;
    if (index == THREAD_MAX - 1)
    {
        process_cmd(new_server_socket);
        g_socket_active_array[index] = 0;
        pthread_exit(NULL);
        return;
    }

    char ip_addr[128] = {0};
    bzero(ip_addr, 128);
    snprintf(ip_addr, 128, "%s", g_davinci_servers[index]);

    int retry_count = 0;
    log_msg(LEVEL_INFO, "talk_to_client Worker start\n");

    //add mutex for g_file_name
    //pthread_mutex_lock(&s_queue_mutex);
    log_msg(LEVEL_INFO, "downloading File: %s On Server...\n", g_file_name);  
    char buffer[BUFFER_SIZE];  
    bzero(buffer, BUFFER_SIZE);  
    snprintf(buffer, BUFFER_SIZE, "%s", g_file_name);  
    // send the file name of the davinci log to IDS Agent
    send(new_server_socket, buffer, BUFFER_SIZE, 0);  
  
    char file_name[FILE_NAME_MAX_SIZE+1];  
    bzero(file_name, FILE_NAME_MAX_SIZE+1);  
    snprintf(file_name, FILE_NAME_MAX_SIZE, "%s.%d", g_file_name, index);
    //pthread_mutex_unlock(&s_queue_mutex);
    FILE* fp = fopen(file_name, "w");  
    if (NULL == fp || errno != 0)
    {  
        log_msg(LEVEL_INFO, "File:\t%s Can Not Open To Write\n", file_name);  
        exit(1);  
    }  
      
    // reveive the data from IDS Agent, and put into buffer
    bzero(buffer, BUFFER_SIZE);  
    int length = 0;  
    while (1)  
    {  
        length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
        if(length <= 0)  
        {  
            log_msg(LEVEL_INFO, "errno is:%d!\n", errno);  
            // errno 11: Resource temporarily unavailable
            // errno 104: if the peer closes the socket,
            if (errno == 11 || errno == 104)
            {
                errno = 0;
                pthread_mutex_lock(&s_queue_mutex);

                log_msg(LEVEL_INFO, "Recieve Data From Server %s TIMEOUT!\n", ip_addr);  
                time_t rawtime;
                struct tm * timeinfo;
                char time_buf[128];
                char temp_file_name[128];
                    
                time(&rawtime); 
                timeinfo = localtime(&rawtime);
                    
                bzero(time_buf, 128);  
                strftime(time_buf, sizeof(time_buf), "%Y%m%d", timeinfo);
                bzero(temp_file_name, 128);  
                snprintf(temp_file_name, 127, "davinci_srv.log.%s.%d", time_buf, index);
                log_msg(LEVEL_INFO, "file_name is:%s\n", file_name);
                log_msg(LEVEL_INFO, "temp_file_name is:%s\n", temp_file_name);

                // the end of today, IDS server should donwload the new davinci log.
                //if (strncmp(g_file_name, temp_file_name, strlen(g_file_name)) != 0)
                if (strncmp(file_name, temp_file_name, strlen(file_name)) != 0)
                {
                    bzero(temp_file_name, 128);  
                    snprintf(temp_file_name, 127, "davinci_srv.log.%s", time_buf);
                    //g_rotate_flag = 1;
                    //g_rotate_array[index] = 1;
                    if (strncmp(g_file_name, temp_file_name, strlen(g_file_name)) != 0)
                    {
                        bzero(g_file_name, FILE_NAME_MAX_SIZE + 1);
                        snprintf(g_file_name, FILE_NAME_MAX_SIZE, 
                                "davinci_srv.log.%s", time_buf);
                    }
                    log_msg(LEVEL_INFO, 
                            "download num:%d: The End of Today, rorate the log.\n", index);
                    pthread_mutex_unlock(&s_queue_mutex);
                    break;
                }

                pthread_mutex_unlock(&s_queue_mutex);
                continue;
            }
            log_msg(LEVEL_INFO, "Recieve Data From Server %s Failed!\n", ip_addr);  
            break;  
        }  

        pthread_mutex_lock(&s_queue_mutex);
        int write_length = fwrite(buffer, sizeof(char), length, fp);  
        if (write_length<length)  
        {  
            log_msg(LEVEL_INFO, "File:\t%s Write Failed\n", file_name);  
            pthread_mutex_unlock(&s_queue_mutex);
            break;  
        }  
        fflush(fp);
        pthread_mutex_unlock(&s_queue_mutex);
        bzero(buffer, BUFFER_SIZE);      
    }  
    log_msg(LEVEL_INFO, "Recieve File:\t %s From Server[%s] Finished\n", g_file_name, ip_addr);  
      
Exit:
    // close the socket and fp.
    log_msg(LEVEL_INFO, "talk_to_client Worker %u exit\n", index);
    log_msg(LEVEL_INFO, "g_sql_count: %lu.\n", g_sql_count);
    fclose(fp); 
    close(new_server_socket); 
    g_socket_active_array[index] = 0;
    pthread_mutex_lock(&s_queue_mutex);
    if (g_connected_agent_count > 0)
    {
        g_connected_agent_count--;
    }
    pthread_mutex_unlock(&s_queue_mutex);
    pthread_exit(NULL);
}

// a new enque thread is spawned if a new connection from IDS agent arrives.
// it will read the davinci log record from local files, which are created by 
// talk_to_client thread, and then enqueue the SQL.
void * enqueue_func(void* ptr)
{
    int* index = (int*)ptr;
    char buffer[BUFFER_SIZE];
    char* sql_ptr = NULL;
    log_msg(LEVEL_INFO, "enqueue Worker %lu start\n", *index);
    // give talk_to_client thread some time to download the davinci log 
    sleep(10);

    char file_name[FILE_NAME_MAX_SIZE+1];  
    char file_name_now[FILE_NAME_MAX_SIZE+1];
    g_is_running = 1;

    bzero(file_name, FILE_NAME_MAX_SIZE + 1);  
    pthread_mutex_lock(&s_queue_mutex);
    snprintf(file_name, FILE_NAME_MAX_SIZE, "%s.%d", g_file_name, *index);
    pthread_mutex_unlock(&s_queue_mutex);
    long long line_count = 0;
    char current_line[DAVINCI_BUFFER_SIZE_BIG] = {0};
    bzero(current_line, DAVINCI_BUFFER_SIZE_BIG);

    while(g_is_running)
    {
        FILE* a = NULL;
        int bytes_read = 0;
        int bytes_read_big = 0;
        long int offset = 0;
        long int last_offset = 0;
        int i = 0;
        int nsl = 0;
        char buf[DAVINCI_BUFFER_SIZE_BIG];
        int rc = 0;

        a = fopen(file_name, "r");
        if (a == 0 || errno != 0)
        {
            log_msg(LEVEL_INFO, "the file:%s doesn't exist!!\n", file_name);
            sleep(10);
            // it might be due to the log rotation, so update the file name and retry.
            // OR talk_to_client thread has some problems, keep on retrying.
            bzero(file_name, FILE_NAME_MAX_SIZE + 1);   
            pthread_mutex_lock(&s_queue_mutex);
            snprintf(file_name, FILE_NAME_MAX_SIZE, "%s.%d", g_file_name, *index);
            pthread_mutex_unlock(&s_queue_mutex);
            errno = 0;
            continue;
        }

        fseek(a, 0, SEEK_SET);
        if (errno != 0)
        {
            log_msg(LEVEL_INFO, "fseek 0 errno is:%d\n", errno);
            break;
        }
        while (1)
        {
            char *sql = NULL;
            bzero(buf, DAVINCI_BUFFER_SIZE_BIG);
            pthread_mutex_lock(&s_queue_mutex);
            fgets(buf, DAVINCI_BUFFER_SIZE_BIG, a);
            if (errno != 0)
            {
                log_msg(LEVEL_INFO, "enqueue thread %lu fgets errno is:%d\n", *index, errno);
                if (errno == 9)
                {
                    errno = 0;
                    nsl++;
                    if (nsl > MAX_WAITING_COUNT)
                    {
                        break;
                    }

                    fclose(a);
                    if (errno != 0)
                    {
                        log_msg(LEVEL_INFO, "close errno is:%d\n", errno);
                        // the file descritor is corrupt. ignore it.
                        errno = 0;
                        //break;
                    }

                    sleep(10);
                    a = fopen(file_name, "r");
                    if (a == 0 || errno != 0)
                    {
                        log_msg(LEVEL_INFO, "open errno is:%d\n", errno);
                        break;
                    }

                    log_msg(LEVEL_INFO, "enqueue thread %lu fseek offset is:%ld\n", *index, offset);
                    rc = fseek(a, offset, SEEK_SET);
                    if (rc == -1 || errno != 0)
                    {
                        log_msg(LEVEL_INFO, "lseek errno is:%d\n", errno);
                        break;
                    }
                    pthread_mutex_unlock(&s_queue_mutex);
                    continue;
                }
                else
                {
                    log_msg(LEVEL_INFO, "enqueue thread %lu breaks\n", *index);
                    pthread_mutex_unlock(&s_queue_mutex);
                    break;
                }
            }
            line_count++;
            if (line_count % 10000 == 0)
            {
                log_msg(LEVEL_INFO, "enqueue thread %lu line_count: %lu.\n", *index, line_count);
            }

            pthread_mutex_unlock(&s_queue_mutex);
            bytes_read = strlen(buf);

            if(feof(a))
            {
                log_msg(LEVEL_INFO, "enqueue thread %lu reach the EOF!! offset is:%ld.\n",
                        *index, offset);
                if (offset == 0)
                {
                    break;
                }
                bzero(file_name_now, FILE_NAME_MAX_SIZE + 1);  
                pthread_mutex_lock(&s_queue_mutex);
                snprintf(file_name_now, FILE_NAME_MAX_SIZE, "%s.%d", g_file_name, *index);
                bytes_read = 0;
                //pthread_mutex_lock(&s_queue_mutex);
                if (strncmp(file_name, file_name_now, strlen(file_name)) != 0)
                {
                    // remove the old davinci log and exit the thread.
                    if (remove(file_name) == 0)
                    {
                        log_msg(LEVEL_INFO, "Removed %s.\n", file_name);
                    }
                    else
                    {
                        log_msg(LEVEL_INFO, "Removed %s failed.\n", file_name);
                    }
                    log_msg(LEVEL_INFO, " The End of today, enqueue thread exits!!\n");
                    pthread_mutex_unlock(&s_queue_mutex);
                    break;
                }
                else
                {
                    sleep(1);
                }
                pthread_mutex_unlock(&s_queue_mutex);
            }

            offset += bytes_read;
            if (bytes_read == 0)
            {
                //log_msg(LEVEL_INFO, "offset:%ld, bytes_read:%ld\n", offset, bytes_read);
                nsl++;
                if (nsl > MAX_WAITING_COUNT)
                {
                    break;
                }
                fclose(a);
                if (errno != 0)
                {
                    log_msg(LEVEL_INFO, "close errno is:%d\n", errno);
                    break;
                }

                sleep(10);
                a = fopen(file_name, "r");
                if (a == 0 || errno != 0)
                {
                    log_msg(LEVEL_INFO, "open errno is:%d\n", errno);
                    break;
                }

                log_msg(LEVEL_INFO, "enqueue thread %lu fseek offset is:%ld\n", *index, offset);
                rc = fseek(a, offset, SEEK_SET);
                if (rc == -1 || errno != 0)
                {
                    log_msg(LEVEL_INFO, "lseek errno is:%d\n", errno);
                    break;
                }
            }
            else
            {
                while(g_queue_node_count >= MAX_QUEUE_SIZE)
                {
                    usleep(10);
                }

                if (line_count <= 3)
                {       
                    continue;
                }       

                sql = strstr(buf, "/*##"); 
        
                if (sql == NULL)
                {       
                    // if last buf is truncated
                    if (strlen(current_line) > 0)
                    {       
                        //strcat(current_line, buf);
                        if (strlen(current_line) + strlen(buf) < DAVINCI_BUFFER_SIZE_BIG)
                        {
                            strncat(current_line, buf, strlen(buf));
                        }
                    }       
                    else//truncated before /*##
                    {
                        snprintf(current_line, sizeof(current_line), "%s", buf);
                    }
                    continue;
                }       
                else    
                {       
                    /* First sql */
                    if (!strlen(current_line))
                    {       
                        // buf not sql, sql poinits to /*##
                        snprintf(current_line, sizeof(current_line), "%s", buf);
                        continue;
                    }       
                }       
                /* This is another new sql */
                sql = current_line; 
                
                int sql_len = strlen(sql);
                sql_ptr = (char*)malloc(sql_len + 1);
                bzero(sql_ptr, sql_len + 1);
                snprintf(sql_ptr, sql_len, "%s", sql);

                // this buf is a new line contains /*##
                snprintf(current_line, sizeof(current_line), "%s", buf);

                if (g_debug_on)
                {
                    log_msg(LEVEL_INFO, "sql_ptr is:%s\n", sql_ptr);
                }
                pthread_mutex_lock(&s_queue_mutex); 
                enqueue(sql_ptr, g_sql_queue);
                g_sql_count++;
                if (g_sql_count % 10000 == 0)
                {
                    log_msg(LEVEL_INFO, "g_sql_count: %lu.\n", g_sql_count);
                }
                g_queue_node_count++;
                pthread_mutex_unlock(&s_queue_mutex); 
            }
        }
        // if open failed, a might be 0, then closing it is wrong.
        if (a)
        {
            fclose(a);
        }
        break;
    }
    //fprintf(stderr,"Worker %lu exit\n",wa->worker_id);
    log_msg(LEVEL_INFO, "enqueue Worker %lu exit\n", *index);
    pthread_exit(NULL);
}

// pop the SQL from queue and call the rule engine.
void * dispatch_func(void)
{
    char* sql_ptr = NULL;
    char *sql = NULL;
    log_msg(LEVEL_INFO, "dispatch Worker start\n");
    g_is_running = 1;
    while(g_is_running)
    {
        while (g_queue_node_count > 0)
        {
            pthread_mutex_lock(&s_queue_mutex); 
            if (!is_empty(g_sql_queue))
            {
                sql_ptr = front_and_dequeue(g_sql_queue);
                g_queue_node_count--;
                if (sql_ptr)
                {
                    if (g_debug_on)
                    {
                        log_msg(LEVEL_INFO, "SQL is poped: %s", sql_ptr);
                    }
                    else if (g_queue_node_count % 10000 == 0 && g_queue_node_count != 0)
                    {
                        log_msg(LEVEL_INFO, "SQL is poped: g_queue_node_count:%d\n", 
                                g_queue_node_count);
                    }

                    sql = NULL;
                    sql = strstr(sql_ptr, "##*/") + 4;
                    if (sql == (char *) 0x04)
                    {
                        log_msg(LEVEL_INFO, "SQL log file format not recognized! no sql!\n");
                        log_big_msg(LEVEL_INFO, "%s\n", sql_ptr);
                        //log_msg(LEVEL_INFO, "\n");
                        //continue to process the next sql.
                        free(sql_ptr);
                        pthread_mutex_unlock(&s_queue_mutex);
                        continue;
                    }
                    davinci_log_rec_t log_rec_head;
                    memset(&log_rec_head, 0, sizeof(log_rec_head));
                    //log_msg(LEVEL_INFO, "davinci record to be parsed: %s\n", sql_ptr);
                    /*
                    sscanf(sql_ptr, "%s %s %[^/*##]/*## %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
                        log_rec_head.dbname,
                        log_rec_head.tbname,
                        log_rec_head.queryType,

                        log_rec_head.reserved,
                        log_rec_head.dbhost,
                        log_rec_head.dbport,

                        log_rec_head.username,
                        log_rec_head.userhost,

                        log_rec_head.queryDate,
                        log_rec_head.queryTime,

                        log_rec_head.sendQueryTime,
                        log_rec_head.receiveResultTime,
                        log_rec_head.sendResultTime,
                        log_rec_head.totalTime,
                        log_rec_head.affectedRows,
                        log_rec_head.resultSize,

                        log_rec_head.queryCount,
                        log_rec_head.logType,
                        log_rec_head.json,
                        log_rec_head.GetSign);
                        */
                    char *ptr_start = strstr(sql_ptr, "/*##");
                    if (ptr_start == NULL)
                    {
                        log_msg(LEVEL_INFO, "SQL log file format not recognized! no header!\n");
                        log_big_msg(LEVEL_INFO, "%s\n", sql_ptr);
                        //log_msg(LEVEL_INFO, "\n");
                        //continue to process the next sql.
                        free(sql_ptr);
                        pthread_mutex_unlock(&s_queue_mutex);
                        continue;
                    }
                    //ptr_start += 4;
                    //sscanf(ptr_start, "%s %s", log_rec_head.reserved, log_rec_head.dbhost);
                    sscanf(sql_ptr, "%s %s %[^/*##]/*## %s %s", 
                        log_rec_head.dbname,
                        log_rec_head.tbname,
                        log_rec_head.queryType,

                        log_rec_head.reserved, 
                        log_rec_head.dbhost);
                    // FIX-ME: remove some normal SQLs from normal_sql for performance enhancement
                    // need to add them to config files later.
                    if ((strlen(sql) < DAVINCI_BUFFER_SIZE 
                            || strstr(sql, "OR (plat='s' AND productLine='s') \
                                    OR (plat='s' AND productLine='s')") == NULL
                            || strstr(sql, " OR namespace='s' OR namespace='s' OR ") == NULL
                            || strstr(sql, "update poms_superfile99_part_tmp__tablet_99 \
                                    set upload_id = unhex (?) where ") == NULL
                            || strstr(sql, "type from que_all_9 where que_id") == NULL
                            || strstr(sql, "from coupon_base where activity_id") == NULL
                            || strstr(sql, "update express_order set owner = ? \
                                    where ? and owner = ? and cuid") == NULL
                            )
                            && (strncmp(log_rec_head.queryType, 
                                "SYNTAX_ERROR", strlen("SYNTAX_ERROR")) != 0)
                            && strlen(sql) < DAVINCI_BUFFER_SIZE*10
                            )
                    {
                        // call rule engine 
                        sql_check_sql(&log_rec_head, sql, sql_ptr);
                    }
                    free(sql_ptr);
                    sql_ptr = NULL;
                }
            }
            pthread_mutex_unlock(&s_queue_mutex); 
        }
        usleep(10);
    }
    log_msg(LEVEL_INFO, "g_sql_count: %lu.\n", g_sql_count);
    log_msg(LEVEL_INFO, "dispatch Worker exit\n");
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    signal(SIGTERM, node_exit);
    signal(SIGINT, node_exit);

    //init the log handle
    g_log_handle = log_init("./ids_server.log", 400000000);
    g_queue_node_count = 0;
    g_sql_queue = create_queue();
    log_msg(LEVEL_INFO, "Initialization complete.\n");
    pthread_mutex_init(&s_queue_mutex, NULL);

    bzero(g_file_name, FILE_NAME_MAX_SIZE+1);  
    time_t rawtime;
    struct tm * timeinfo;
    char time_buf[128]; 

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    bzero(time_buf, 128);  
    strftime(time_buf, sizeof(time_buf), "%Y%m%d", timeinfo);
    snprintf(g_file_name, FILE_NAME_MAX_SIZE, "davinci_srv.log.%s", time_buf);  
    log_msg(LEVEL_INFO, "g_file_name is:%s\n", g_file_name);  

    //init the sql 
    sql_init();
    sql_init2();
    //设置一个socket地址结构server_addr,代表服务器internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr)); //把一段内存区的内容全部设置为0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    //server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    server_addr.sin_port = htons(g_ids_server_port);
    log_msg(LEVEL_INFO, "Server Bind Port : %d .\n", g_ids_server_port); 

    //创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        log_msg(LEVEL_INFO, "Create Socket Failed!");
        exit(1);
    }
    
    //把socket和socket地址结构联系起来
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        //log_msg(LEVEL_INFO, "Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT); 
        log_msg(LEVEL_INFO, "Server Bind Port : %d Failed!", g_ids_server_port); 
        exit(1);
    }
    
    //server_socket用于监听
    if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
    {
        log_msg(LEVEL_INFO, "Server Listen Failed!"); 
        exit(1);
    }

    /*
    //init the sql 
    sql_init();
    sql_init2();
    */

    int j = 0;
    for (j = 0; j< g_davinci_server_count; j++)
    {
        log_msg(LEVEL_INFO, "davinci server[%d]:%s\n", j, g_davinci_servers[j]);
    }

    pthread_t dispatch_thread;
    pthread_attr_t dispatch_thread_attr;
    pthread_attr_init(&dispatch_thread_attr);
    pthread_attr_setdetachstate(&dispatch_thread_attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&dispatch_thread, &dispatch_thread_attr, dispatch_func, NULL) < 0)
    {
        log_msg(LEVEL_INFO, "pthread_create Failed : %s\n", strerror(errno));
    }

    int rank[THREAD_MAX];
    int i = 0;
    for (i = 0;i < THREAD_MAX ;i++)
    {
        g_wa_array[i].new_socket = 0;
        g_wa_array[i].worker_id = 0;
        rank[i] = i;
    }

    while(1)
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);

        // accept a new connection if its valid, otherwise suspend here.
        // new_server_socket is the socket that can be used for communication.
        //int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
        int new_server_socket = accept(server_socket, (struct sockaddr*)NULL, NULL);
        if (new_server_socket < 0)
        {
            log_msg(LEVEL_INFO, "Server Accept Failed!\n");
            break;
        }

        getpeername(new_server_socket, (struct sockaddr *)&client_addr, &length);
        char peer_ip_addr[16];
        log_msg(LEVEL_INFO, "connected peer address = %s:%d\n", 
                inet_ntop(AF_INET, &client_addr.sin_addr, peer_ip_addr, sizeof(peer_ip_addr)), 
                ntohs(client_addr.sin_port));

        int ip_index = 0; 
        if (strncmp(peer_ip_addr, "127.0.0.1", strlen(peer_ip_addr)) == 0)
        {
            log_msg(LEVEL_INFO, " console from localhost connected.\n");
            ip_index = THREAD_MAX - 1;
            if (g_socket_active_array[ip_index] != 0)
            {
                log_msg(LEVEL_INFO, 
                        "there is already a socket from this ip. refuse the connection.\n");
                continue;
            }
            pthread_t child_thread;
            pthread_attr_t child_thread_attr;
            pthread_attr_init(&child_thread_attr);
            pthread_attr_setdetachstate(&child_thread_attr, PTHREAD_CREATE_DETACHED);
            g_wa_array[ip_index].new_socket = new_server_socket; 
            g_wa_array[ip_index].worker_id = ip_index; 

            if (pthread_create(&child_thread, &child_thread_attr, 
                        talk_to_client, (void *)(&g_wa_array[ip_index])) < 0)
            {
                log_msg(LEVEL_INFO, "pthread_create Failed : %s\n", strerror(errno));
            }
            continue;
        }

        ip_index = get_index_of_agent_ip_tables(peer_ip_addr); 

        log_msg(LEVEL_INFO, "ip_index is:%d.\n", ip_index);
        if (ip_index == -1)
        {
            log_msg(LEVEL_INFO, "this ip is invalid. refuse the connection.\n");
            continue;
        }
        if (g_socket_active_array[ip_index] != 0)
        {
            log_msg(LEVEL_INFO, "there is already a socket from this ip. refuse the connection.\n");
            continue;
        }
        pthread_mutex_lock(&s_queue_mutex);
        g_connected_agent_count++;
        pthread_mutex_unlock(&s_queue_mutex);
        log_msg(LEVEL_INFO, "connected agent count is:%d.\n", g_connected_agent_count);

        // create a talk_to_client thread to handle the socket connection. 
        pthread_t child_thread;
        pthread_attr_t child_thread_attr;
        pthread_attr_init(&child_thread_attr);
        pthread_attr_setdetachstate(&child_thread_attr, PTHREAD_CREATE_DETACHED);
        g_wa_array[ip_index].new_socket = new_server_socket; 
        g_wa_array[ip_index].worker_id = ip_index; 

        if (pthread_create(&child_thread, &child_thread_attr, 
                    talk_to_client, (void *)(&g_wa_array[ip_index])) < 0)
        {
            log_msg(LEVEL_INFO, "pthread_create Failed : %s\n", strerror(errno));
        }

        // create an enqueue_thread for a new connection.
        pthread_t enqueue_thread;
        pthread_attr_t enqueue_thread_attr;
        pthread_attr_init(&enqueue_thread_attr);
        pthread_attr_setdetachstate(&enqueue_thread_attr, PTHREAD_CREATE_DETACHED);
        if (pthread_create(&enqueue_thread, &enqueue_thread_attr, 
                    enqueue_func, (void *)(&rank[ip_index])) < 0)
        {
            log_msg(LEVEL_INFO, "pthread_create Failed : %s\n", strerror(errno));
        }
    }
    //close the listen socket
    close(server_socket);
    //clean up the sql related resource.
    sql_cleanup();
    return 0;
}
