/*******************************************************************************
 *      File Name: log.c                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 2016/11/05-23:17                                    
 *	Modified Time: 2016/11/05-23:17                                    
 *******************************************************************************/
#include <stdio.h>
#include <stdarg.h>  
#include <stdlib.h>  
#include <string.h>  
#include <time.h>  
#include <fcntl.h>
#include "ids_log.h"

#define FSYNC_FREQUENCY 1
unsigned int g_log_count = 0;
#define BUF_SIZE 1024 
#define BUF_SIZE_BIG  1048576//1MB 

log_st* g_log_handle = NULL;//for ids log
static const char * log_str[] = {"DEBUG", "INFO", "WARN", "ERROR"}; 

log_st *log_init(char *path, int size)  
{  
    char new_path[128] = {0};  
    if (NULL == path) 
    {
        return NULL;  
    }
    log_st *log = (log_st *)malloc(sizeof(log_st));  
    memset(log, 0, sizeof(log_st));  
    //snprintf(new_path, 128, "%s.new", path);  
    snprintf(new_path, 128, "%s", path);  
    if (-1 == (log->fd = open(new_path, 
                    O_RDWR | O_APPEND | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR | S_IROTH)))  
    {  
        free(log);  
        log = NULL;  
        return NULL;  
    }  
    //strncpy(log->path, path, 128);  
    snprintf(log->path, 128, "%s", path);
    log->size = (size > 0 ? size:0);  
    return log;  
}  

void log_msg(int level, const char *msg, ...)  
{  
    va_list ap;  
    char *pos = NULL;  
    char _n = '\n';  
    char message[BUF_SIZE] = {0};  
    int message_len = 0;  
    int sz = 0;  
    log_st* log = g_log_handle;
    if(NULL == log) 
    {
        return;  
    }
    char ti[32];
    time_t now = time(NULL);
    strftime(ti, sizeof(ti), "%Y-%m-%d %H:%M:%S", localtime(&now));
    //now = time(NULL);  
    //pos = ctime(&now);  
    //sz = strlen(pos);  
    //pos[sz-1]=']';  
    //snprintf(message, BUF_SIZE, "[%s ", pos);  
    //for (pos = message; *pos; pos++);  
    snprintf(message, BUF_SIZE, "[%s][%s]", ti, log_str[level]);
    for (pos = message; *pos; pos++)
    {
        //do nothing.
    };

    sz = pos - message;  
    va_start(ap, msg);  
    message_len = vsnprintf(pos, BUF_SIZE - sz, msg, ap);  
    va_end(ap);  
    //fprintf(fp, "[%s] [%s] %s\n", ti, log_str[level], msg);

    if (message_len <= 0) 
    {
        return;  
    }

    //printf("%s\n", message);  
    write(log->fd, message, strlen(message));  
    //write(log->fd, &_n, 1);  
    if (g_log_count++ % FSYNC_FREQUENCY == 0)
    {
        fsync(log->fd);  
    }
}  

void log_big_msg(int level, const char *msg, ...)  
{  
    va_list ap;  
    char *pos = NULL;  
    char _n = '\n';  
    char message[BUF_SIZE_BIG] = {0};  
    int message_len = 0;  
    int sz = 0;  
    log_st* log = g_log_handle;
    if(NULL == log) 
    {
        return;  
    }
    char ti[32];
    time_t now = time(NULL);
    strftime(ti, sizeof(ti), "%Y-%m-%d %H:%M:%S", localtime(&now));
    //now = time(NULL);  
    //pos = ctime(&now);  
    //sz = strlen(pos);  
    //pos[sz-1]=']';  
    //snprintf(message, BUF_SIZE, "[%s ", pos);  
    //for (pos = message; *pos; pos++);  
    snprintf(message, BUF_SIZE_BIG, "[%s][%s]", ti, log_str[level]);
    for (pos = message; *pos; pos++)
    {
        //do nothing.
    };

    sz = pos - message;  
    va_start(ap, msg);  
    message_len = vsnprintf(pos, BUF_SIZE_BIG - sz, msg, ap);  
    va_end(ap);  
    //fprintf(fp, "[%s] [%s] %s\n", ti, log_str[level], msg);

    if (message_len <= 0) 
    {
        return;  
    }

    //printf("%s\n", message);  
    write(log->fd, message, strlen(message));  
    //write(log->fd, &_n, 1);  
    if (g_log_count++ % FSYNC_FREQUENCY == 0)
    {
        fsync(log->fd);  
    }
}  

void log_checksize(log_st *log)  
{  
    struct stat stat_buf;  
    char new_path[128] = {0};  
    char bak_path[128] = {0};  
    if(NULL == log || '\0' == log->path[0]) 
    {
        return;  
    }
    memset(&stat_buf, 0, sizeof(struct stat));  
    fstat(log->fd, &stat_buf);  
    if(stat_buf.st_size > log->size)  
    {  
        close(log->fd);  
        snprintf(bak_path, 128, "%s.bak", log->path);  
        snprintf(new_path, 128, "%s.new", log->path);  
        remove(bak_path); //delete the file *.bak first  
        rename(new_path, bak_path); //change the name of the file *.new to *.bak  
        //create a new file  
        log->fd = open(new_path, O_RDWR | O_APPEND | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR | S_IROTH);
    }  
}  
