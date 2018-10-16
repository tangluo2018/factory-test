#include <stdio.h>
#include <stdlib.h>
#include "factorytest.h"
/*
* memory test, read total mem size
*/


#define TOTAL_SIZE 2000000

/*delete space*/
void delsp(char *src)
{
  char *tmp;
  tmp = src;

  while(*src){
    if(*src != ' '){
     *tmp = *src;
     tmp++;
    }
    src++;
  }
  *tmp = '\0';
}

int parse_tsize(char *info)
{
    int tsize = 0;
    char *str;
    char *tmp = strstr(info, "kB");
    int len = tmp - info;

    str = (char *)malloc(len + 1);
    if(!str){
      ALOGE("Malloc failed\n");
      return -1;
    }
    strncpy(str, info, len);
    delsp(str);
    tsize = atoi(str);
    free(str);
    return tsize;
}

int memory_test(int uart_fd, uart_msg *msg)
{
     int ret = 1;
     char buf[1024];
     char *mfile = "/proc/meminfo";
     FILE *fd;
     int num_read;
     char *info;
     char *delim = ":";
     char *total = "MemTotal";
     int  tsize = 0;
     //uart_msg msg;
     int len;
     char dbuf[128];
     char data;
 
   //while(1){
     //receive_msg(uart_fd, &msg, 8);
     if(msg->order == MEMORY_TEST){
        data = *msg->data;
	if(data == 0xFF){
           //break;
	}
        fd = fopen(mfile, "rb");
        if(!fd){
	   ALOGE("Open %s failed", mfile);
           return -1;
        }

        memset(buf, 0, 1024);

        num_read = fread(buf, 1, 1024, fd);
        if(num_read > 0){
          info = strtok(buf, delim);
	  
	  while(info){
             if(!strcmp(total, info)){
               info = strtok(NULL, delim);
	       tsize = parse_tsize(info);
	       ALOGI("memory total size: %d\n", tsize);
               send_msg(uart_fd, MEMORY_TEST, (char*)&tsize, sizeof(tsize)); 
	       break;
	    }else{
              info = strtok(NULL, delim);
	    }
	  }
       }        
     }
   //}
     ALOGI("Memory test end");
     return 0;
}
