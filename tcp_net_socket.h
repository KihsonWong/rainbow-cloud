/******************************************************************
filename:               tcp_net_socket.h
Author:                         date:
Description:            
Fuction List:			
********************************************************************/
 
#ifndef _TCP_NET_SOCKET_H
#define _TCP_NET_SOCKET_H
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#define SERV_PORT 39873
#define MAXCLIENT 10
  
extern int tcp_init();
extern int tcp_accept(int sfd);
extern int tcp_connet();
extern void signalhandler(void);
extern void setSockNonBlockMode(int sock);      
extern int updateMaxFD(fd_set fds, int maxfd); 	
#endif
