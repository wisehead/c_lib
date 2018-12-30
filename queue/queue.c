/*******************************************************************************
 *      File Name: queue.c                                               
 *         Author: Hui Chen. (c) 2016                             
 *           Mail: alex.chenhui@gmail.com                                       
 *   Created Time: 2016/10/18-11:36                                    
 *  Modified Time: 2016/10/18-11:36                                    
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "ids_queue_list.h"  
#include "ids_log.h"
  
typedef struct QNode  
{  
    ElementType Element;  
    struct QNode *Next;  
}QNode, *QNodePtr;  
  
struct Node {  
    QNodePtr Front;  
    QNodePtr Rear;  
};  
  
int  
is_empty(Queue Q)  
{  
    return Q->Front == Q->Rear;  
}  
  
Queue  
create_queue(void)  
{  
    Queue q;
    q = malloc(sizeof(struct Node));  
    q->Front = q->Rear = malloc(sizeof(struct QNode));  
    if (!q->Front)  
    {
        //FatalError("Out of space!!!");  
        log_msg(LEVEL_ERROR, "Out of space!!!\n");
        exit(1);
    }
    q->Front->Next = NULL;  
    return q;  
}  
  
void  
make_empty(Queue Q)  
{  
    if(Q == NULL)  
    {
        //Error("Must use CreateQueue first");  
        log_msg(LEVEL_ERROR, "Must use CreateQueue first\n");
    }
    else  
    {
        while (!is_empty(Q))  
        {
            dequeue(Q);  
        }
    }
}  
  
void  
dispose_queue(Queue Q)  
{  
    while (Q->Front) {  
        Q->Rear = Q->Front->Next;  
        free(Q->Front);  
        Q->Front = Q->Rear;  
    }  
    log_msg(LEVEL_INFO, "Dispose queue completed!!!\n");
}  
void  
enqueue(ElementType X, Queue Q)  
{  
    QNodePtr p;  
    p = malloc(sizeof(QNode));  
    if (!p)  
    {
        //FatalError("Out of space!!!");  
        log_msg(LEVEL_ERROR, "Out of space!!!\n");
        exit(1);
    }
    p->Element = X;  
    p->Next = NULL;  
    Q->Rear->Next = p;  
    Q->Rear = p;  
}  
  
ElementType  
front(Queue Q)  
{  
    if (!is_empty(Q))  
    {
        return Q->Front->Next->Element;  
    }
    return 0; /* Return value used to avoid warning */  
}  
  
void  
dequeue(Queue Q)  
{  
    if (!is_empty(Q))  
    {  
        QNodePtr p;  
        /*
        p = malloc(sizeof(QNode));  
        if (!p)  
        {
            FatalError("Out of space!!!");  
        }
        */
        p = Q->Front->Next;  
        Q->Front->Next = p->Next;  
        if (Q->Rear == p)  
        {
            Q->Rear = Q->Front;  
        }
        free(p);  
    }  
}  
  
ElementType  
front_and_dequeue(Queue Q)  
{  
    if (!is_empty(Q))  
    {  
        QNodePtr p;  
        /*
        p = malloc( sizeof( QNode ) );  
        if (!p)  
          FatalError( "Out of space!!!" );  
        */
        p = Q->Front->Next;  
        if (!p)  
        {
            //FatalError("Out of space!!!");  
            log_msg(LEVEL_ERROR, "Out of space!!!\n");
            exit(1);
        }
        ElementType temp = 0;  
        temp = p->Element;  
        Q->Front->Next = p->Next;  
        if (Q->Rear == p)  
        {
            Q->Rear = Q->Front;  
        }
        free(p);  
        return temp;  
    }  
    //Error("Empty queue!!!");  
    log_msg(LEVEL_ERROR, "Empty queue!!!\n");
    //exit(1);
    return 0; /* Return value used to avoid warning */  
}  
