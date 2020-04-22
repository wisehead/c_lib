/*******************************************************************************
 *      File Name: config.c                                               
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
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#define PCRE2_CODE_UNIT_WIDTH 8
//#include "sql_define.h"
#include "ids_config.h"

#define STRING_SIZE 131070 //for some big SQLs
#define BUFFER_SIZE 16384  //for some big SQLs
#define ERR_MSG_LEN 100   
const char *g_config_file = "conf/ids_agent.conf";

/* log file which contains the SQLs. */
char g_sql_file[256][128];
int g_sql_file_count;

/* IP address of ids serser. */
char g_ids_servers[256][128];
int g_ids_server_count;
char g_preferred_ids_server[128] = {0};
unsigned int g_ids_server_port;

/* if the log file is in davinci format */
char g_sql_file_format[16];

/* mysql connection parameters */
char g_host[64];
char g_user[32];
char g_password[32];
unsigned int g_port;
char g_db[32];
static char s_socket[256];//use the static, because in other files, we met a name confict.

char g_check_re;
char g_machine_learning_on;

// 1 - success
// 0 - failed
int load_connect_config()
{
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
                printf("configure parameter length exceeds limits, at line %d\n",
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
            printf("sql file count exceeds the max file count(128), ignore new files.\n");
            return;
        }

        bzero(g_sql_file[g_sql_file_count], 128);
        snprintf(g_sql_file[g_sql_file_count], 127, "%s", value);
        g_sql_file_count++;
    }
    else if (!strcmp(parameter, "ids_server"))
    {
        if (g_ids_server_count == 5)
        {
            printf("IDS server count exceeds the max file count(128), ignore new servers.\n");
            return;
        }

        bzero(g_ids_servers[g_ids_server_count], 128);
        snprintf(g_ids_servers[g_ids_server_count], 127, "%s", value);
        g_ids_server_count++;
    }
    else if (!strcmp(parameter, "preferred_ids_server"))
    {
        bzero(g_preferred_ids_server, 128);
        snprintf(g_preferred_ids_server, 127, "%s", value);
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
            printf("unkown value of check_re %s, using default.\n", value);
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
            printf("unkown value of g_machine_learning_on %s, using default.\n", value);
            g_machine_learning_on = 0;
        }
    }
    else if (!strcmp(parameter, "ids_server_port"))
    {
        g_ids_server_port = atoi(value);
    }
    else
    {
        printf("unrecognized parameter: [%s]\n", parameter);
    }
}
