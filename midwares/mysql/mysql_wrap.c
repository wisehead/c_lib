/*******************************************************************************
 *      File Name: mysql_wrap.c                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: alex.chenhui@gmail.com                                       
 *   Created Time: 2016/10/10-18:28                                    
 *	Modified Time: 2016/10/10-18:28                                    
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <glib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>    // for sockaddr_in  
#include <sys/types.h>    // for socket  
#include <sys/socket.h>    // for socket  

#include <mysql.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "api.h"

#include "sql_define.h"
#include "sql_config.h"
#include "ids_log.h"
#include "ids_helper.h"

#define STRING_SIZE 131070 //for some big SQLs
#define BUFFER_SIZE 16384  //for some big SQLs
#define ERR_MSG_LEN 100   
const char *g_config_file = "conf/ids_server.conf";
const char *g_re_config = "conf/dtest_re.conf";

/* log file which contains the SQLs. */
char g_sql_file[256][128] = {0};
int g_sql_file_count;

/* IP address of davinci serser. */
char g_davinci_servers[256][128] = {0};
int g_davinci_server_count;
unsigned int g_ids_server_port;
char g_ml_server_ip[16] = {0};
unsigned int g_ml_server_port;
/* if the log file is in davinci format */
char g_sql_file_format[16] = {0};

/* mysql connection parameters */
char g_host[64];
char g_user[32];
char g_password[32];
unsigned int g_port;
char g_db[32];
static char s_socket[256];//use the static, because in other files, we met a name confict.

MYSQL *g_conn = NULL;
MYSQL_STMT *g_stmt_err = NULL;
MYSQL_STMT *g_stmt_normal = NULL;
MYSQL_STMT *g_stmt_suspect = NULL;
MYSQL_STMT *g_stmt_insert_exception = NULL;
MYSQL_STMT *g_stmt_select_normal = NULL;
MYSQL_STMT *g_stmt_select_exception = NULL;
MYSQL_STMT *g_stmt_select_suspect = NULL;
MYSQL_STMT *g_stmt_select_error = NULL;
MYSQL_BIND g_bind_err[11];
MYSQL_BIND g_bind_normal[3];
//MYSQL_BIND g_bind_suspect[4];
MYSQL_BIND g_bind_suspect[12];
MYSQL_BIND g_bind_insert_exception[4];
MYSQL_BIND g_bind_select_exception[4];
MYSQL_BIND g_bind_select_normal[4];
MYSQL_BIND g_bind_select_suspect[2];
MYSQL_BIND g_bind_select_error[2];

//char g_buf[1024000];
//char g_clone_buf[1024000];

unsigned int g_total_count;
unsigned int g_normal_count;
unsigned int g_error_count;
unsigned int g_re_sql_count;

GHashTable * g_hash_table_for_normal_sql;
GHashTable * g_hash_table_for_normal_sql_distinct;
GHashTable * g_hash_table_for_error_sql;
GHashTable * g_hash_table_for_suspect_sql;
GHashTable * g_hash_table_for_exception_sql;

mem_pool_t *g_parser_pool;
sql_parser_t *g_sql_parser;

char g_check_re;
char g_machine_learning_on;

#define MAX_RE 64
pcre2_code *g_re_array[MAX_RE];
unsigned int g_re_count = 0;
pcre2_match_data *g_match_data = NULL;

// in nano seconds
unsigned long g_total_time;
unsigned long g_time_read_file;
unsigned long g_total_check_time;
unsigned long g_normalization_time;
unsigned long g_time_check_normal;
unsigned long g_time_check_error;
unsigned long g_time_check_suspect; // parser time for suspect sql
unsigned long g_time_insert;
unsigned long g_time_re;

void set_parameter (char *parameter, char *value);
void dump_connect_parameters();
int load_connect_config();
void sql_re_cleanup();
int sql_load_re();
void print_report();

long time_subtract(struct timespec *end, struct timespec *start)
{
    return (end->tv_sec - start->tv_sec) * pow(10, 9) +
           (end->tv_nsec - start->tv_nsec);
}

char *sql_get_sql(char *line)
{
    char *sql = NULL;

    if (line == NULL)
    {
        return NULL;
    }

    sql = strstr(line, "Query") + 6;

    /* 
      There is no need to check Prepare and Execute,
      they will never be injected.
    */
    /*
    if (sql <= (char *) 0x06)
    {
        sql = strstr(line, "Prepare") + 8;
    }

    if (sql <= (char *) 0x08)
    {
        sql = strstr(line, "Execute") + 8;
    }
    */

    return sql;
}

static void my_on_exit(int fd, int fd_read)
{
    if (fd)
    {
        close(fd);
    }
    if (fd_read)
    {
        close(fd_read);
    }
    mp_clear(g_parser_pool);
    return;
} 

void print_key_value(gpointer key, gpointer value, gpointer user_data)
{
    log_msg(LEVEL_INFO, "%s ---> %s\n", (unsigned int*)key, value);
}

void display_hash_table(GHashTable *table)
{
    g_hash_table_foreach(table, print_key_value, NULL);
}

void sql_check_sql(davinci_log_rec_t* log_rec_head, char *sql, char *sql_ptr)
{
    int client_socket = 0;
    int ret = 0;
    struct timespec start_time = {0};
    struct timespec end_time   = {0};
    unsigned long time_spent   = 0;
    int fd = 0;
    int fd_read = 0;
    if (log_rec_head == NULL || sql == NULL)
    {
        my_on_exit(fd, fd_read);
        return;
    }

    if (strcasestr(sql, "XA") ||
        strstr(sql, "......"))
    {
        my_on_exit(fd, fd_read);
        return;
    }

    g_total_count++;
    
    /*
    if (g_total_count % 1000 == 0)
    {
        log_msg(LEVEL_INFO, "processed %lu SQLs...\n", g_total_count);
    }
    */

    sql_parser_clear(g_sql_parser);

    if (clock_gettime(CLOCK_REALTIME, &start_time))
    {
        log_msg(LEVEL_INFO, "failed to get parse start time, time indicator might be incorrect.\n");
    }

    char sql_text[BUFFER_SIZE] = {'\0'};
    char sql_temp[BUFFER_SIZE*2] = {'\0'};
    unsigned long sql_len_normalized = 0;
    unsigned long sql_len_original = 0;
    unsigned int id = 0;
    unsigned int db_host = 0;

    //strncpy(sql_text, sql, (BUFFER_SIZE - 1));
    snprintf(sql_text, BUFFER_SIZE, "%s", sql);
    sql_len_original = strlen(sql_text);
    if (clock_gettime(CLOCK_REALTIME, &start_time)) {
        log_msg(LEVEL_INFO, 
                "failed to get normalization start time, time indicator might be incorrect.\n");
    }

    (void)sql_normalize_en(sql_text, sql_len_original, &id, sql_temp);

    if (clock_gettime(CLOCK_REALTIME, &end_time))
    {
        log_msg(LEVEL_INFO, 
                "failed to get normalization end time, time indicator might be incorrect.\n");
    }

    g_normalization_time += time_subtract(&end_time, &start_time);

    char new_id[STR_LEN];
    bzero(new_id, STR_LEN);
    snprintf(new_id, STR_LEN, "%s%u", log_rec_head->dbhost, id);
    db_host = atoi(log_rec_head->dbhost);
    //log_msg(LEVEL_INFO, "new_id is:%s. dbhost:%s, id:%u\n", new_id, log_rec_head->dbhost, id);
    // lookup the hash table first, if hits, then no need to parse the SQL.
    //if (g_hash_table_lookup(g_hash_table_for_normal_sql, &new_id))
    // use g_hash_table_for_normal_sql_distinct instead 
    if (g_hash_table_lookup(g_hash_table_for_normal_sql_distinct, &id))
    {
        //log_msg(LEVEL_INFO, "[INFO]the SQL hits the normal SQL cache.\n");
        my_on_exit(fd, fd_read);
        return;
    }
    else
    {
        //log_msg(LEVEL_INFO, "new_id is:%s. dbhost:%s, id:%u\n", new_id, log_rec_head->dbhost, id);
    }

    //display_hash_table(g_hash_table_for_normal_sql_distinct);
    //exit(1);
    // lookup the hash table first, if hits, then no need to parse the SQL.
    //if (g_hash_table_lookup(g_hash_table_for_exception_sql, &id))
    if (g_hash_table_lookup(g_hash_table_for_exception_sql, &new_id))
    {
        //log_msg(LEVEL_INFO, "[INFO]the SQL hits the exception SQL cache.\n");
        my_on_exit(fd, fd_read);
        return;
    }

    // lookup the black list hash table first, if hits, then no need to parse the SQL.
    if (g_hash_table_lookup(g_hash_table_for_error_sql, &id))
    {
        //log_msg(LEVEL_ERROR, "the SQL hits the error SQL cache.\n");
        my_on_exit(fd, fd_read);
        return;
    }
    //display_hash_table(g_hash_table_for_error_sql);
    // lookup the suspect hash table first, if hits, then no need to parse the SQL.
    if (g_hash_table_lookup(g_hash_table_for_suspect_sql, &new_id))
    {
        //log_msg(LEVEL_ERROR, "the SQL hits the suspect SQL cache.\n");
        my_on_exit(fd, fd_read);
        return;
    }
    //log_msg(LEVEL_INFO, "new_id is:%s. dbhost:%s, id:%u\n", new_id, log_rec_head->dbhost, id);

    ret = check_sql(sql, g_sql_parser, 0, NULL, 0, NULL, NULL);

    if (clock_gettime(CLOCK_REALTIME, &end_time))
    {
        log_msg(LEVEL_INFO, "failed to get parse end time, time indicator might be incorrect.\n");
    }

    time_spent = time_subtract(&end_time, &start_time);
    g_total_check_time += time_spent;

    if (ret == SQL_SUCC && g_sql_parser->err_no == SQL_SUCC)
    {
        bool suspect_flag = false;

        if (sql_check_sql_re(sql))
        {
            g_time_check_suspect += time_spent;
            my_on_exit(fd, fd_read);
            return;
        }
        if (!g_hash_table_lookup(g_hash_table_for_normal_sql_distinct, &id))
        {

            if (g_machine_learning_on == 1)
            {
                char buf[1024];
                memset(buf, 0, 1024);
                int ret = 0;
                //const char* ip_addr = "10.46.206.59";
                //const char* ip_addr = "127.0.0.1";
                const char* ip_addr = g_ml_server_ip;
                struct sockaddr_in client_addr;
                //int client_socket = 0;  
                struct timeval timeout = {15, 0};
                char sendbuf[1024];
                char recvbuf[1024];

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
                
                unsigned int rand_ids_num = 0;
                
                log_msg(LEVEL_INFO, "Connecting to ML server:%s\n", ip_addr);
                if(inet_aton(ip_addr, &server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数  
                {       
                    log_msg(LEVEL_INFO, "Server IP Address Error!\n");  
                    exit(1);  
                }       

                server_addr.sin_port = htons(g_ml_server_port);  
                socklen_t server_addr_length = sizeof(server_addr);  
                //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接  
                if (connect(client_socket, 
                            (struct sockaddr*)&server_addr, server_addr_length) < 0)  
                {       
                    log_msg(LEVEL_INFO, "Can Not Connect To %s!\n", ip_addr);  
                    exit(1);  
                }  
                //bzero(sendbuf, 1024);
                //snprintf(sendbuf, 1024, "%s", sql_text);
                
                if (send(client_socket, sql_text, sql_len_original, 0) < 0)  
                {  
                    if (errno != 0)
                    {
                        log_msg(LEVEL_INFO, "send errno is:%d\n", errno);
                        errno = 0;
                    }
                    log_msg(LEVEL_INFO, "Send SQL:\t%s Failed\n", sql_text);  
                }  
                socklen_t length = sizeof(client_addr);  
                int retry = 0;
                while (retry < 600)
                {
                    length = recv(client_socket, buf, 1024, 0);  
                    int my_length = length;
                    if (length < 0)  
                    {  
                        log_msg(LEVEL_INFO, "Server Recieve Data Failed!\n");  
                    }  
                    //log_msg(LEVEL_INFO, "buf from ML server is :%d!\n", buf[0]);  
                    //log_msg(LEVEL_INFO, "buf from ML server is :%s!\n", buf);  
                    sleep(1);
                    retry++;
                    //log_msg(LEVEL_INFO, "length of buf from ML server is :%d!\n", strlen(buf));  
                    //log_msg(LEVEL_INFO, "length is :%d!\n", length);  
                    //log_msg(LEVEL_INFO, "my_length is :%d!\n", my_length);  
                    if (my_length > 0)
                    { 
                        log_msg(LEVEL_INFO, "buf from ML server is :%s!\n", buf);  
                        //log_msg(LEVEL_INFO, "length of buf from ML server is :%d!\n", strlen(buf));  
                        break;
                    }
                }

                //close(client_socket);  

                if (buf[0] == '1')
                {
                    log_msg(LEVEL_ERROR, "this SQL is dubious. inserting to table suspect_sql.\n"); 
                    suspect_flag = true;
                }
                else if (buf[0] == '0')
                {
                    g_time_check_normal += time_spent;
                    log_msg(LEVEL_ERROR, "this SQL is normal.\n"); 
                    //log_msg(LEVEL_ERROR, "this SQL is normal. inserting to table normal_sql.\n"); 
                    g_normal_count++;
                }
                else
                {
                    log_msg(LEVEL_ERROR, "the data from machine learning is incorrect.\n"); 
                    my_on_exit(fd, fd_read);
                    return;
                }
            }
        }
            
        if (suspect_flag)
        {
            //g_hash_table_insert(g_hash_table_for_suspect_sql, &id, sql_temp);
            g_hash_table_insert(g_hash_table_for_suspect_sql, &new_id, (char*)1);
            sql_len_normalized = strlen(sql_temp);

            memset(g_bind_suspect, 0, sizeof(g_bind_suspect));

            {
                char err_msg[ERR_MSG_LEN] = {'\0'};
                char sql_text[BUFFER_SIZE] = {'\0'};
                //unsigned int  db_port = 0;
                unsigned long sql_length = 0;
                unsigned long err_msg_length = 0;
                unsigned long user_name_length = 0;
                unsigned long user_host_length = 0;
                unsigned long db_name_length = 0;
                unsigned long table_name_length = 0;
                unsigned long log_type_length = 0;

                if (ret == SQL_SUCC)
                {
                    ret = g_sql_parser->err_no;
                }

                memset(log_rec_head, 0, sizeof(log_rec_head));
                //log_msg(LEVEL_INFO, "davinci record to be parsed: %s\n", sql_ptr);
                // parse the davinci log record
                sscanf(sql_ptr, 
                    "%s %s %[^/*##]/*## %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
                    log_rec_head->dbname,
                    log_rec_head->tbname,
                    log_rec_head->queryType,

                    log_rec_head->reserved,
                    log_rec_head->dbhost,
                    log_rec_head->dbport,

                    log_rec_head->username,
                    log_rec_head->userhost,

                    log_rec_head->queryDate,
                    log_rec_head->queryTime,

                    log_rec_head->sendQueryTime,
                    log_rec_head->receiveResultTime,
                    log_rec_head->sendResultTime,
                    log_rec_head->totalTime,
                    log_rec_head->affectedRows,
                    log_rec_head->resultSize,

                    log_rec_head->queryCount,
                    log_rec_head->logType,
                    log_rec_head->json,
                    log_rec_head->GetSign);

                //strncpy(err_msg, sql_err_to_str(ret), 32);
                bzero(err_msg, ERR_MSG_LEN);
                snprintf(err_msg, ERR_MSG_LEN - 1, "%s", sql_err_to_str(ret));
                err_msg_length = strlen(err_msg);

                //intrude time
                struct tm tm_time;
                MYSQL_TIME  ts;
                char datetime[STR_LEN];
                bzero(datetime, STR_LEN);
                snprintf(datetime, STR_LEN, "%s %s", 
                        log_rec_head->queryDate, log_rec_head->queryTime);
                strptime(datetime, "%Y-%m-%d %H:%M:%S", &tm_time);

                /* supply the data to be sent in the ts structure */
                ts.year = tm_time.tm_year + 1900;
                ts.month = tm_time.tm_mon + 1;
                ts.day = tm_time.tm_mday;

                ts.hour = tm_time.tm_hour;
                ts.minute = tm_time.tm_min;
                ts.second = tm_time.tm_sec;
                //log_msg(LEVEL_INFO, "%d-%d-%d %d:%d:%d\n",
                //        ts.year, ts.month,  ts.day, ts.hour, ts.minute, ts.second);

                // db host, ip addr num 
                unsigned long int  db_host = 0;
                db_host = atol(log_rec_head->dbhost);       

                // db port
                unsigned int  db_port = 0;
                db_port = atoi(log_rec_head->dbport);       

                user_name_length = strlen(log_rec_head->username);
                user_host_length = strlen(log_rec_head->userhost);
                db_name_length = strlen(log_rec_head->dbname);
                table_name_length = strlen(log_rec_head->tbname);

                // query count
                unsigned int  query_count = 0;
                query_count = atoi(log_rec_head->queryCount);

                // log type
                log_type_length = strlen(log_rec_head->logType);

                //strncpy(sql_text, sql, (BUFFER_SIZE - 1));
                snprintf(sql_text, (BUFFER_SIZE - 1), "%s", sql);
                sql_length = strlen(sql_text);

                memset(g_bind_suspect, 0, sizeof(g_bind_suspect));

                g_bind_suspect[0].buffer_type = MYSQL_TYPE_LONG;
                g_bind_suspect[0].is_unsigned = 1;
                g_bind_suspect[0].buffer = (char *)&id;
                g_bind_suspect[0].is_null = 0;
                g_bind_suspect[0].length = 0;

                /* error type */
                g_bind_suspect[1].buffer_type = MYSQL_TYPE_STRING;
                g_bind_suspect[1].buffer = (char *)err_msg;
                g_bind_suspect[1].buffer_length = sizeof(err_msg);
                g_bind_suspect[1].is_null = 0;
                g_bind_suspect[1].length = &err_msg_length;

                /* intrude time */
                g_bind_suspect[2].buffer_type = MYSQL_TYPE_DATETIME;
                g_bind_suspect[2].buffer = (char *)&ts;
                g_bind_suspect[2].is_null = 0;
                g_bind_suspect[2].length = 0;

                log_msg(LEVEL_INFO, "found new injection:%s\n", sql_ptr);
                log_msg(LEVEL_INFO, "\n");
            //log_msg(LEVEL_INFO, "%s %s %s/*## %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",
            //    log_rec_head->dbname,
            //    log_rec_head->tbname,
            //    log_rec_head->queryType,

            //    log_rec_head->reserved,
            //    log_rec_head->dbhost,
            //    log_rec_head->dbport,

            //    log_rec_head->username,
            //    log_rec_head->userhost,

            //    log_rec_head->queryDate,
            //    log_rec_head->queryTime,

            //    log_rec_head->sendQueryTime,
            //    log_rec_head->receiveResultTime,
            //    log_rec_head->sendResultTime,
            //    log_rec_head->totalTime,
            //    log_rec_head->affectedRows,
            //    log_rec_head->resultSize,

            //    log_rec_head->queryCount,
            //    log_rec_head->logType,
            //    log_rec_head->json,
            //    log_rec_head->GetSign);
                /*db host, ip number*/
                g_bind_suspect[3].buffer_type = MYSQL_TYPE_LONGLONG;
                g_bind_suspect[3].is_unsigned = 1;
                g_bind_suspect[3].buffer = (char *)&db_host;
                g_bind_suspect[3].is_null = 0;
                g_bind_suspect[3].length = 0;

                /* port */
                g_bind_suspect[4].buffer_type = MYSQL_TYPE_LONG;
                g_bind_suspect[4].is_unsigned = 1;
                g_bind_suspect[4].buffer = (char *)&db_port;
                g_bind_suspect[4].is_null = 0;
                g_bind_suspect[4].length = 0;

                /* user name */
                g_bind_suspect[5].buffer_type = MYSQL_TYPE_STRING;
                g_bind_suspect[5].buffer = (char *)log_rec_head->username;
                g_bind_suspect[5].buffer_length = sizeof(log_rec_head->username);
                g_bind_suspect[5].is_null = 0;
                g_bind_suspect[5].length = &user_name_length;

                /* user host */
                g_bind_suspect[6].buffer_type = MYSQL_TYPE_STRING;
                g_bind_suspect[6].buffer = (char *)log_rec_head->userhost;
                g_bind_suspect[6].buffer_length = sizeof(log_rec_head->userhost);
                g_bind_suspect[6].is_null = 0;
                g_bind_suspect[6].length = &user_host_length;

                /* db name */
                g_bind_suspect[7].buffer_type = MYSQL_TYPE_STRING;
                g_bind_suspect[7].buffer = (char *)log_rec_head->dbname;
                g_bind_suspect[7].buffer_length = sizeof(log_rec_head->dbname);
                g_bind_suspect[7].is_null = 0;
                g_bind_suspect[7].length = &db_name_length;

                /* table name */
                g_bind_suspect[8].buffer_type = MYSQL_TYPE_STRING;
                g_bind_suspect[8].buffer = (char *)log_rec_head->tbname;
                g_bind_suspect[8].buffer_length = sizeof(log_rec_head->tbname);
                g_bind_suspect[8].is_null = 0;
                g_bind_suspect[8].length = &table_name_length;

                /* query count*/
                g_bind_suspect[9].buffer_type = MYSQL_TYPE_LONG;
                g_bind_suspect[9].is_unsigned = 1;
                g_bind_suspect[9].buffer = (char *)&query_count;
                g_bind_suspect[9].is_null = 0;
                g_bind_suspect[9].length = 0;

                /* log type */
                g_bind_suspect[10].buffer_type = MYSQL_TYPE_STRING;
                g_bind_suspect[10].buffer = (char *)log_rec_head->logType;
                g_bind_suspect[10].buffer_length = sizeof(log_rec_head->logType);
                g_bind_suspect[10].is_null = 0;
                g_bind_suspect[10].length = &log_type_length;

                /* sql text */
                g_bind_suspect[11].buffer_type = MYSQL_TYPE_STRING;
                g_bind_suspect[11].buffer = (char *)sql_text;
                g_bind_suspect[11].buffer_length = sizeof(sql_text);
                g_bind_suspect[11].is_null = 0;
                g_bind_suspect[11].length = &sql_length;

            }
            /*
            g_bind_suspect[0].buffer_type = MYSQL_TYPE_LONG;
            g_bind_suspect[0].is_unsigned = 1;
            g_bind_suspect[0].buffer = (char *)&id;
            g_bind_suspect[0].is_null = 0;
            g_bind_suspect[0].length = 0;

            g_bind_suspect[1].buffer_type = MYSQL_TYPE_LONG;
            g_bind_suspect[1].is_unsigned = 1;
            g_bind_suspect[1].buffer = (char *)&db_host;
            g_bind_suspect[1].is_null = 0;
            g_bind_suspect[1].length = 0;

            g_bind_suspect[2].buffer_type = MYSQL_TYPE_STRING;
            g_bind_suspect[2].buffer = (char *)sql_temp;
            g_bind_suspect[2].buffer_length = sizeof(sql_temp);
            g_bind_suspect[2].is_null = 0;
            g_bind_suspect[2].length = &sql_len_normalized;

            g_bind_suspect[3].buffer_type = MYSQL_TYPE_STRING;
            g_bind_suspect[3].buffer = (char *)sql_text;
            g_bind_suspect[3].buffer_length = sizeof(sql_text);
            g_bind_suspect[3].is_null = 0;
            g_bind_suspect[3].length = &sql_len_original;
            */
        }
        else 
        {
            //snprintf(new_id, STR_LEN, "%s%u", log_rec_head->dbhost, id);

            //g_hash_table_insert(g_hash_table_for_normal_sql, &new_id, sql_temp);
            // insert 1 instead of sql_temp, to save memory
            // will not insert into the g_hash_table_for_normal_sql any more
            //g_hash_table_insert(g_hash_table_for_normal_sql, &new_id, (char*)1);
            g_hash_table_insert(g_hash_table_for_normal_sql_distinct, &id, (char*)1);
            sql_len_normalized = strlen(sql_temp);
 
            memset(g_bind_normal, 0, sizeof(g_bind_normal));

            g_bind_normal[0].buffer_type = MYSQL_TYPE_LONG;
            g_bind_normal[0].is_unsigned = 1;
            g_bind_normal[0].buffer = (char *)&id;
            g_bind_normal[0].is_null = 0;
            g_bind_normal[0].length = 0;

            // change the schema of table normal_sql, using id as primary key
            // remove the dbhost column
            /*
            g_bind_normal[1].buffer_type = MYSQL_TYPE_LONG;
            g_bind_normal[1].is_unsigned = 1;
            g_bind_normal[1].buffer = (char *)&db_host;
            g_bind_normal[1].is_null = 0;
            g_bind_normal[1].length = 0;
            */

            g_bind_normal[1].buffer_type = MYSQL_TYPE_STRING;
            g_bind_normal[1].buffer = (char *)sql_temp;
            g_bind_normal[1].buffer_length = sizeof(sql_temp);
            g_bind_normal[1].is_null = 0;
            g_bind_normal[1].length = &sql_len_normalized;

            g_bind_normal[2].buffer_type = MYSQL_TYPE_STRING;
            g_bind_normal[2].buffer = (char *)sql_text;
            g_bind_normal[2].buffer_length = sizeof(sql_text);
            g_bind_normal[2].is_null = 0;
            g_bind_normal[2].length = &sql_len_original;
        }

        /* Bind the buffers */
        if (suspect_flag)
        {
            if (mysql_stmt_bind_param(g_stmt_suspect, g_bind_suspect))
            {
                log_msg(LEVEL_ERROR, " mysql_stmt_bind_param() failed for suspect sqls\n");
                log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_suspect));
            }
        }
        else
        {
            if (mysql_stmt_bind_param(g_stmt_normal, g_bind_normal))
            {
                log_msg(LEVEL_ERROR, " mysql_stmt_bind_param() failed for normal sqls\n");
                log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_normal));
            }
        }

        if (clock_gettime(CLOCK_REALTIME, &start_time))
        {
            log_msg(LEVEL_INFO, "failed to get start time of inserting normal sql, "
                    "time indicator might be incorrect.\n");
        }

        /* Execute the INSERT statement - 1*/
        if (suspect_flag)
        {
            if (mysql_stmt_execute(g_stmt_suspect))
            {
                log_msg(LEVEL_ERROR, " mysql_stmt_execute(), 1 failed for suspect sqls\n");
                log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_suspect));
            }
        }
        else
        {
            // will not insert into normal_sql table any more
            /*
            if (mysql_stmt_execute(g_stmt_normal))
            {
                log_msg(LEVEL_ERROR, " mysql_stmt_execute(), 1 failed for normal sqls\n");
                log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_normal));
            }
            */
        }

        if (clock_gettime(CLOCK_REALTIME, &end_time))
        {
            log_msg(LEVEL_INFO, "failed to get end time of inserting normal sql, "
                    "time indicator might be incorrect.\n");
        }

        g_time_insert += time_subtract(&end_time, &start_time);
    }
    else
    {
        char err_msg[ERR_MSG_LEN] = {'\0'};
        char sql_text[BUFFER_SIZE] = {'\0'};
        //unsigned int  db_port = 0;
        unsigned long sql_length = 0;
        unsigned long err_msg_length = 0;
        unsigned long user_name_length = 0;
        unsigned long user_host_length = 0;
        unsigned long db_name_length = 0;
        unsigned long table_name_length = 0;
        unsigned long log_type_length = 0;

        if (ret == SQL_SUCC)
        {
            ret = g_sql_parser->err_no;
        }

        g_time_check_error += time_spent;

        memset(log_rec_head, 0, sizeof(log_rec_head));
        //log_msg(LEVEL_INFO, "davinci record to be parsed: %s\n", sql_ptr);
        // parse the davinci log record
        sscanf(sql_ptr, "%s %s %[^/*##]/*## %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
            log_rec_head->dbname,
            log_rec_head->tbname,
            log_rec_head->queryType,

            log_rec_head->reserved,
            log_rec_head->dbhost,
            log_rec_head->dbport,

            log_rec_head->username,
            log_rec_head->userhost,

            log_rec_head->queryDate,
            log_rec_head->queryTime,

            log_rec_head->sendQueryTime,
            log_rec_head->receiveResultTime,
            log_rec_head->sendResultTime,
            log_rec_head->totalTime,
            log_rec_head->affectedRows,
            log_rec_head->resultSize,

            log_rec_head->queryCount,
            log_rec_head->logType,
            log_rec_head->json,
            log_rec_head->GetSign);

        //strncpy(err_msg, sql_err_to_str(ret), 32);
        bzero(err_msg, ERR_MSG_LEN);
        snprintf(err_msg, ERR_MSG_LEN - 1, "%s", sql_err_to_str(ret));
        err_msg_length = strlen(err_msg);

        //intrude time
        struct tm tm_time;
        MYSQL_TIME  ts;
        char datetime[STR_LEN];
        bzero(datetime, STR_LEN);
        snprintf(datetime, STR_LEN, "%s %s", log_rec_head->queryDate, log_rec_head->queryTime);
        strptime(datetime, "%Y-%m-%d %H:%M:%S", &tm_time);

        /* supply the data to be sent in the ts structure */
        ts.year = tm_time.tm_year + 1900;
        ts.month = tm_time.tm_mon + 1;
        ts.day = tm_time.tm_mday;

        ts.hour = tm_time.tm_hour;
        ts.minute = tm_time.tm_min;
        ts.second = tm_time.tm_sec;
        //log_msg(LEVEL_INFO, "%d-%d-%d %d:%d:%d\n",
        //        ts.year, ts.month,  ts.day, ts.hour, ts.minute, ts.second);

        // db host, ip addr num 
        unsigned long int  db_host = 0;
        db_host = atol(log_rec_head->dbhost);       

        // db port
        unsigned int  db_port = 0;
        db_port = atoi(log_rec_head->dbport);       

        user_name_length = strlen(log_rec_head->username);
        user_host_length = strlen(log_rec_head->userhost);
        db_name_length = strlen(log_rec_head->dbname);
        table_name_length = strlen(log_rec_head->tbname);

        // query count
        unsigned int  query_count = 0;
        query_count = atoi(log_rec_head->queryCount);

        // log type
        log_type_length = strlen(log_rec_head->logType);

        //strncpy(sql_text, sql, (BUFFER_SIZE - 1));
        snprintf(sql_text, (BUFFER_SIZE - 1), "%s", sql);
        sql_length = strlen(sql_text);

        g_error_count++;

        memset(g_bind_err, 0, sizeof(g_bind_err));

        /* error type */
        g_bind_err[0].buffer_type = MYSQL_TYPE_STRING;
        g_bind_err[0].buffer = (char *)err_msg;
        g_bind_err[0].buffer_length = sizeof(err_msg);
        g_bind_err[0].is_null = 0;
        g_bind_err[0].length = &err_msg_length;

        /* intrude time */
        g_bind_err[1].buffer_type = MYSQL_TYPE_DATETIME;
        g_bind_err[1].buffer = (char *)&ts;
        g_bind_err[1].is_null = 0;
        g_bind_err[1].length = 0;

        //log_msg(LEVEL_INFO, " mysql_stmt_bind_param() failed for error sql\n");
        log_msg(LEVEL_INFO, "found new injection:%s\n", sql_ptr);
        log_msg(LEVEL_INFO, "\n");
    //log_msg(LEVEL_INFO, "%s %s %s/*## %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",
    //    log_rec_head->dbname,
    //    log_rec_head->tbname,
    //    log_rec_head->queryType,

    //    log_rec_head->reserved,
    //    log_rec_head->dbhost,
    //    log_rec_head->dbport,

    //    log_rec_head->username,
    //    log_rec_head->userhost,

    //    log_rec_head->queryDate,
    //    log_rec_head->queryTime,

    //    log_rec_head->sendQueryTime,
    //    log_rec_head->receiveResultTime,
    //    log_rec_head->sendResultTime,
    //    log_rec_head->totalTime,
    //    log_rec_head->affectedRows,
    //    log_rec_head->resultSize,

    //    log_rec_head->queryCount,
    //    log_rec_head->logType,
    //    log_rec_head->json,
    //    log_rec_head->GetSign);
        /*db host, ip number*/
        g_bind_err[2].buffer_type = MYSQL_TYPE_LONGLONG;
        g_bind_err[2].is_unsigned = 1;
        g_bind_err[2].buffer = (char *)&db_host;
        g_bind_err[2].is_null = 0;
        g_bind_err[2].length = 0;

        /* port */
        g_bind_err[3].buffer_type = MYSQL_TYPE_LONG;
        g_bind_err[3].is_unsigned = 1;
        g_bind_err[3].buffer = (char *)&db_port;
        g_bind_err[3].is_null = 0;
        g_bind_err[3].length = 0;

        /* user name */
        g_bind_err[4].buffer_type = MYSQL_TYPE_STRING;
        g_bind_err[4].buffer = (char *)log_rec_head->username;
        g_bind_err[4].buffer_length = sizeof(log_rec_head->username);
        g_bind_err[4].is_null = 0;
        g_bind_err[4].length = &user_name_length;

        /* user host */
        g_bind_err[5].buffer_type = MYSQL_TYPE_STRING;
        g_bind_err[5].buffer = (char *)log_rec_head->userhost;
        g_bind_err[5].buffer_length = sizeof(log_rec_head->userhost);
        g_bind_err[5].is_null = 0;
        g_bind_err[5].length = &user_host_length;

        /* db name */
        g_bind_err[6].buffer_type = MYSQL_TYPE_STRING;
        g_bind_err[6].buffer = (char *)log_rec_head->dbname;
        g_bind_err[6].buffer_length = sizeof(log_rec_head->dbname);
        g_bind_err[6].is_null = 0;
        g_bind_err[6].length = &db_name_length;

        /* table name */
        g_bind_err[7].buffer_type = MYSQL_TYPE_STRING;
        g_bind_err[7].buffer = (char *)log_rec_head->tbname;
        g_bind_err[7].buffer_length = sizeof(log_rec_head->tbname);
        g_bind_err[7].is_null = 0;
        g_bind_err[7].length = &table_name_length;

        /* query count*/
        g_bind_err[8].buffer_type = MYSQL_TYPE_LONG;
        g_bind_err[8].is_unsigned = 1;
        g_bind_err[8].buffer = (char *)&query_count;
        g_bind_err[8].is_null = 0;
        g_bind_err[8].length = 0;

        /* log type */
        g_bind_err[9].buffer_type = MYSQL_TYPE_STRING;
        g_bind_err[9].buffer = (char *)log_rec_head->logType;
        g_bind_err[9].buffer_length = sizeof(log_rec_head->logType);
        g_bind_err[9].is_null = 0;
        g_bind_err[9].length = &log_type_length;

        /* sql text */
        g_bind_err[10].buffer_type = MYSQL_TYPE_STRING;
        g_bind_err[10].buffer = (char *)sql_text;
        g_bind_err[10].buffer_length = sizeof(sql_text);
        g_bind_err[10].is_null = 0;
        g_bind_err[10].length = &sql_length;

        /* Bind the buffers */
        if (mysql_stmt_bind_param(g_stmt_err, g_bind_err))
        {
            log_msg(LEVEL_ERROR, " mysql_stmt_bind_param() failed for error sql\n");
            log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_err));
            exit(1);
        }

        if (clock_gettime(CLOCK_REALTIME, &start_time))
        {
            log_msg(LEVEL_INFO, "failed to get start time of inserting error sql, "
                    "time indicator might be incorrect.\n");
        }

        /* Execute the INSERT statement - 1*/
        if (mysql_stmt_execute(g_stmt_err))
        {
            log_msg(LEVEL_ERROR, " mysql_stmt_execute(), 1 failed for error sql\n");
            log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_err));
            exit(1);
        }

        // insert to the black list after stmt_executed.
        //g_hash_table_insert(g_hash_table_for_error_sql, &id, sql_temp);
        //inset 1 instead of sql_temp to save memory.
        g_hash_table_insert(g_hash_table_for_error_sql, &id, (char*)1);
        if (clock_gettime(CLOCK_REALTIME, &end_time))
        {
            log_msg(LEVEL_INFO, "failed to get end time of inserting error sql, "
                    "time indicator might be incorrect.\n");
        }

        g_time_insert += time_subtract(&end_time, &start_time);
    }

    mp_clear(g_parser_pool);

    //if (!(g_total_count % 1000))
    //if (!(g_total_count % 10))
    {
        mysql_commit(g_conn);
    }
Exit:
    if (fd)
    {
        close(fd);
    }
    if (fd_read)
    {
        close(fd_read);
    }
    mp_clear(g_parser_pool);
    close(client_socket);
    return;
}

int sql_is_general_log_header(char *line)
{
    if ((strstr(line, "Connect")   && strstr(line, "@")) ||
        strstr(line, "Close stmt") ||
        strstr(line, "Init DB") ||
        strstr(line, "Field List") ||
        strstr(line, "Prepare") ||
        strstr(line, "Execute") ||
        strstr(line, "Quit"))
    {
        return 1;
    }

    return 0;
}

int insert_exception(char* sql)
{
    int exception_param_count = 0;
    char *insert_exception_sql = NULL;
    unsigned long length[4];
    long int_data = 0;
    char          str_data1[STRING_SIZE];
    char          str_data2[STRING_SIZE];
    my_bool       is_null[3];
    my_bool       error[3];

    g_stmt_insert_exception = mysql_stmt_init(g_conn);
    if (!g_stmt_insert_exception)
    {
        log_msg(LEVEL_ERROR, "failed to create statement for the insert exception sqls: %s\n",
                mysql_error(g_conn));
        exit(1);
    }

    insert_exception_sql = "insert into exception_sql "
                        "(id, dbhost, sql_text, original_sql) "
                        "values "
                        "(?, ?, ?, ?)";

    if (mysql_stmt_prepare(g_stmt_insert_exception, 
                insert_exception_sql, strlen(insert_exception_sql)))
    {
        log_msg(LEVEL_ERROR, "failed to prepare statement for the exception sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    exception_param_count = mysql_stmt_param_count(g_stmt_insert_exception);
    if (exception_param_count != 4)
    {
        log_msg(LEVEL_INFO, "invalid parameter count returned by MySQL for the exception SQLs.\n");
        exit(1);
    }
    char sql_text[BUFFER_SIZE] = {'\0'};
    char sql_temp[BUFFER_SIZE*2] = {'\0'};
    unsigned long sql_len_normalized = 0;
    unsigned long sql_len_original = 0;
    unsigned int id = 0;
    char *db_host_ptr = strstr(sql, "###"); 
    if (db_host_ptr == NULL)
    {
        log_msg(LEVEL_ERROR, "no dbhost passed in. exit!\n");
        exit(1);
    }
    char db_host[STR_LEN];
    bzero(db_host, STR_LEN);
    snprintf(db_host, db_host_ptr - sql, "%s", sql);
    unsigned int int_db_host = 0;
    int_db_host = atoi(db_host);

    // for "###"
    db_host_ptr += 3;
    //strncpy(sql_text, sql, (BUFFER_SIZE - 1));
    //snprintf(sql_text, BUFFER_SIZE, "%s", sql);
    snprintf(sql_text, BUFFER_SIZE, "%s", db_host_ptr);
    sql_len_original = strlen(sql_text);

    (void)sql_normalize_en(sql_text, sql_len_original, &id, sql_temp);

    char new_id[STR_LEN];
    bzero(new_id, STR_LEN);
    snprintf(new_id, STR_LEN, "%s%u", db_host, id);

    // lookup the hash table first, if hits, then no need to parse the SQL.
    //if (!g_hash_table_lookup(g_hash_table_for_exception_sql, &id))
    if (!g_hash_table_lookup(g_hash_table_for_exception_sql, new_id))
    {
        sql_len_normalized = strlen(sql_temp);

        memset(g_bind_insert_exception, 0, sizeof(g_bind_insert_exception));

        g_bind_insert_exception[0].buffer_type = MYSQL_TYPE_LONG;
        g_bind_insert_exception[0].is_unsigned = 1;
        g_bind_insert_exception[0].buffer = (char *)&id;
        g_bind_insert_exception[0].is_null = 0;
        g_bind_insert_exception[0].length = 0;

        g_bind_insert_exception[1].buffer_type = MYSQL_TYPE_LONG;
        g_bind_insert_exception[1].is_unsigned = 1;
        g_bind_insert_exception[1].buffer = (char *)&int_db_host;
        g_bind_insert_exception[1].is_null = 0;
        g_bind_insert_exception[1].length = 0;

        g_bind_insert_exception[2].buffer_type = MYSQL_TYPE_STRING;
        g_bind_insert_exception[2].buffer = (char *)sql_temp;
        g_bind_insert_exception[2].buffer_length = sizeof(sql_temp);
        g_bind_insert_exception[2].is_null = 0;
        g_bind_insert_exception[2].length = &sql_len_normalized;

        g_bind_insert_exception[3].buffer_type = MYSQL_TYPE_STRING;
        g_bind_insert_exception[3].buffer = (char *)sql_text;
        g_bind_insert_exception[3].buffer_length = sizeof(sql_text);
        g_bind_insert_exception[3].is_null = 0;
        g_bind_insert_exception[3].length = &sql_len_original;

        if (mysql_stmt_bind_param(g_stmt_insert_exception, g_bind_insert_exception))
        {
            log_msg(LEVEL_ERROR, " mysql_stmt_bind_param() failed for insert exception sqls\n");
            log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_insert_exception));
        }

        if (mysql_stmt_execute(g_stmt_insert_exception))
        {
            log_msg(LEVEL_ERROR, " mysql_stmt_execute(), 1 failed for insert exception sqls\n");
            log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_insert_exception));
        }
        else
        {
            mysql_commit(g_conn);
            //g_hash_table_insert(g_hash_table_for_exception_sql, new_id, sql_temp);
            g_hash_table_insert(g_hash_table_for_exception_sql, new_id, (char*)1);
        }
    }
Exit:
    mp_clear(g_parser_pool);
    if (sql)
    {
        free(sql);
        sql = NULL;
    }
}

void sql_init()
{
    int err_param_count = 0;
    int normal_param_count = 0;
    int suspect_param_count = 0;

    char *insert_err_sql = NULL;
    char *insert_normal_sql = NULL;
    char *insert_suspect_sql = NULL;
    char *select_exception_sql = NULL;
    char *select_normal_sql = NULL;
    char *select_suspect_sql = NULL;
    char *select_error_sql = NULL;
    unsigned long length[4];
    long int_data = 0;
    int db_host = 0;
    char          str_data1[STRING_SIZE];
    char          str_data2[STRING_SIZE];
    my_bool       is_null[4];
    my_bool       error[4];

    g_parser_pool = mp_init(1024*1024*300);

    g_hash_table_for_normal_sql = NULL;
    g_hash_table_for_normal_sql_distinct = NULL;
    g_hash_table_for_error_sql = NULL;
    g_hash_table_for_suspect_sql = NULL;
    g_hash_table_for_exception_sql = NULL;
    g_sql_parser = sql_parser_init(g_parser_pool);

    sql_normalize_init();

    g_sql_file_count = 0;
    g_davinci_server_count = 0;

    /* Load the configuration parameters for connection to mysql. */
    if (!load_connect_config())
    {
        exit(1);
    }

    if (g_check_re && !sql_load_re())
    {
        log_msg(LEVEL_INFO, "failed to load regular expressions!\n");
        exit(1);
    }
    //sql_load_config();
    sql_set_sec_level(3, 1);

    // Debug
    // sql_dump_config();
    // dump_all_deny();
    struct sql_config_summary_t *summary = sql_get_config_summary();
    sql_format_config_summary(summary);

    struct sql_config_detail_t *detail = sql_get_config_detail();
    sql_format_config_detail(detail);

    dump_connect_parameters();

    // Connect to mysql
    g_conn = mysql_init(NULL);
    if (!mysql_real_connect(g_conn, g_host, g_user, g_password,
                            g_db, g_port, s_socket, 0))
    {
        log_msg(LEVEL_ERROR, "failed to connect: %s\n", mysql_error(g_conn));
        exit(1);
    }
    else
    {
        log_msg(LEVEL_INFO, "connected to mysql [%s]!\n", g_conn->server_version);
    }

    g_stmt_err = mysql_stmt_init(g_conn);
    if (!g_stmt_err)
    {
        log_msg(LEVEL_ERROR, "failed to create statement for the error sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    /*
    insert_err_sql = "insert into ids_event " 
                     "(alarm_type, intrude_time, port, user, srchost, "
                     "dbname, tblname, createtime, sql_text) "
                     "values "
                     "(?, ?, 5008, user(), @@hostname, "
                     "database(), '', current_timestamp, ?)";
                     */
    insert_err_sql = "insert into ids_event " 
                     "(alarm_type, intrude_time, dbhost, port, user, srchost, "
                     "dbname, tblname, querycount, logtype, sql_text) "
                     "values "
                     "(?, ?, ?, ?, ?, ?, "
                     "?, ?, ?, ?, ?)";
    /*
    insert_err_sql = "insert into ids_event " 
                     "(alarm_type, intrude_time, dbhost , port, user, srchost, "
                     "dbname, tblname, logtype, sql_text) "
                     "values "
                     "(?, ?, ?, ?, ?,?, "
                     "?, ?, ?, ?)";
*/

    if (mysql_stmt_prepare(g_stmt_err, insert_err_sql, strlen(insert_err_sql)))
    {
        log_msg(LEVEL_ERROR, "failed to prepare statement for the error sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    err_param_count = mysql_stmt_param_count(g_stmt_err);
    if (err_param_count != 11)
    {
        log_msg(LEVEL_INFO, "invalid parameter count returned by MySQL for the error SQLs\n");
        exit(1);
    }

    g_stmt_normal = mysql_stmt_init(g_conn);
    if (!g_stmt_normal)
    {
        log_msg(LEVEL_ERROR, "failed to create statement for the normal sqls: %s\n",
                mysql_error(g_conn));
        exit(1);
    }

    /*
    insert_normal_sql = "insert into normal_sql "
                        "(id, dbhost, sql_text, original_sql) "
                        "values "
                        "(?, ?, ?, ?)";
                        */
    // change the schema of normal_sql, remove dbhost column, using id as primary key
    insert_normal_sql = "insert into normal_sql "
                        "(id, sql_text, original_sql) "
                        "values "
                        "(?, ?, ?)";

    if (mysql_stmt_prepare(g_stmt_normal, insert_normal_sql, strlen(insert_normal_sql)))
    {
        log_msg(LEVEL_ERROR, "failed to prepare statement for the normal sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    normal_param_count = mysql_stmt_param_count(g_stmt_normal);
    if (normal_param_count != 3)
    {
        log_msg(LEVEL_INFO, "invalid parameter count returned by MySQL for the normal SQLs.\n");
        exit(1);
    }

    g_stmt_suspect = mysql_stmt_init(g_conn);
    if (!g_stmt_suspect)
    {
        log_msg(LEVEL_ERROR, "failed to create statement for the suspect sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    /*
    insert_suspect_sql = "insert into suspect_sql "
                         "(id, dbhost, intrude_time, sql_text, original_sql) "
                         "values "
                         "(?, ?, current_timestamp, ?, ?)";
                         */
    insert_suspect_sql = "insert into suspect_sql_new " 
                     "(id, alarm_type, intrude_time, dbhost, port, user, srchost, "
                     "dbname, tblname, querycount, logtype, sql_text) "
                     "values "
                     "(?, ?, ?, ?, ?, ?, ?, "
                     "?, ?, ?, ?, ?)";

    if (mysql_stmt_prepare(g_stmt_suspect, insert_suspect_sql, strlen(insert_suspect_sql)))
    {
        log_msg(LEVEL_ERROR, "failed to prepare statement for the suspect sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    suspect_param_count = mysql_stmt_param_count(g_stmt_suspect);
    if (suspect_param_count != 12)
    {
        log_msg(LEVEL_INFO, "invalid parameter count returned by MySQL for the suspect SQLs.\n");
        exit(1);
    }

    // we need to create the hash map before select exception SQL excuted.
    //if (!(g_hash_table_for_exception_sql = g_hash_table_new(g_int_hash, g_int_equal)))
    if (!(g_hash_table_for_exception_sql = g_hash_table_new(g_str_hash, g_str_equal)))
    {
        log_msg(LEVEL_INFO, "Error when initializing the exception hash table!\n");
        exit(1);
    }

    g_stmt_select_exception = mysql_stmt_init(g_conn);
    if (!g_stmt_select_exception)
    {
        log_msg(LEVEL_ERROR, "failed to create statement for the select exception sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    select_exception_sql = "SELECT id, dbhost, sql_text, original_sql FROM exception_sql";

    if (mysql_stmt_prepare(g_stmt_select_exception, 
                select_exception_sql, strlen(select_exception_sql)))
    {
        log_msg(LEVEL_ERROR, "failed to prepare statement for the select exception sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    /* Execute the SELECT query */
    if (mysql_stmt_execute(g_stmt_select_exception))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_execute(), failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_exception));
        exit(1);
    }

    /* Bind the result buffers for all 4 columns before fetching them */

    memset(g_bind_select_exception, 0, sizeof(g_bind_select_exception));

    /* INTEGER COLUMN */
    g_bind_select_exception[0].buffer_type = MYSQL_TYPE_LONGLONG;
    g_bind_select_exception[0].buffer = (char *)&int_data;
    g_bind_select_exception[0].is_null = &is_null[0];
    g_bind_select_exception[0].length = &length[0];
    g_bind_select_exception[0].error = &error[0];

    /* INTEGER COLUMN */
    g_bind_select_exception[1].buffer_type = MYSQL_TYPE_LONG;
    g_bind_select_exception[1].buffer = (char *)&db_host;
    g_bind_select_exception[1].is_null = &is_null[1];
    g_bind_select_exception[1].length = &length[1];
    g_bind_select_exception[1].error = &error[1];

    /* STRING COLUMN */
    g_bind_select_exception[2].buffer_type = MYSQL_TYPE_STRING;
    g_bind_select_exception[2].buffer = (char *)str_data1;
    g_bind_select_exception[2].buffer_length = STRING_SIZE;
    g_bind_select_exception[2].is_null = &is_null[2];
    g_bind_select_exception[2].length = &length[2];
    g_bind_select_exception[2].error = &error[2];

    /* STRING COLUMN */
    g_bind_select_exception[3].buffer_type = MYSQL_TYPE_STRING;
    g_bind_select_exception[3].buffer = (char *)str_data2;
    g_bind_select_exception[3].buffer_length = STRING_SIZE;
    g_bind_select_exception[3].is_null = &is_null[3];
    g_bind_select_exception[3].length = &length[3];
    g_bind_select_exception[3].error = &error[3];

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(g_stmt_select_exception, g_bind_select_exception))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_bind_result() failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_exception));
        exit(1);
    }

    int true_value = 1;
    //mysql_stmt_attr_set(g_stmt_select, STMT_ATTR_UPDATE_MAX_LENGTH, (void*) &true_value);    
    /* Now buffer all results to client (optional step) */
    if (mysql_stmt_store_result(g_stmt_select_exception))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_store_result() failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_exception));
        exit(1);
    }

    /* Fetch all rows */
    int row_count = 0;
    log_msg(LEVEL_INFO, "Fetching exception_sql results ...\n");
    int rc_fetch = 0;
    while ((rc_fetch = mysql_stmt_fetch(g_stmt_select_exception)) == 0)
    {
        row_count++;
        char *key_in_hash = (char *)malloc(STR_LEN);
        /*
        // column 2 
        fprintf(stdout, "  row %d\n", row_count);

        // column 1
        fprintf(stdout, "   column1 (integer)  : ");
        if (is_null[0])
          fprintf(stdout, " NULL\n");
        else
          fprintf(stdout, " %u(%ld)\n", int_data, length[0]);

        // column 1
        fprintf(stdout, "   column2 (integer)  : ");
        if (is_null[1])
          fprintf(stdout, " NULL\n");
        else
          fprintf(stdout, " %u(%ld)\n", db_host, length[1]);

        // column 2 
        fprintf(stdout, "   column2 (string)   : ");
        if (is_null[1])
          fprintf(stdout, " NULL\n");
        else
          fprintf(stdout, " %s(%ld)\n", str_data1, length[1]);

        // column 3 
        fprintf(stdout, "   column3 (string)   : ");
        if (is_null[2])
          fprintf(stdout, " NULL\n");
        else
          fprintf(stdout, " %s(%ld)\n", str_data2, length[2]);
        */
        bzero(key_in_hash, STR_LEN);
        //char str_db_host[STR_LEN];
        //char str_int_data[STR_LEN];
        //bzero(str_db_host, STR_LEN);
        //bzero(str_int_data, STR_LEN);
        //itoa(db_host, str_db_host, STR_LEN);
        //itoa(int_data, str_int_data, STR_LEN);
        snprintf(key_in_hash, STR_LEN, "%u%u", db_host, int_data);
        //*key_in_hash = db_host;
        //*key_in_hash<<32;
        //*key_in_hash += int_data;
        //fprintf(stdout, "*key_in_hash is: %s\n", key_in_hash);
        //char* value_in_hash = (char*)malloc(BUFFER_SIZE);
        //bzero(value_in_hash, BUFFER_SIZE);
        //snprintf(value_in_hash, BUFFER_SIZE, "%s", str_data1);
        // insert the md5 value and sql text to the hash map.
        //g_hash_table_insert(g_hash_table_for_exception_sql, key_in_hash, value_in_hash);
        g_hash_table_insert(g_hash_table_for_exception_sql, key_in_hash, (char*)1);
    }
    if (rc_fetch)
    {
        log_msg(LEVEL_INFO, "[INFO]rc_fetch is:%d\n", rc_fetch);
        log_msg(LEVEL_INFO, "[INFO] %s\n", mysql_stmt_error(g_stmt_select_exception));
    }
    /* Validate rows fetched */
    log_msg(LEVEL_INFO, " total rows fetched: %d\n", row_count);

    /* Close the statement */
    if (mysql_stmt_close(g_stmt_select_exception))
    {
        log_msg(LEVEL_ERROR, " failed while closing the statement\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_exception));
        exit(1);
    }

    //log_msg(LEVEL_INFO, "Prepare statement successful\n");

    if (!(g_hash_table_for_normal_sql = g_hash_table_new(g_str_hash, g_str_equal)))
    {
        log_msg(LEVEL_INFO, "Error when initializing the white list hash table!\n");
        exit(1);
    }
    //if (!(g_hash_table_for_suspect_sql = g_hash_table_new(g_int_hash, g_int_equal)))
    if (!(g_hash_table_for_normal_sql_distinct = g_hash_table_new(g_int_hash, g_int_equal)))
    {
        log_msg(LEVEL_INFO, "Error when initializing the white list distinct hash table!\n");
        exit(1);
    }
    g_stmt_select_normal = mysql_stmt_init(g_conn);
    if (!g_stmt_select_normal)
    {
        log_msg(LEVEL_ERROR, "failed to create statement for the select normal sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    //select_normal_sql = "SELECT id, dbhost, sql_text, original_sql FROM normal_sql";
    //select_normal_sql = "SELECT id, dbhost FROM normal_sql";
    select_normal_sql = "SELECT id FROM normal_sql";

    if (mysql_stmt_prepare(g_stmt_select_normal, select_normal_sql, strlen(select_normal_sql)))
    {
        log_msg(LEVEL_ERROR, "failed to prepare statement for the select normal sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    /* Execute the SELECT query */
    if (mysql_stmt_execute(g_stmt_select_normal))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_execute(), failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_normal));
        exit(1);
    }

    /* Bind the result buffers for all 4 columns before fetching them */

    memset(g_bind_select_normal, 0, sizeof(g_bind_select_normal));

    /* INTEGER COLUMN */
    // just need the md5 id for the sql.
    g_bind_select_normal[0].buffer_type = MYSQL_TYPE_LONGLONG;
    g_bind_select_normal[0].buffer = (char *)&int_data;
    g_bind_select_normal[0].is_null = &is_null[0];
    g_bind_select_normal[0].length = &length[0];
    g_bind_select_normal[0].error = &error[0];

    /* INTEGER COLUMN */
    /*
    g_bind_select_normal[1].buffer_type = MYSQL_TYPE_LONG;
    g_bind_select_normal[1].buffer = (char *)&db_host;
    g_bind_select_normal[1].is_null = &is_null[1];
    g_bind_select_normal[1].length = &length[1];
    g_bind_select_normal[1].error = &error[1];
    */

    ///* STRING COLUMN */
    //g_bind_select_normal[2].buffer_type = MYSQL_TYPE_STRING;
    //g_bind_select_normal[2].buffer = (char *)str_data1;
    //g_bind_select_normal[2].buffer_length = STRING_SIZE;
    //g_bind_select_normal[2].is_null = &is_null[2];
    //g_bind_select_normal[2].length = &length[2];
    //g_bind_select_normal[2].error = &error[2];

    ///* STRING COLUMN */
    //g_bind_select_normal[3].buffer_type = MYSQL_TYPE_STRING;
    //g_bind_select_normal[3].buffer = (char *)str_data2;
    //g_bind_select_normal[3].buffer_length = STRING_SIZE;
    //g_bind_select_normal[3].is_null = &is_null[3];
    //g_bind_select_normal[3].length = &length[3];
    //g_bind_select_normal[3].error = &error[3];

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(g_stmt_select_normal, g_bind_select_normal))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_bind_result() failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_normal));
        exit(1);
    }

    //int true_value = 1;
    //mysql_stmt_attr_set(g_stmt_select, STMT_ATTR_UPDATE_MAX_LENGTH, (void*) &true_value);    
    /* Now buffer all results to client (optional step) */
    if (mysql_stmt_store_result(g_stmt_select_normal))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_store_result() failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_normal));
        exit(1);
    }

    /* Fetch all rows */
    row_count = 0;
    log_msg(LEVEL_INFO, "Fetching normal_sql results ...\n");
    rc_fetch = 0;
    while ((rc_fetch = mysql_stmt_fetch(g_stmt_select_normal)) == 0)
    {
        row_count++;
        /*
        char *key_in_hash = (char *)malloc(STR_LEN);
        bzero(key_in_hash, STR_LEN);
        snprintf(key_in_hash, STR_LEN, "%u%u", db_host, int_data);
        char* value_in_hash = NULL;
        */
        //char* value_in_hash = (char*)malloc(BUFFER_SIZE);
        //bzero(value_in_hash, BUFFER_SIZE);
        //snprintf(value_in_hash, BUFFER_SIZE, "%s", str_data1);
        // insert the md5 value and sql text to the hash map.
        //g_hash_table_insert(g_hash_table_for_normal_sql, key_in_hash, value_in_hash);
        //g_hash_table_insert(g_hash_table_for_normal_sql, key_in_hash, (char*)1);

        /*
        char sql_text[BUFFER_SIZE] = {'\0'};
        char sql_temp[BUFFER_SIZE] = {'\0'}; 
        unsigned long sql_len_normalized = 0;
        unsigned long sql_len_original = 0;
        unsigned int id = 0; 

        //strncpy(sql_text, sql, (BUFFER_SIZE - 1));
        snprintf(sql_text, BUFFER_SIZE, "%s", str_data1);
        sql_len_original = strlen(sql_text);

        (void)sql_normalize_en(sql_text, sql_len_original, &id, sql_temp);
        unsigned int *key_in_hash_distinct = (unsigned int *)malloc(sizeof(unsigned int));
        *key_in_hash_distinct = id;
        */
        // just insert the md5 id of SQL to the hash map.
        unsigned int *key_in_hash_distinct = (unsigned int *)malloc(sizeof(unsigned int));
        *key_in_hash_distinct = int_data;
        g_hash_table_insert(g_hash_table_for_normal_sql_distinct, key_in_hash_distinct, (char*)1);
    }
    if (rc_fetch)
    {
        log_msg(LEVEL_INFO, "[INFO]rc_fetch is:%d\n", rc_fetch);
        log_msg(LEVEL_INFO, "[INFO] %s\n", mysql_stmt_error(g_stmt_select_normal));
    }
    /* Validate rows fetched */
    log_msg(LEVEL_INFO, " total rows fetched: %d\n", row_count);

    /* Close the statement */
    if (mysql_stmt_close(g_stmt_select_normal))
    {
        log_msg(LEVEL_ERROR, " failed while closing the statement\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_normal));
        exit(1);
    }

    // initilize the suspect hash table from table suspect_sql.
    //if (!(g_hash_table_for_normal_sql = g_hash_table_new(g_str_hash, g_str_equal)))
    //if (!(g_hash_table_for_suspect_sql = g_hash_table_new(g_int_hash, g_int_equal)))
    if (!(g_hash_table_for_suspect_sql = g_hash_table_new(g_str_hash, g_str_equal)))
    {
        log_msg(LEVEL_INFO, "Error when initializing the suspect hash table!\n");
        exit(1);
    }
    g_stmt_select_suspect = mysql_stmt_init(g_conn);
    if (!g_stmt_select_suspect)
    {
        log_msg(LEVEL_ERROR, "failed to create statement for the select suspect sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    //select_suspect_sql = "SELECT id, dbhost FROM suspect_sql";
    select_suspect_sql = "SELECT id, dbhost FROM suspect_sql_new";

    if (mysql_stmt_prepare(g_stmt_select_suspect, select_suspect_sql, strlen(select_suspect_sql)))
    {
        log_msg(LEVEL_ERROR, "failed to prepare statement for the select suspect sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    /* Execute the SELECT query */
    if (mysql_stmt_execute(g_stmt_select_suspect))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_execute(), failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_suspect));
        exit(1);
    }

    /* Bind the result buffers for all 2 columns before fetching them */

    memset(g_bind_select_suspect, 0, sizeof(g_bind_select_suspect));

    /* INTEGER COLUMN */
    g_bind_select_suspect[0].buffer_type = MYSQL_TYPE_LONGLONG;
    g_bind_select_suspect[0].buffer = (char *)&int_data;
    g_bind_select_suspect[0].is_null = &is_null[0];
    g_bind_select_suspect[0].length = &length[0];
    g_bind_select_suspect[0].error = &error[0];

    /* INTEGER COLUMN */
    g_bind_select_suspect[1].buffer_type = MYSQL_TYPE_LONG;
    g_bind_select_suspect[1].buffer = (char *)&db_host;
    g_bind_select_suspect[1].is_null = &is_null[1];
    g_bind_select_suspect[1].length = &length[1];
    g_bind_select_suspect[1].error = &error[1];

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(g_stmt_select_suspect, g_bind_select_suspect))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_bind_result() failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_suspect));
        exit(1);
    }

    //int true_value = 1;
    //mysql_stmt_attr_set(g_stmt_select, STMT_ATTR_UPDATE_MAX_LENGTH, (void*) &true_value);    
    /* Now buffer all results to client (optional step) */
    if (mysql_stmt_store_result(g_stmt_select_suspect))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_store_result() failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_suspect));
        exit(1);
    }

    /* Fetch all rows */
    row_count = 0;
    log_msg(LEVEL_INFO, "Fetching suspect_sql results ...\n");
    rc_fetch = 0;
    while ((rc_fetch = mysql_stmt_fetch(g_stmt_select_suspect)) == 0)
    {
        row_count++;
        char *key_in_hash = (char *)malloc(STR_LEN);
        bzero(key_in_hash, STR_LEN);
        snprintf(key_in_hash, STR_LEN, "%u%u", db_host, int_data);
        //unsigned int *key_in_hash = (unsigned int *)malloc(sizeof(unsigned int));
        //*key_in_hash = int_data;

        // insert NULL to the hash table, since we never use the value, we just lookup
        // the key.
        g_hash_table_insert(g_hash_table_for_suspect_sql, key_in_hash, (char*)1);
    }
    if (rc_fetch)
    {
        log_msg(LEVEL_INFO, "rc_fetch is:%d\n", rc_fetch);
        log_msg(LEVEL_INFO, "%s\n", mysql_stmt_error(g_stmt_select_suspect));
    }
    /* Validate rows fetched */
    log_msg(LEVEL_INFO, " total rows fetched: %d\n", row_count);

    /* Close the statement */
    if (mysql_stmt_close(g_stmt_select_suspect))
    {
        log_msg(LEVEL_ERROR, " failed while closing the statement\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_suspect));
        exit(1);
    }

    // initialize the error hash map from the db table ids_event
    if (!(g_hash_table_for_error_sql = g_hash_table_new(g_int_hash, g_int_equal)))
    {
        log_msg(LEVEL_INFO, "Error when initializing the error hash table!\n");
        exit(1);
    }
    g_stmt_select_error = mysql_stmt_init(g_conn);
    if (!g_stmt_select_error)
    {
        log_msg(LEVEL_ERROR, "failed to create statement for the select error sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    select_error_sql = "SELECT id, sql_text FROM ids_event";

    if (mysql_stmt_prepare(g_stmt_select_error, select_error_sql, strlen(select_error_sql)))
    {
        log_msg(LEVEL_ERROR, "failed to prepare statement for the select error sqls: %s\n", 
                mysql_error(g_conn));
        exit(1);
    }

    /* Execute the SELECT query */
    if (mysql_stmt_execute(g_stmt_select_error))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_execute(), failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_error));
        exit(1);
    }

    /* Bind the result buffers for all 2 columns before fetching them */

    memset(g_bind_select_error, 0, sizeof(g_bind_select_error));

    /* INTEGER COLUMN */
    g_bind_select_error[0].buffer_type = MYSQL_TYPE_LONGLONG;
    g_bind_select_error[0].buffer = (char *)&int_data;
    g_bind_select_error[0].is_null = &is_null[0];
    g_bind_select_error[0].length = &length[0];
    g_bind_select_error[0].error = &error[0];

    /* STRING COLUMN */
    g_bind_select_error[1].buffer_type = MYSQL_TYPE_STRING;
    g_bind_select_error[1].buffer = (char *)str_data1;
    g_bind_select_error[1].buffer_length = STRING_SIZE;
    g_bind_select_error[1].is_null = &is_null[1];
    g_bind_select_error[1].length = &length[1];
    g_bind_select_error[1].error = &error[1];

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(g_stmt_select_error, g_bind_select_error))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_bind_result() failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_error));
        exit(1);
    }

    //int true_value = 1;
    //mysql_stmt_attr_set(g_stmt_select, STMT_ATTR_UPDATE_MAX_LENGTH, (void*) &true_value);    
    /* Now buffer all results to client (optional step) */
    if (mysql_stmt_store_result(g_stmt_select_error))
    {
        log_msg(LEVEL_ERROR, " mysql_stmt_store_result() failed\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_error));
        exit(1);
    }

    /* Fetch all rows */
    row_count = 0;
    log_msg(LEVEL_INFO, "Fetching ids_event results ...\n");
    rc_fetch = 0;
    while ((rc_fetch = mysql_stmt_fetch(g_stmt_select_error)) == 0)
    {
        row_count++;
        char sql_text[BUFFER_SIZE] = {'\0'};
        char sql_temp[BUFFER_SIZE] = {'\0'}; 
        unsigned long sql_len_normalized = 0;
        unsigned long sql_len_original = 0;
        unsigned int id = 0; 

        //strncpy(sql_text, sql, (BUFFER_SIZE - 1));
        snprintf(sql_text, BUFFER_SIZE, "%s", str_data1);
        sql_len_original = strlen(sql_text);

        (void)sql_normalize_en(sql_text, sql_len_original, &id, sql_temp);

        unsigned int *key_in_hash = (unsigned int *)malloc(sizeof(unsigned int));
        *key_in_hash = id;

        //char* value_in_hash = (char*)malloc(BUFFER_SIZE);
        char* value_in_hash = NULL;
        // insert NULL to the hash table, since we never use the value, we just lookup
        // the key.
        //snprintf(value_in_hash, BUFFER_SIZE, "%s", str_data1);
        // insert the md5 value and sql text to the hash map.
        //log_msg(LEVEL_INFO, "key_in_hash is:%u\n", *key_in_hash);
        //g_hash_table_insert(g_hash_table_for_error_sql, key_in_hash, value_in_hash);
        g_hash_table_insert(g_hash_table_for_error_sql, key_in_hash, (char*)1);
    }
    if (rc_fetch)
    {
        log_msg(LEVEL_INFO, "rc_fetch is:%d\n", rc_fetch);
        log_msg(LEVEL_INFO, "%s\n", mysql_stmt_error(g_stmt_select_error));
    }
    /* Validate rows fetched */
    log_msg(LEVEL_INFO, " total rows fetched: %d\n", row_count);

    /* Close the statement */
    if (mysql_stmt_close(g_stmt_select_error))
    {
        log_msg(LEVEL_ERROR, " failed while closing the statement\n");
        log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_select_error));
        exit(1);
    }

    log_msg(LEVEL_INFO, "Prepare statement successful\n");

    g_total_count  = 0;
    g_error_count  = 0;
    g_normal_count = 0;
    g_re_sql_count = 0;

    g_total_time         = 0;
    g_time_read_file     = 0;
    g_total_check_time   = 0;
    g_normalization_time = 0;
    g_time_check_normal  = 0;
    g_time_check_error   = 0;
    g_time_check_suspect = 0;
    g_time_insert        = 0;
    g_time_re            = 0;
}

void sql_init2()
{
    int i = 0;
    for (i = 0; i < g_deny_config_error_count; i++)
    {
        char message[1024] = { '\0' };
        format_config_error(&g_deny_config_error_list[i],
                            message, sizeof(message));

        log_msg(LEVEL_INFO, "config error: %s\n", message);
    }

    if (mysql_autocommit(g_conn, 0))
    {
        log_msg(LEVEL_ERROR, "failed to turn off autocommit: %s\n", mysql_error(g_conn));
        exit(1);
    }
}

void sql_cleanup()
{
    mysql_commit(g_conn);

    // Cleanup 
    sql_parser_free(g_sql_parser);
    mp_free(g_parser_pool);

    /* cleanup for normalization */
    sql_normalize_destroy();

    if (g_stmt_err)
    {
        mysql_stmt_close(g_stmt_err);
    }

    if (g_stmt_normal)
    {
        mysql_stmt_close(g_stmt_normal);
    }

    if (g_stmt_suspect)
    {
        mysql_stmt_close(g_stmt_suspect);
    }

    if (!g_conn)
    {
        mysql_close(g_conn);
    }

    if (!g_hash_table_for_normal_sql)
    {
        g_hash_table_destroy(g_hash_table_for_normal_sql);
    }
    if (!g_hash_table_for_normal_sql_distinct)
    {
        g_hash_table_destroy(g_hash_table_for_normal_sql_distinct);
    }

    if (!g_hash_table_for_error_sql)
    {
        g_hash_table_destroy(g_hash_table_for_error_sql);
    }

    if (!g_hash_table_for_suspect_sql)
    {
        g_hash_table_destroy(g_hash_table_for_suspect_sql);
    }

    if (!g_hash_table_for_exception_sql)
    {
        g_hash_table_destroy(g_hash_table_for_exception_sql);
    }

    sql_re_cleanup();
}

// 1 - success
// 0 - failed
int load_connect_config()
{
    bzero(g_davinci_servers, 128*256);
    struct  stat st;
    FILE   *fp = NULL;
    char   *line = NULL; 
    size_t  len = 0;
    ssize_t bytes_read = 0;
    int line_count = 0;

    // Open the config file
    if(stat(g_config_file, &st))
    {
        return 0;
    }

    fp = fopen(g_config_file, "r"); // read only
    if (fp == NULL)
    {
        return 0;
    }

    while ((bytes_read = getline(&line, &len, fp)) != -1)
    {
        unsigned int pos = 0;
        int parameter_done = 0;
        char parameter[256] = {'\0'};
        char value[256] = {'\0'};

        line_count++;

        for (; pos < strlen(line); pos++)
        {
            if (line[pos] == '#') /* comments */
            {
                break;
            }

            if (line[pos] == ' ' || line[pos] == '\t' || line[pos] == '\n')
            {
                continue;
            }

            if(line[pos] == '=')
            {
                parameter_done = 1;
                continue;
            }

            if(!parameter_done)
            {
                parameter[strlen(parameter)] = line[pos];
            }
            else
            {
                value[strlen(value)] = line[pos];
            }

            if(strlen(parameter) == 256 || strlen(value) == 256)
            {
                log_msg(LEVEL_INFO, "configure parameter length exceeds limits, at line %d\n",
                       line_count);
                break;
            }
        }

        if (!strlen(parameter) || !strlen(value))
        {
            continue;
        }

        set_parameter(parameter, value);
    }

    // Cleanup
    fclose(fp);

    if (line)
    {
        free(line);
    }

    return 1;
}

void set_parameter (char *parameter, char *value)
{
    if (!parameter || strlen(parameter) > 256 ||
        !value     || strlen(value) > 256)
    {
        return;
    }

    if (!strcmp(parameter, "host"))
    {
        //strcpy(host, value);
        bzero(g_host, 64);
        snprintf(g_host, 63, "%s", value);
    }
    else if (!strcmp(parameter, "port"))
    {
        g_port = atoi(value);
    }
    else if (!strcmp(parameter, "db"))
    {
        //strcpy(db, value);
        bzero(g_db, 32);
        snprintf(g_db, 31, "%s", value);
    }
    else if (!strcmp(parameter, "user"))
    {
        //strcpy(user, value);
        bzero(g_user, 32);
        snprintf(g_user, 31, "%s", value);
    }
    else if (!strcmp(parameter, "password"))
    {
        //strcpy(password, value);
        bzero(g_password, 32);
        snprintf(g_password, 31, "%s", value);
    }
    else if (!strcmp(parameter, "socket"))
    {
        //strcpy(socket, value);
        bzero(s_socket, 256);
        snprintf(s_socket, 255, "%s", value);
    }
    else if (!strcmp(parameter, "sql_file"))
    {
        if (g_sql_file_count == 128)
        {
            log_msg(LEVEL_INFO, 
                    "sql file count exceeds the max file count(128), ignore new files.\n");
            return;
        }

        bzero(g_sql_file[g_sql_file_count], 128);
        snprintf(g_sql_file[g_sql_file_count], 127, "%s", value);
        g_sql_file_count++;
    }
    else if (!strcmp(parameter, "davinci_server"))
    {
        if (g_davinci_server_count == 128)
        {
            log_msg(LEVEL_INFO, 
                    "davinci server count exceeds the max file count(128), ignore new servers.\n");
            return;
        }

        bzero(g_davinci_servers[g_davinci_server_count], 128);
        snprintf(g_davinci_servers[g_davinci_server_count], 127, "%s", value);
        g_davinci_server_count++;
    }
    else if (!strcmp(parameter, "sql_file_format"))
    {
        //strcpy(sql_file_format, value);
        bzero(g_sql_file_format, 16);
        snprintf(g_sql_file_format, 15, "%s", value);
    }
    else if (!strcmp(parameter, "check_re"))
    {
        if (value[0] == '0')
        {
            g_check_re = 0;
        }
        else if (value[0] == '1')
        {
            g_check_re = 1;
        }
        else
        {
            log_msg(LEVEL_INFO, "unkown value of check_re %s, using default.\n", value);
            g_check_re = 0;
        }
    }
    else if (!strcmp(parameter, "g_machine_learning_on"))
    {
        if (value[0] == '0')
        {
            g_machine_learning_on = 0;
        }
        else if (value[0] == '1')
        {
            g_machine_learning_on = 1;
        }
        else
        {
            log_msg(LEVEL_INFO, 
                    "unkown value of g_machine_learning_on %s, using default.\n", value);
            g_machine_learning_on = 0;
        }
    }
    else if (!strcmp(parameter, "ids_server_port"))
    {
        g_ids_server_port = atoi(value);
        log_msg(LEVEL_INFO, "ids_server_port : %d .\n", g_ids_server_port); 
    }
    else if (!strcmp(parameter, "ml_server_ip"))
    {
        bzero(g_ml_server_ip, 16);
        snprintf(g_ml_server_ip, 16, "%s", value);
        log_msg(LEVEL_INFO, "ml_server_ip : %s .\n", g_ml_server_ip);
    }
    else if (!strcmp(parameter, "ml_server_port"))
    {
        g_ml_server_port = atoi(value);
        log_msg(LEVEL_INFO, "ml_server_port : %d .\n", g_ml_server_port);
    }
    else
    {
        log_msg(LEVEL_INFO, "unrecognized parameter: [%s]\n", parameter);
    }
}

void dump_connect_parameters()
{
    int i = 0;
    log_msg(LEVEL_INFO, "Dumping connection parameters....\n");
    log_msg(LEVEL_INFO, "=========================================================\n");

    log_msg(LEVEL_INFO, "host:     [%s]\n", g_host);
    log_msg(LEVEL_INFO, "port:     [%d]\n", g_port);
    log_msg(LEVEL_INFO, "db:       [%s]\n", g_db);
    log_msg(LEVEL_INFO, "user:     [%s]\n", g_user);
    log_msg(LEVEL_INFO, "password: [%s]\n", g_password);
    log_msg(LEVEL_INFO, "socket:   [%s]\n", s_socket);
    log_msg(LEVEL_INFO, "file format[%s]\n", g_sql_file_format);
    log_msg(LEVEL_INFO, "check_re: [%d]\n", g_check_re);
    log_msg(LEVEL_INFO, "machine_learning: [%d]\n", g_machine_learning_on);
    log_msg(LEVEL_INFO, "file count[%d]\n", g_sql_file_count);

    for (i = 0; i < g_sql_file_count; i++)
    {
        log_msg(LEVEL_INFO, "sql file: [%s]\n", g_sql_file[i]);
    }

    log_msg(LEVEL_INFO, "Dumping connection parameters completed.\n");
    log_msg(LEVEL_INFO, "=========================================================\n");
}

void print_report()
{
    unsigned int sql_count_whitelist = g_hash_table_size(g_hash_table_for_normal_sql); 
    unsigned int sql_count_blacklist = g_hash_table_size(g_hash_table_for_error_sql); 
    unsigned int sql_count_suspect = g_hash_table_size(g_hash_table_for_suspect_sql);
    unsigned int sql_count_exception = g_hash_table_size(g_hash_table_for_exception_sql);

    log_msg(LEVEL_INFO, "Dumping report.......\n");
    log_msg(LEVEL_INFO, "=========================================================\n");

    log_msg(LEVEL_INFO, "total sql count:             %d\n", g_total_count);
    log_msg(LEVEL_INFO, "total normal sql count:      %d\n", g_normal_count);
    log_msg(LEVEL_INFO, "total count in whitelist:    %d\n", sql_count_whitelist);
    log_msg(LEVEL_INFO, "total count in blacklist:    %d\n", sql_count_blacklist);
    log_msg(LEVEL_INFO, "total count in suspectlist:  %d\n", sql_count_suspect);
    log_msg(LEVEL_INFO, "total count in exception list:  %d\n", sql_count_exception);
    log_msg(LEVEL_INFO, "total error sql count:       %d\n", g_error_count);
    log_msg(LEVEL_INFO, "total insert into DB:        %d\n", 
           g_error_count + sql_count_whitelist + sql_count_suspect);
    log_msg(LEVEL_INFO, "total sql checked with re:   %d\n", g_re_sql_count);

    if (g_total_count)
    {
        log_msg(LEVEL_INFO, "pencent of normal sql:       %d\n", 
                g_normal_count * 100 / g_total_count);
        log_msg(LEVEL_INFO, "pencent of error sql:        %d\n", 
                g_error_count * 100 / g_total_count);
    }

    log_msg(LEVEL_INFO, "total time(nanoseconds):     %lu\n", g_total_time);
    log_msg(LEVEL_INFO, "total time read file:        %lu\n", g_time_read_file);
    log_msg(LEVEL_INFO, "total check time:            %lu\n", g_total_check_time);
    log_msg(LEVEL_INFO, "total normalization time:    %lu\n", g_normalization_time);
    log_msg(LEVEL_INFO, "total time check normal sql: %lu\n", g_time_check_normal);
    log_msg(LEVEL_INFO, "total time check error sql:  %lu\n", g_time_check_error);
    log_msg(LEVEL_INFO, "total time check suspect sql:%lu\n", g_time_check_suspect);
    log_msg(LEVEL_INFO, "total time insert into DB:   %lu\n", g_time_insert);
    log_msg(LEVEL_INFO, "total check with re:         %lu\n", g_time_re);

    if (g_total_count)
    {
        log_msg(LEVEL_INFO, "average check time:          %ld\n", 
                g_total_check_time / g_total_count);
    }

    if (g_normal_count)
    {
        log_msg(LEVEL_INFO, "average check time normal:   %ld\n", 
                g_time_check_normal / g_normal_count);
        log_msg(LEVEL_INFO, "average normalization time:  %ld\n", 
                g_normalization_time / g_normal_count);
    }

    if (g_error_count)
    {
        log_msg(LEVEL_INFO, "average check time error:    %ld\n", 
                g_time_check_error / g_error_count);
    }

    if (g_error_count || sql_count_whitelist || sql_count_suspect)
    {
        log_msg(LEVEL_INFO, "average time insert into DB: %ld\n", 
               g_time_insert / (g_error_count + sql_count_whitelist + sql_count_suspect));
    }

    if (g_re_sql_count)
    {
        log_msg(LEVEL_INFO, "average time check with re:  %ld\n", g_time_re / g_re_sql_count);
        log_msg(LEVEL_INFO, "average time check suspect:  %ld\n", 
                g_time_check_suspect / g_re_sql_count);
    }

    log_msg(LEVEL_INFO, "=========================================================\n");
    log_msg(LEVEL_INFO, "Dumping report completed.\n");
}

int sql_load_re()
{
    struct  stat st;
    FILE   *fp = NULL;
    char   *line = NULL; 
    size_t  len = 0;
    ssize_t bytes_read = 0;

    g_re_count = 0;

    // Open the config file
    if(stat(g_re_config, &st))
    {
        log_msg(LEVEL_INFO, 
                "failed to get stats of regular expression config file [%s], reason: %d\n",
               g_re_config,
               errno);

        return 0;
    }

    fp = fopen(g_re_config, "r"); // read only
    if (fp == NULL)
    {
        log_msg(LEVEL_INFO, "failed to open file [%s], reason: %d\n", g_re_config, errno);
        return 0;
    }

    while ((bytes_read = getline(&line, &len, fp)) != -1)
    {
        pcre2_code *re = NULL;
        int errornumber = 0;
        unsigned long length = strlen(line);

        PCRE2_SIZE erroroffset;

        if (line[length-1] == '\n')
        {
            line[length-1] == '\0';
        }

        if (length == 0)
        {
            continue;
        }

        re = pcre2_compile((PCRE2_SPTR) line,     /* the pattern */
                           PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
                           PCRE2_CASELESS,        /* default options */
                           &errornumber,          /* for error number */
                           &erroroffset,          /* for error offset */
                           NULL);                 /* use default compile context */

        /* Compilation failed: print the error message and exit. */
        if (re == NULL)
        {
            PCRE2_UCHAR buffer[256];
            pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
            log_msg(LEVEL_INFO, "PCRE2 compilation failed at offset %d: %s\n", 
                    (int)erroroffset, buffer);

            return 0;
        }

        g_re_array[g_re_count] = re;
        g_re_count++;

        log_msg(LEVEL_INFO, "regular expression added: %s\n", line);

        if (g_re_count == MAX_RE)
        {
            log_msg(LEVEL_INFO, "max regular expression reached: %d", MAX_RE);
            break;
        }

    }

    // Cleanup
    fclose(fp);

    if (line)
    {
        free(line);
    }

    g_match_data = pcre2_match_data_create(1,     /* We only care match or not */
                                         NULL); /* Using malloc to allocate memory */

    return 1;
}

void sql_re_cleanup ()
{
    unsigned int i = 0;
    pcre2_code *re = NULL;

    pcre2_match_data_free(g_match_data);

    for (i = 0; i < g_re_count; i++)
    {
        re = g_re_array[i]; 
        pcre2_code_free(re);
    }
}

/*
    1: match
    0: no match
*/
int sql_check_sql_re (char *sql)
{
    int rc = 0;
    int i = 0;
    struct timespec start_time = {0};
    struct timespec end_time   = {0};
    unsigned long length = strlen(sql);
    pcre2_code *re = NULL;

    g_re_sql_count++;

    for (i = 0; i < g_re_count; i++)
    {
        re = g_re_array[i];

        if (clock_gettime(CLOCK_REALTIME, &start_time))
        {
            log_msg(LEVEL_INFO, "failed to get start time of inserting suspect sql, "
                    "time indicator might be incorrect.\n");
        }

        rc = pcre2_match(re,         /* the compiled pattern */
                         sql,        /* the subject string */
                         length,     /* the length of the subject */
                         0,          /* start at offset 0 in the subject */
                         0,          /* default options */
                         g_match_data, /* block for storing the result */
                         NULL);      /* use default match context */

        if (clock_gettime(CLOCK_REALTIME, &end_time))
        {
            log_msg(LEVEL_INFO, "failed to get end time of inserting suspect sql, "
                   "time indicator might be incorrect.\n");
        }

        g_time_re += time_subtract(&end_time, &start_time);

        /* Match */
        if (rc >= 0)
        {
            char sql_text[BUFFER_SIZE] = {'\0'};
            unsigned long sql_length = 0;

            memset(g_bind_suspect, 0, sizeof(g_bind_suspect));
            //strncpy(sql_text, sql, (BUFFER_SIZE - 1));
            snprintf(sql_text, (BUFFER_SIZE - 1), "%s", sql);
            sql_length = strlen(sql_text);

            g_bind_suspect[0].buffer_type = MYSQL_TYPE_STRING;
            g_bind_suspect[0].buffer = (char *)sql_text;
            g_bind_suspect[0].buffer_length = sizeof(sql_text);
            g_bind_suspect[0].is_null = 0;
            g_bind_suspect[0].length = &sql_length;

            /* Bind the buffers */
            if (mysql_stmt_bind_param(g_stmt_suspect, g_bind_suspect))
            {
                log_msg(LEVEL_ERROR, " mysql_stmt_bind_param() failed for suspect sqls\n");
                log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_suspect));
            }

            if (clock_gettime(CLOCK_REALTIME, &start_time))
            {
                log_msg(LEVEL_INFO, "failed to get start time of inserting suspect sql, "
                       "time indicator might be incorrect.\n");
            }

            /* Execute the INSERT statement - 1*/
            if (mysql_stmt_execute(g_stmt_suspect))
            {
                log_msg(LEVEL_ERROR, " mysql_stmt_execute(), 1 failed for suspect sqls\n");
                log_msg(LEVEL_ERROR, " %s\n", mysql_stmt_error(g_stmt_suspect));
                // exit ids_server if the connection is break down.
                // supervisor will restart the ids server process.
                return 1; // Return match
            }

            if (clock_gettime(CLOCK_REALTIME, &end_time))
            {
                log_msg(LEVEL_INFO, "failed to get end time of inserting suspect sql, "
                       "time indicator might be incorrect.\n");
            }

            g_time_insert += time_subtract(&end_time, &start_time);

            return 1; // Return match
        }
    }

    // No match, 
    return 0;
}
