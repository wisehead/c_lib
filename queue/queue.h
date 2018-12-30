/*******************************************************************************
 *      File Name: queue.h                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: alex.chenhui@gmail.com                                       
 *   Created Time: 2016/10/18-11:37                                    
 *  Modified Time: 2016/10/18-11:37                                    
 *******************************************************************************/
#ifndef DBA_IDS_QUEUE_LIST_H  
#define DBA_IDS_QUEUE_LIST_H  
  
#include <stdio.h>
#define ElementType char*  
//#define Error( Str )        FatalError( Str )  
//#define FatalError( Str )   fprintf( stderr, "%s\n", Str ), exit( 1 )  
struct Node;  
struct QNode;  
typedef struct Node *PtrToNode;  
typedef PtrToNode Queue;  
long int g_queue_node_count;
  
int is_empty(Queue Q);  
Queue create_queue(void);  
void dispose_queue(Queue Q);  
void make_empty(Queue Q);  
void enqueue(ElementType X, Queue Q);  
ElementType front(Queue Q);  
void dequeue(Queue Q);  
ElementType front_and_dequeue(Queue Q);  
  
#endif /* DBA_QUEUE_LIST_H */  
