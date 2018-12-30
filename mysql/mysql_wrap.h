/*******************************************************************************
 *      File Name: mysql_wrap.h                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: alex.chenhui@gmail.com                                       
 *   Created Time: 2016/10/10-18:26                                    
 *	Modified Time: 2016/10/10-18:26                                    
 *******************************************************************************/
#ifndef DBA_IDS_HELPER_H 
#define DBA_IDS_HELPER_H 

#include <stdio.h>
#include "sql_define.h"
/* IP address of davinci serser. */
extern char g_davinci_servers[256][128];
extern int g_davinci_server_count;
extern unsigned int g_ids_server_port;

#define STR_LEN 128
typedef struct _davinci_log_rec_t 
{
    char dbname[STR_LEN];
    char tbname[STR_LEN];
    char queryType[STR_LEN];
    //int dbhost;
    //int dbport;
    char dbhost[STR_LEN];//int
    char dbport[STR_LEN];//int
    char username[STR_LEN];
    char userhost[STR_LEN];
    char queryDate[STR_LEN];
    char queryTime[STR_LEN];
    //int sendQueryTime;
    //int receiveResultTime;
    //int sendResultTime;
    //int totalTime;
    //int affectedRows;
    //int resultSize;
    //int queryCount;
    char sendQueryTime[STR_LEN];//int
    char receiveResultTime[STR_LEN];//int
    char sendResultTime[STR_LEN];//int
    char totalTime[STR_LEN];//int
    char affectedRows[STR_LEN];//int
    char resultSize[STR_LEN];//int
    char queryCount[STR_LEN];//int
    char logType[STR_LEN];
    char json[STR_LEN];
    char GetSign[STR_LEN];
    char fingerprint[STR_LEN];
    char reserved[STR_LEN];
} davinci_log_rec_t;

void set_parameter (char *parameter, char *value);
void dump_connect_parameters();
int load_connect_config();    
void sql_re_cleanup();
int sql_load_re();            
void print_report();   
long time_subtract(struct timespec *end, struct timespec *start);
char *sql_get_sql(char *line);
void sql_init();
void sql_init2();
void sql_cleanup();
void sql_check_sql(davinci_log_rec_t *log_rec_head, char *sql, char *sql_ptr);
int sql_check_sql_re (char *sql);
int insert_exception(char* sql);

#endif /* DBA_IDS_HELPER_H */
