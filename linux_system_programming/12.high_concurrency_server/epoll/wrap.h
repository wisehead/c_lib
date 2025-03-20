/*******************************************************************************
 *      File name: wrap.h                                               
 *         Author: Hui Chen. (c) 2022                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 22/09/19-9月:09                                    
 * Modified Time: 22/09/19-9月:09                                    
 *******************************************************************************/
/* wrap.h */ 
#ifndef __WRAP_H_ 
#define __WRAP_H_
void perr_exit(const char *s);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
void Bind(int fd, const struct sockaddr *sa, socklen_t salen);
void Connect(int fd, const struct sockaddr *sa, socklen_t salen);
void Listen(int fd, int backlog);
int Socket(int family, int type, int protocol);
ssize_t Read(int fd, void *ptr, size_t nbytes);
ssize_t Write(int fd, const void *ptr, size_t nbytes);
void Close(int fd);
ssize_t Readn(int fd, void *vptr, size_t n);
ssize_t Writen(int fd, const void *vptr, size_t n);
static ssize_t my_read(int fd, char *ptr);
ssize_t Readline(int fd, void *vptr, size_t maxlen);
#endif
