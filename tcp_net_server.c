/*********************************************************************
File Name:               tcp_net_server.c
Author:                          date:
Description:            
Fuction List:			int tcp_init() 							//用于初始化操作
						int tcp_accept(int sfd)					//用于服务器的接收
						int tcp_connect(const char* ip)			//用于客户端的连接
						void signalhandler(void)				//用于信号处理，让服务器在按下Ctrl+c或Ctrl+\时不会退出
********************************************************************/
 
#include "tcp_net_socket.h"
#include "look_up.h"

int main(int argc, char *argv[])
{
  signalhandler();

  int sfd;
  int client[MAXCLIENT], maxfd, cid, i;
  char buf1[1024];
  char buf2[1024];
  fd_set readfds, readfds_bak;
  int ret;
  int ack_cid = 0;

  memset(buf1, 0, 1024);
  memset(buf2, 0, 1024);

  sfd = tcp_init();
  maxfd = sfd;

  FD_ZERO(&readfds);
  FD_ZERO(&readfds_bak);
  FD_SET(sfd, &readfds_bak);
  for(cid=0;cid<10;cid++){
      client[cid] = -1;                   
  }

  char heart_packet[24] = "HOLD ON CONNECTING HEART";
                           
	while(1){	
    //"readfd" and "timeout" change after every select(), maxfd maybe change.
    readfds = readfds_bak;
    maxfd = updateMaxFD(readfds, maxfd);
    ret = select(maxfd+1, &readfds, NULL, NULL, NULL);
    if(ret == -1){
      perror("select()");
    }
    else if(ret){
      for(i=0;i<=maxfd;i++){
        if(!FD_ISSET(i, &readfds)){
          continue;
        }
        if(i == sfd){
          //this is rainbow's socket, no read or write, just tcp_accpet.
          for(cid=0;cid<10;cid++){
            if(client[cid]==-1){
              client[cid] = tcp_accept(sfd); 
              if(client[cid] == -1) printf("rainbow connect error!");
              if(cid == 0) printf("rainbow connected!\n");
              else printf("client[%d] connected!\n", cid);
              if(client[cid] > maxfd) maxfd = client[cid];
              FD_SET(client[cid], &readfds_bak);
              break;
            } 
            if((cid==9) && (client[cid]==-1))
              printf("clients are over, stop access!");                   
          }
        } else {
          //this is client's socket, read and write.
          for(cid=0;cid<10;cid++){
            if(client[cid]!=-1 && FD_ISSET(client[cid], &readfds)){//readable
              if(FD_ISSET(client[0], &readfds)) {
                ret = read(client[0], buf1, 1024);
                if(ret == -1)
                  perror("Rainbow Read Failed.");
                else if(ret == 0) {//client close
                  if(close(client[cid]) == -1){
                    perror("close rainbow fail!"); 
                  }
                  FD_CLR(client[cid], &readfds_bak);//clr from readfds
                  client[cid] = -1;//release one position
                  printf("rainbow disconnected!\n");
                } else {//receive data from rainbow	
                  if (ret == 24 ) {//judge if it is heart packet 
                      printf("%s\n", heart_packet);
                      memset(buf1, 0, 1024);
                      break;
                  } else {
                      printf("Rainbow Read: %d bytes read.\n", ret);                
                      if(ack_cid == 0) {
                          memset(buf1, 0, 1024);
                          printf("No client, so don't need to send info.\n");
                          break;
                      }
                      for(i=0;i<MAXCLIENT;i++){
                      
                      }
                      if(ret != write(client[ack_cid], buf1, ret))//rainbow ack last client
                          perror("Client write error.\n");
                  
                      memset(buf1, 0, 1024);
                      printf("send to client from rainbow success.\n");
                  }
                }
              } else if(FD_ISSET(client[cid], &readfds) && (cid != 0)){
                ret = read(client[cid], buf2, 1024);
                if(ret == -1)
                  perror("Client Read Failed.\n");
                else if(ret == 0) {
                  if(close(client[cid]) == -1){
                    perror("close client fail!"); 
                  }
                  FD_CLR(client[cid], &readfds_bak);
                  client[cid] = -1;
                  if(1>=get_accepted_client_num(client, MAXCLIENT)){
                    ack_cid = 0;	
                  }
                  printf("client[%d] closed!\n", cid);
                } else {	
                  printf("Client[%d] Read: %d bytes read.\n", cid, ret);
                  if(ret != write(client[0], buf2, ret))//send to rainbow from last client
                    perror("write Rainbow error.\n");
                    
                  ack_cid = cid;  
                  memset(buf2, 0, 1024);
                  printf("send to rainbow from client[%d] success.\n", cid);  
                }
              } 	                     
            }
          }      
        }
      }
    } else {
      printf("No data within NULL.\n");
    }
    i++;
  }
  close(sfd);
  return 0;
}