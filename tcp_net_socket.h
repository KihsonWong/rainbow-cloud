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

#define SERV_PORT 8181
#define RAINBOWMAX 1000
#define CLIENTMAX 6
#define TEMPMAX 1000  //the num is the maximal value that clients connect cloud server at the same time
#define HEARTLENGTH 25
#define AUTHASKLENGTH 12
#define AUTHRAINBOWLENGTH 12
#define AUTHRAINBOWCLIENTLENGTH 30

typedef unsigned int UINT32;

typedef struct _FD_INFO{
    int index;    //temp index
    int fd;       //temp fd
}FD_INFO;

typedef struct _DEVICE_INFO {
    int fd;
	int type;
#define DEVICE_RAINBOW 0
#define DEVICE_CLIENT 1
	int Id;
	int password;
} DEVICE_INFO;

extern int tcp_init();
extern int tcp_accept(int sfd);
extern int tcp_connet();
extern void signalhandler(void);
extern void setSockNonBlockMode(int sock);      
extern int updateMaxFD(fd_set fds, int maxfd); 	
extern UINT32 getRainbowId(char *str);
extern void clearDeviceTable(DEVICE_INFO *device);
extern UINT32 getClientId(char *str);
extern int getDoubleId(DEVICE_INFO dinfo[RAINBOWMAX][CLIENTMAX+1], UINT32 *currid, UINT32 *curcid, int curfd);
extern int serverMessageHandler(DEVICE_INFO dinfo[RAINBOWMAX][CLIENTMAX+1], UINT32 rid, UINT32 cid);


#endif
