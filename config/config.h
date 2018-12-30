/*******************************************************************************
 *      File Name: config.h                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 2016/10/10-18:26                                    
 *	Modified Time: 2016/10/10-18:26                                    
 *******************************************************************************/
#ifndef DBA_IDS_CONFIG_H 
#define DBA_IDS_CONFIG_H 

/* IP address of ids serser. */
extern char g_ids_servers[256][128];
extern int g_ids_server_count;
extern char g_preferred_ids_server[128];
extern unsigned int g_ids_server_port;

void set_parameter (char *parameter, char *value);
int load_connect_config();    

#endif /* DBA_IDS_CONFIG_H */
