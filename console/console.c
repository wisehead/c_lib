/*******************************************************************************
 *      File Name: console.c                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 2016/09/30-15:54                                    
 *	Modified Time: 2016/09/30-15:54                                    
 *******************************************************************************/
#include <stdio.h>
#include <netinet/in.h>    // for sockaddr_in  
#include <sys/types.h>    // for socket  
#include <sys/socket.h>    // for socket  
#include <arpa/inet.h>  
#include <stdio.h>        // for printf  
#include <stdlib.h>        // for exit  
#include <string.h>        // for bzero  
#include <unistd.h>  
/* 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 
*/  
  
#define HELLO_WORLD_SERVER_PORT    6666   
#define BUFFER_SIZE 16384  
#define FILE_NAME_MAX_SIZE 512  
/* ids_console supports the following command-line arguments:
 * 
 * -I - add an exception sql to the exception_sql table. 
 * -o outfile - write output to outfile instead of stdout
 * 
 * The optString global tells getopt() which options we
 * support, and which options have arguments.
 */
enum cmd_type_t {
    CMD_INSERT_EXCEPTION,
    CMD_UPDATE_CONFIG,
    CMD_MAX_TYPE
};

struct global_args_t {
    enum cmd_type_t  cmd_id;
    char *sql;                  /* -I option */
}global_args;

static const char *optString = "I:Uvh?";

/* Display program usage, and exit.
 */
void display_usage(void)
{
    puts("ids_console - console of the ids server.");
    puts("usage: ids_console -option optargs");
    puts("-h : help info.");
    puts("-I : insert the SQL to the exception_sql table.");
    puts("-U : update the rules changes to the IDS Server.");
    /* ... */
    exit(EXIT_FAILURE);
}

/* Convert the input files to HTML, governed by global_args.
 */
void convert_document(void)
{
    /* ... */
}
  
int main(int argc, char **argv)  
{  
    int opt = 0;
    
    /* Initialize global_args before we get to work. */
    global_args.sql = NULL;     /* false */
    
    /* Process the arguments with getopt(), then 
     * populate global_args. 
     */
    opt = getopt(argc, argv, optString);
    while (opt != -1) {
        switch (opt) {
            case 'I':
                //global_args.noIndex = 1; /* true */
                global_args.cmd_id = CMD_INSERT_EXCEPTION;
                global_args.sql = optarg; /* true */
                break;
            case 'U':
                //global_args.noIndex = 1; /* true */
                //global_args.sql = optarg; /* true */ 
                global_args.cmd_id = CMD_UPDATE_CONFIG;
                break;
                
            case 'h':   /* fall-through is intentional */
            case '?':
                display_usage();
                //break;
                exit(1);
                
            default:
                display_usage();
                /* You won't actually get here. */
                exit(1);
                //break;
        }
        
        opt = getopt(argc, argv, optString);
    }
    //convert_document();

    if (argc != 2 && argc != 3)  
    {  
        display_usage();
        exit(1);  
    }  

    char buffer[BUFFER_SIZE];  
    bzero(buffer, BUFFER_SIZE);  
    if (global_args.cmd_id == CMD_INSERT_EXCEPTION && global_args.sql == NULL)
    {
        printf("No SQL text!\n");  
        exit(1);
    }
    //snprintf(buffer, BUFFER_SIZE, "COMMAND:%s;OPTION:%s", argv[1], argv[2]);
    if (argc == 3)
    {
        snprintf(buffer, BUFFER_SIZE, "COMMAND:%s;OPTION:%s", argv[1], argv[2]);
    }
    else if (argc == 2)
    {
        snprintf(buffer, BUFFER_SIZE, "COMMAND:%s", argv[1]);
    }
    
    //����һ��socket��ַ�ṹclient_addr,����ͻ���internet��ַ, �˿�  
    struct sockaddr_in client_addr;  
    bzero(&client_addr, sizeof(client_addr)); //��һ���ڴ���������ȫ������Ϊ0  
    client_addr.sin_family = AF_INET;    //internetЭ����  
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY��ʾ�Զ���ȡ������ַ  
    client_addr.sin_port = htons(0);    //0��ʾ��ϵͳ�Զ�����һ�����ж˿�  
    //��������internet����Э��(TCP)socket,��client_socket����ͻ���socket  
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);  
    if (client_socket < 0)  
    {  
        printf("Create Socket Failed!\n");  
        exit(1);  
    }  
    //�ѿͻ�����socket�Ϳͻ�����socket��ַ�ṹ��ϵ����  
    if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))  
    {  
        printf("Client Bind Port Failed!\n");   
        exit(1);  
    }  
  
    //����һ��socket��ַ�ṹserver_addr,�����������internet��ַ, �˿�  
    struct sockaddr_in server_addr;  
    bzero(&server_addr, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;  
    if (inet_aton("127.0.0.1", &server_addr.sin_addr) == 0) //��������IP��ַ���Գ���Ĳ���  
    {  
        printf("Server IP Address Error!\n");  
        exit(1);  
    }  
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);  
    socklen_t server_addr_length = sizeof(server_addr);  
    //���������������,���ӳɹ���client_socket�����˿ͻ����ͷ�������һ��socket����  
    if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)  
    {  
        printf("Can Not Connect To IDS server!\n");  
        exit(1);  
    }  
          
    size_t  len = 0;
    size_t  line_len = 0;
    line_len = strlen(buffer);
    if (send(client_socket, buffer, line_len, 0) < 0)  
    {  
        printf("Send command:\t%s Failed\n", buffer);  
    }  
    bzero(buffer, BUFFER_SIZE);  
      
    //�ر�socket  
    close(client_socket);  
    //return 0;  
    return EXIT_SUCCESS;
}  
