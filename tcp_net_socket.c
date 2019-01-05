/*********************************************************************
File Name:               tcp_net_socket.c
Author:                          date:
Description:            
Fuction List:			int tcp_init() 							//用于初始化操作
						int tcp_accept(int sfd)					//用于服务器的接收
						int tcp_connect(const char* ip)			//用于客户端的连接
						void signalhandler(void)				//用于信号处理，让服务器在按下Ctrl+c或Ctrl+\时不会退出
						void setSockNonBlockMode(int sock)      //用于建立非阻塞模式
						int updateMaxFD(fd_set fds, int maxfd) 	//用于更新maxfd
********************************************************************/
 
#include "tcp_net_socket.h"

/**server receive message and handler
  *input: dinfo[][] device infomation
  *input: rid rainbow id
  *input: cid client id
  *return: >0 success =0 close the socket <0 receive or send msg fail
  */
int serverMessageHandler(DEVICE_INFO dinfo[RAINBOWMAX][CLIENTMAX+1], UINT32 rid, UINT32 cid)
{
    int ret = 1;
	UINT32 tmp_cid;
    char buf[1024];
	UINT32 tmp_cid_num = 0;
	
    if (dinfo[rid][cid].type == DEVICE_RAINBOW) {
		ret = read(dinfo[rid][cid].fd, buf, 1024);
        if (ret == -1) {
            perror("Rainbow Read Failed.");
		} else if (ret == 0) {//device closed
            if(close(dinfo[rid][cid].fd) == -1){
               perror("close rainbow fail!"); 
            }
			clearDeviceTable(&dinfo[rid][cid]);
            printf("rainbow disconnected!\n");
		} else {
            if (ret == HEARTLENGTH && strcmp(buf, heart_packet)) {//judge if it is heart packet 
                printf("RAINBOW HEARTBEAT: %s\n", heart_packet);
                memset(buf, 0, 1024);
			    return ret;
			} 
			printf("Rainbow Read: %d bytes read.\n", ret);  
			for(tmp_cid=1;tmp_cid<CLIENTMAX;tmp_cid++) {
                if (dinfo[rid][tmp_cid].fd != -1) {
					tmp_cid_num++;
                    if (ret != write(dinfo[rid][tmp_cid].fd, buf, 1024)) {
						ret = -1;
                        perror("device write error.\n");
					}
				}
			}
			memset(buf, 0, 1024);
			
			if (tmp_cid_num) {
                printf("rainbow send %d clients msg success.\n", tmp_cid_num);
			} else {
                printf("no device send.\n");
			}
		}
	} else {
        ret = read(dinfo[rid][cid].fd, buf, 1024);
		if (ret == -1) {
            perror("client Read Failed.");
		} else if (ret == 0) {//device closed
            if(close(dinfo[rid][cid].fd) == -1){
               perror("close client device failed!"); 
            }
			clearDeviceTable(&dinfo[rid][cid]);
            printf("client disconnected!\n");
		} else {
            if (ret == HEARTLENGTH && strcmp(buf, heart_packet)) {//judge if it is heart packet 
                printf("CLIENT HEARTBEAT: %s\n", heart_packet);
                memset(buf, 0, 1024);
			    return ret;
			}
			printf("Client Read: %d bytes read.\n", ret);
			if (ret != write(dinfo[rid][0].fd, buf, 1024)) {
                ret = -1;
				perror("rainbow write error.\n");
			}
			memset(buf, 0, 1024);
			printf("client send msg to rainbow success.\n");
		}
	}

	return ret;
}

/**get current socket's rainbowid or cid*/
int getDoubleId(DEVICE_INFO **dinfo, UINT32 *currid, UINT32 *curcid, int curfd)
{
	UINT32 rainbowid, cid;
	
	for(rainbowid=0;rainbowid<RAINBOWMAX;rainbowid++) {//look up the socket is rainbow or client
          for(cid=0;cid<CLIENTMAX+1;cid++) {
              if (dinfo[rainbowid][cid].fd == curfd) {
                  *currid = rainbowid;
				  *curcid = cid;
			      printf("get current rainbowid: %d, cid: %d", rainbowid, cid);
			      return 0;
			  }
		  }
	}
	if (cid == CLIENTMAX+1 && rainbowid == RAINBOWMAX) {
        printf("get current double id fail!");
		return -1;
	}
}


/**get rainbow id*/
UINT32 getRainbowId(char *str)
{
    char *temp = str;
    UINT32 id = 0;
	UINT32 i;
	
	temp += AUTHRAINBOWLENGTH;

	id = atoi(temp);
	if (id == 0) {
		printf("getRainbowId fail!");
	} else if (id > RAINBOWMAX) {
        id = 0;
		printf("getRainbowId exceed maxmum!");
	} else {
        printf("get rainbow id: %d", id);
	}

	return id;
}

/**get rainbow id which client want to connect*/
UINT32 getClientId(char *str)
{
    char *temp = str;
    UINT32 id = 0;
	UINT32 i;
	
	temp += AUTHRAINBOWCLIENTLENGTH;

	id = atoi(temp);
	if (id == 0) {
		printf("get rainbow id fail from client!");
	} else if (id > RAINBOWMAX) {
        id = 0;
		printf("get rainbow id exceed maxmum from client!");
	} else {
        printf("get rainbow id which client want to connect: %d", id);
	}

	return id;
}

/**clear device info struct table*/
void clearDeviceTable(DEVICE_INFO *device)
{
    DEVICE_INFO temp;

	temp.fd = -1;
	temp.Id = -1;
	temp.password = -1;
	temp.type = -1;

	*device = temp;
}

//得到最大的FD
int updateMaxFD(fd_set fds, int maxfd)
{
	int i, new_maxfd = 0;
	for(i=0;i<=maxfd;i++){
    	if(FD_ISSET(i, &fds)&&(i>new_maxfd)){
      		new_maxfd = i;
      	}                      
    }
}

//设置sock为non-blocking mode
void setSockNonBlockMode(int sock)
{
	int flags;
	flags = fcntl(sock, F_GETFL, 0);
	if(flags < 0)
    {
    	perror("fcntl(F_GETFL) failed");
        //return;
    }
    if(fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		perror("fcntl(F_SETFL) failed");
        //return;
	}
}

//用于初始化操作
int tcp_init()  
{
    int bReuseaddr=1;
	int port;
	int sfd = socket(AF_INET, SOCK_STREAM, 0);     //创建套接字
	if(sfd == -1)
	{
		perror("socket");
		return -1;
	}

	//setSockNonBlockMode(sfd);

	int ret;
	struct sockaddr_in serveraddr;
 
	//printf("please input server listen port:");
	//scanf("%d", &port);
	memset(&serveraddr,0,sizeof(struct sockaddr));	
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERV_PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &bReuseaddr, sizeof(bReuseaddr))<0) {//avoid server restart "bind already in use"
		perror("setsockopt");
    }
                                                                         
	ret = bind(sfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr));
	if(ret == -1)
	{
		perror("bind");
		return -1;
	}
	
	ret = listen(sfd,3000);           //监听它，并设置允许最大的连接数为10个
	if(ret == -1)
	{
		perror("listen");
		close(sfd);
		return -1;
	}
	
	return sfd;
}
 
//用于服务器的接收
int tcp_accept(int sfd)
{
	struct sockaddr_in clientaddr;
	memset(&clientaddr, 0, sizeof(struct sockaddr));
	int addrlen = sizeof(struct sockaddr);
	
	//sfd接受客户端的连接，并创建新的socket为new_fd，将请求连接的客户端的ip、port保存在结构体clientaddr中
	int new_fd = accept(sfd, (struct sockaddr*)&clientaddr, &addrlen);       
	if(new_fd == -1)
	{
		perror("accept");
		//close(sfd);   //temperary deletion
		return -1;
	}
	printf("%s %d success connet...\n", 
	inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
	
	return new_fd;
}
 
//用于客户端的连接
int tcp_connect(const char* ip)
{
	int ret;
	int sfd = socket(AF_INET, SOCK_STREAM, 0);     //申请新的socket
	if(sfd == -1)
	{
		perror("socket");
		return -1;
	}
	
	struct sockaddr_in serveraddr;
	
	memset(&serveraddr, 0,sizeof(struct sockaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERV_PORT);
	serveraddr.sin_addr.s_addr = inet_addr(ip);    
	
	ret = connect(sfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr));       //将sfd连接至指定的服务器网络地址 serveraddr
	if(ret == -1)
	{
		perror("connect");
		close(sfd);
		return -1;
	}
	
	return sfd;
}
 
//用于信号处理，让服务器在按下Ctrl+c或Ctrl+\时不会退出
void signalhandler(void)
{
	sigset_t sigSet;
	sigemptyset(&sigSet);
	sigaddset(&sigSet,SIGINT);
	sigaddset(&sigSet,SIGQUIT);
	sigprocmask(SIG_BLOCK,&sigSet,NULL);
}
