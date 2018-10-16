#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "factorytest.h"

int usb_test(int uart_fd, uart_msg *msg)
{
   //uart_msg msg;
   char data;
   int status;

   char *usb_id = " ID";
   char *delim = ":";
   int count = 0;
   char result;

//while(1){
   //receive_msg(uart_fd, &msg, 8);
   if(msg->order == USB_TEST){
       data = *msg->data;
       if(data != 0xFF){
          FILE *fp = NULL;
	  int num_read;
	  char buf[1024];
	  char *info;
	  memset(buf, 0, 1024);
	  status = system("busybox lsusb > /data/usb_ports");
	  //if(!WIFEXITED(status) || WEXITSTATUS(status) != 1){
          //   ALOGE("Error(Status %d)", WEXITSTATUS(status));
	  //   return -1;
	  //}
	  fp = fopen("/data/usb_ports", "rb");
	  if(!fp){
             ALOGE("Open /data/usb_ports faile");
	     return -1;
	  }
	  num_read = fread(buf, 1, 1024, fp);
	  if(num_read > 0){
             info = strtok(buf, delim);
	     ALOGI("INFO: %s", info);
	     while(info){
                if(!strncmp(usb_id, info, 3)){
		   ALOGI("usb_id: %s", info);
		   info = strtok(NULL, delim);
                   count++;
		}else{
                   info = strtok(NULL, delim);
		}
		
	     }
             if(count >= 5){
                 result = 0x01;
	     }else{
                 result = 0x02;
	     }
	     ALOGI("Usb count: %d", count);
	     count = 0;
	  }
	  send_msg(uart_fd, USB_TEST, (char*)&result, sizeof(result));
       }else{
          //break;
       }
   }
//}
  ALOGI("Usb test end");
  return 0;
}

