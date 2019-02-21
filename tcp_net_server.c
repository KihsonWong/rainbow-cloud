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
//#include "look_up.h"

int main(int argc, char *argv[])
{
  signalhandler();

  int sfd;
  FD_INFO fd_temp[TEMPMAX];
  UINT32 temp;  
  DEVICE_INFO device[RAINBOWMAX][CLIENTMAX+1];
  int maxfd, curfd;
  UINT32 cid, rainbowid, curcid, currainbowid;
  char buf1[1024];
  //char buf2[1024];
  fd_set readfds, readfds_bak;
  int ret;
  int ack_cid = 0;
  UINT32 temp_id;
  char tmp_buff[100];

  memset(buf1, 0, 1024);
  //memset(buf2, 0, 1024);

  sfd = tcp_init();
  maxfd = sfd;

  FD_ZERO(&readfds);
  FD_ZERO(&readfds_bak);
  FD_SET(sfd, &readfds_bak);
  FD_SET(0, &readfds_bak); //0：standard input
  //FD_SET(1, &readfds_bak); //1：standard output

  for (temp=0;temp<TEMPMAX;temp++) {
      fd_temp[temp].fd = -1;
	  fd_temp[temp].index = -1;
  }
  
  for(rainbowid=0;rainbowid<RAINBOWMAX;rainbowid++)
      for(cid=0;cid<CLIENTMAX;cid++){
	  	  clearDeviceTable(&device[rainbowid][cid]);
      }

  char auth_ask[AUTHASKLENGTH] = "Who are you?";   
  char auth_rainbow[AUTHRAINBOWLENGTH] = "I'm rainbow ";
  char auth_rainbow_client[AUTHRAINBOWCLIENTLENGTH] = "I'm client, I want to connect ";
  
  while(1){	
    //"readfd" and "timeout" change after every select(), maxfd maybe change.
    readfds = readfds_bak;
    maxfd = updateMaxFD(readfds, maxfd);
    ret = select(maxfd+1, &readfds, NULL, NULL, NULL);
    if(ret == -1){
      perror("select()");
    }
    else if(ret){
      for(curfd=0;curfd<=maxfd;curfd++){
        if(!FD_ISSET(curfd, &readfds)){
          continue;
        }
        if (curfd == 0) {
            ret = read(0, buf1, 1024);
            printf( "this is a test: reserved interface!\n");
            memset(buf1, 0, 1024);
            break;
        }
        if(curfd == sfd){
          //this is cloud server's socket, no read or write, just tcp_accpet.
          for (temp=0;temp<TEMPMAX;temp++) {
              if (fd_temp[temp].index == -1 || fd_temp[temp].fd == -1) {
                  fd_temp[temp].fd = tcp_accept(sfd);  
				  fd_temp[temp].index = temp;
			      break;
			  }
		  } 
		  if (fd_temp[temp].fd == -1) {
              printf("device connect error!\n");
			  break;
		  }  
          printf("device connect success, start confirming device type and authentication...\n");
		  if (AUTHASKLENGTH != write(fd_temp[temp].fd, auth_ask, AUTHASKLENGTH)) {
              perror("write device error");
			  break;
		  }
		  if(fd_temp[temp].fd  > maxfd) maxfd = fd_temp[temp].fd ;
          FD_SET(fd_temp[temp].fd , &readfds_bak);
#if 0
          for(cid=0;cid<10;cid++){
            if(device[cid]==-1){
              device[cid] = tcp_accept(sfd); 
              if(device[cid] == -1) printf("rainbow connect error!\n");
              if(cid == 0) printf("rainbow connected!\n");
              else printf("device[%d] connected!\n", cid);
              if(device[cid] > maxfd) maxfd = device[cid];
              FD_SET(device[cid], &readfds_bak);
              break;
            } 
            if((cid==9) && (device[cid]==-1))
              printf("devices are over, stop access!\n");                   
          }
#endif /* 0 */
        } else {
          //this is device's socket, read and write.
          for (temp=0;temp<TEMPMAX;temp++) {
               if (FD_ISSET(fd_temp[temp].fd, &readfds)) {
                   ret = read(fd_temp[temp].fd, buf1, 1024);
                   printf("%s\n", buf1);
				   if (ret == -1) {
				   	   FD_CLR(fd_temp[temp].fd, &readfds_bak);//clr from readfds
                       perror("server read faile");
					   if (close(fd_temp[temp].fd) == -1) {
                          perror("close device fail"); 
                       }
                       //release one position
                       fd_temp[temp].fd = -1;   
                       fd_temp[temp].index = -1;
					   break;
				   } else if (ret == 0) { //device closed
                       if (close(fd_temp[temp].fd) == -1) {
                          perror("close device fail"); 
                       }
                       FD_CLR(fd_temp[temp].fd, &readfds_bak);//clr from readfds
                       //release one position
                       fd_temp[temp].fd = -1;   
                       fd_temp[temp].index = -1;
                       printf("device disconnected!\n");
					   break;
				   }
				   if (strncmp(buf1, auth_rainbow, AUTHRAINBOWLENGTH)==0 && (ret > AUTHRAINBOWLENGTH)) {
				   	   temp_id = getRainbowId(buf1);
					   if (temp_id == 0) {
                           strcpy(tmp_buff, "device connected fail!");
                           if (0 >= write(fd_temp[temp].fd, tmp_buff, 100)) {
                               perror("write device error");
                           }
                           memset(tmp_buff, 0, 100);
            
					   	   FD_CLR(fd_temp[temp].fd, &readfds_bak);//clr from readfds
                           if (close(fd_temp[temp].fd) == -1) {
                               perror("close device fail"); 
                           }
                           //release one position
                           fd_temp[temp].fd = -1;   
                           fd_temp[temp].index = -1;
                           break;
					   }
                       /*determine whether the rainbow id exits or not*/
                       for(rainbowid=0;rainbowid<RAINBOWMAX;rainbowid++) {
                           if (device[rainbowid][0].Id == temp_id) {
                               printf("device connected fail, the rainbow id is in use!\n");
                               strcpy(tmp_buff, "device connected fail, the rainbow id is in use!");
                               if (0 >= write(fd_temp[temp].fd, tmp_buff, 100)) {
                                   perror("write device error");
                               }
                               memset(tmp_buff, 0, 100);  
                               
                               if (close(fd_temp[temp].fd) == -1) {
                                   perror("close device fail"); 
							   }
                               FD_CLR(fd_temp[temp].fd, &readfds_bak);//clr from readfds
            
                               fd_temp[temp].fd = -1;
						       fd_temp[temp].index = -1;

                               break;
                           }
                       }
                       if (rainbowid < RAINBOWMAX) {//the rainbow id exit, so break
                           break;
                       }
                       /*determine whether the rainbow queue is full or not*/
                       for(rainbowid=0;rainbowid<RAINBOWMAX;rainbowid++) {
                            if (device[rainbowid][0].fd == -1) {
                                device[rainbowid][0].fd = fd_temp[temp].fd;
								device[rainbowid][0].type = DEVICE_RAINBOW;
							    device[rainbowid][0].Id = temp_id;
								device[rainbowid][0].password = temp_id;
								
								fd_temp[temp].fd = -1;
							    fd_temp[temp].index = -1;

                                strcpy(tmp_buff, "rainbow device certification success!");
                                if (0 >= write(device[rainbowid][0].fd, tmp_buff, 100)) {
                                    perror("write device error");
                                }
                                memset(tmp_buff, 0, 100);  
            
								printf("rainbow device certification success!\n");
								break;
							}
					   }
                       if (rainbowid == RAINBOWMAX) {
                           printf("rainbow nummber is full, rainbow device can't connect!\n");
						   strcpy(tmp_buff, "rainbow nummber is full, rainbow device can't connect!");
						   if (0 >= write(fd_temp[temp].fd, tmp_buff, 100)) {
                               perror("write device error");
						   }

							memset(tmp_buff, 0, 100);

                            if (close(fd_temp[temp].fd) == -1) {
                                perror("close device fail"); 
							}
                            FD_CLR(fd_temp[temp].fd, &readfds_bak);//clr from readfds
            
							fd_temp[temp].fd = -1;
						    fd_temp[temp].index = -1;

					   } 
                       break;//for (temp=0;temp<TEMPMAX;temp++) save running time
				   } else if (strncmp(buf1, auth_rainbow_client, AUTHRAINBOWCLIENTLENGTH)==0 && (ret > AUTHRAINBOWCLIENTLENGTH)) {
				       temp_id = getClientId(buf1);//get rainbow id that client wants to connect
					   if (temp_id == 0) {
                           strcpy(tmp_buff, "device connected fail!");
                           if (0 >= write(fd_temp[temp].fd, tmp_buff, 100)) {
                               perror("write device error");
                           }
                           memset(tmp_buff, 0, 100);
            
                           if (close(fd_temp[temp].fd) == -1) {
                               perror("close device fail"); 
                           }
                           FD_CLR(fd_temp[temp].fd, &readfds_bak);//clr from readfds
                           //release one position
                           fd_temp[temp].fd = -1;   
                           fd_temp[temp].index = -1;

                           break;//for (temp=0;temp<TEMPMAX;temp++) 
					   }
                       /*lookup whether the rainbow device that clients want to connect exits or not*/
					   for (rainbowid=0;rainbowid<RAINBOWMAX;rainbowid++) {
                            if (device[rainbowid][0].Id == temp_id) {
                                /*determine if there is empty client position in the rainbow*/
                                for (cid=1;cid<CLIENTMAX+1;cid++) {
                                   if (device[rainbowid][cid].fd == -1) {
                                       device[rainbowid][cid].fd = fd_temp[temp].fd;
								       device[rainbowid][cid].type = DEVICE_CLIENT;
								       device[rainbowid][cid].Id = temp_id;
								       device[rainbowid][cid].password = temp_id;

								       fd_temp[temp].fd = -1;
								       fd_temp[temp].index = -1;
            
                                       strcpy(tmp_buff, "client device connected rainbow success!");
                                       if (0 >= write(device[rainbowid][cid].fd, tmp_buff, 100)) {
                                           perror("write device error");
                                       }
                                       memset(tmp_buff, 0, 100);
            
                                       //printf("fd: %d, Id: %d\n", device[rainbowid][cid].fd, device[rainbowid][cid].Id);
								       printf("client device connected rainbow success!\n");
								       break;
							       }
					            }
								if (cid == CLIENTMAX+1) {
                                    printf("devices connecting rainbow are full, close the device!\n");
									strcpy(tmp_buff, "devices connecting rainbow are full, close the device!");
									if (0 >= write(fd_temp[temp].fd, tmp_buff, 100)) {
                                        perror("write device error.\n");
									}

									memset(tmp_buff, 0, 100);
									
                                    FD_CLR(fd_temp[temp].fd, &readfds_bak);//clr from readfds
									if (close(fd_temp[temp].fd) == -1) {
                                        perror("close device fail!"); 
									}
            
									fd_temp[temp].fd = -1;
								    fd_temp[temp].index = -1;

								} 
                                break;//for (rainbowid=0;rainbowid<RAINBOWMAX;rainbowid++)
							}
					   }
                       if (rainbowid == RAINBOWMAX) {
                           printf("device doesn't find the rainbow id, close the device!\n");
						   strcpy(tmp_buff, "device doesn't find the rainbow id, close the device!");
						   if (0 >= write(fd_temp[temp].fd, tmp_buff, 100)) {
                               perror("write device error");
						   }

							memset(tmp_buff, 0, 100);
                            
                            FD_CLR(fd_temp[temp].fd, &readfds_bak);//clr from readfds
                            if (close(fd_temp[temp].fd) == -1) {
                                perror("close device fail"); 
                            }
            
							fd_temp[temp].fd = -1;
						    fd_temp[temp].index = -1;
					   } 
                       break;//for (temp=0;temp<TEMPMAX;temp++) 
				   } else {
				       printf("unknow device, close it!\n");
                       if (close(fd_temp[temp].fd) == -1) {
                          perror("close device fail!"); 
                       }
					   FD_CLR(fd_temp[temp].fd, &readfds_bak);//clr from readfds
					   fd_temp[temp].fd = -1;
					   fd_temp[temp].index = -1;
				   }
				   break;
			   }
		  }
		  if (temp < TEMPMAX) {
              /**the socket is used to built connection, so break*/
			  break; //for(i=0;i<=maxfd;i++)
		  }
		  /**the socket is used to communicate*/
		  ret = getDoubleId(device, &currainbowid, &curcid, curfd);
		  if (ret < 0) {
		  	  printf("unknown error!\n");
		  	  FD_CLR(curfd, &readfds_bak);//clr from readfds
              break;//for(curfd=0;curfd<=maxfd;curfd++)
		  }
		  ret = serverMessageHandler(device, currainbowid, curcid);
		  if (ret == 0) {//ret == 0 need FD_CLR  ret == -1 not need
              FD_CLR(curfd, &readfds_bak);//clr from readfds
              break;//for(curfd=0;curfd<=maxfd;curfd++)
		  } else if (ret < 0) {
              printf("unknown thing.\n");
			  break;
		  }
#if 0
          for(cid=0;cid<10;cid++) {
            if(device[cid]!=-1 && FD_ISSET(device[cid], &readfds)){//readable
              if(FD_ISSET(device[0], &readfds)) {
                ret = read(device[0], buf1, 1024);
                if(ret == -1)
                  perror("Rainbow Read Failed.");
                else if(ret == 0) {//device close
                  if(close(device[cid]) == -1){
                    perror("close rainbow fail!"); 
                  }
                  FD_CLR(device[cid], &readfds_bak);//clr from readfds
                  device[cid] = -1;//release one position
                  printf("rainbow disconnected!\n");
                } else {//receive data from rainbow	
                  if (ret == HEARTLENGTH && strcmp(buf1, heart_packet)) {//judge if it is heart packet 
                      printf("%s\n", heart_packet);
                      memset(buf1, 0, 1024);
                      break;
                  } else {
                      printf("Rainbow Read: %d bytes read.\n", ret);                
                      if(ack_cid == 0) {
                          memset(buf1, 0, 1024);
                          printf("No device, so don't need to send info.\n");
                          break;
                      }
                
                      if(ret != write(device[ack_cid], buf1, ret))//rainbow ack last device
                          perror("device write error.\n");
                  
                      memset(buf1, 0, 1024);
                      printf("send to device from rainbow success.\n");
                  }
                }
              } else if(FD_ISSET(device[cid], &readfds) && (cid != 0)){
                ret = read(device[cid], buf2, 1024);
                if(ret == -1)
                  perror("device Read Failed.\n");
                else if(ret == 0) {
                  if(close(device[cid]) == -1){
                    perror("close device fail!"); 
                  }
                  FD_CLR(device[cid], &readfds_bak);
                  device[cid] = -1;
                  if(1>=get_accepted_device_num(device, MAXdevice)){
                    ack_cid = 0;	
                  }
                  printf("device[%d] closed!\n", cid);
                } else {	
                  printf("device[%d] Read: %d bytes read.\n", cid, ret);
                  if(ret != write(device[0], buf2, ret))//send to rainbow from last device
                    perror("write Rainbow error.\n");
                    
                  ack_cid = cid;  
                  memset(buf2, 0, 1024);
                  printf("send to rainbow from device[%d] success.\n", cid);  
                }
              } 	                     
            }
          }      
#endif /* 0 */
        }
      }
    } else {
      printf("No data within NULL.\n");
    }
    curfd++;
  }
  close(sfd);
  return 0;
}