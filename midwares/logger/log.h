/*******************************************************************************
 *      File Name: log.h                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 2016/11/05-23:16                                    
 *	Modified Time: 2016/11/05-23:16                                    
 *******************************************************************************/
#ifndef DBA_IDS_LOG_H 
#define DBA_IDS_LOG_H 

//#define BUF_SIZE 1024 
#define LEVEL_DEBUG 0  
#define LEVEL_INFO 1  
#define LEVEL_WARN 2  
#define LEVEL_ERROR 3  
/*
enum LOG_LEVEL
{
    LEVEL_DEBUG,
    LEVEL_INFO,
    LEVEL_WARN, 
    LEVEL_ERROR,
    LEVEL_COUNT
};
*/

extern unsigned int g_log_count;
typedef struct _log_st log_st;  
struct _log_st  
{  
    char path[128];  
    int fd;  
    int size;  
    int level;  
    int num;  
}; 

extern log_st* g_log_handle;//for ids log
log_st *log_init(char *path, int size); 
void log_msg(int level, const char *msg, ...);
void log_big_msg(int level, const char *msg, ...);

#endif /* DBA_IDS_LOG_H */
