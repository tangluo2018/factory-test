#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include "factorytest.h"

#define PING_RESULTS "/data/ping_results"
#define SCAN_RESULTS "/data/scan_results"
#define BUF_SIZE 4096
#define MAX_AP_COUNT 64
#define MAX_ESSID_LEN 128

#define DEFAULT_ESSID "\"wifi_test\""
 
int dbm = 0xFF;

static int get_ping_results()
{
   int fd;
   int ret = 0;
   char buf[512];
   char *p;
   int k = 0;
   char temp[4];
   int loss;
  
   fd = open(PING_RESULTS, O_RDONLY);
   if(fd < 0){
     printf("open ping_results file failed\n");
     return -1;
   }
   ret = read(fd, buf, 512);
   if(ret < 0){
      printf("read ping_results file failed\n");
      return -1;
   }

   p = strstr(buf, "packets received");
   if(p == NULL){
      close(fd);
      return -1;
   }

   p += strlen("packets received, ");

   while(*p != '%'){
     if((*p != '\0') || (*p != '\n')){
         temp[k++] = *p++;
     }
   }
   loss = atoi(temp);
   printf("%d%% packets loss\n", loss);
   if(loss < 100){
      ret = 0;
   }else{
      close(fd);
      ret =  -1;
   }
   close(fd);
   
   return ret;
}

static int process_ssid_signal(char* src)
{
   int ret = 0;
   int i;
   char ssids[MAX_AP_COUNT][MAX_ESSID_LEN];
   char *ssid, *signal;
   char *temp_ssid, *temp_signal;

   ALOGI("process ssid and signal ...\n");

   for(i=0; i<MAX_AP_COUNT; i++){
      temp_ssid = &ssids[i][0];
      ssid = strstr(src, "ESSID:");
      if(ssid == NULL){
         ret = -1;
	 break;
      }
      ssid += strlen("ESSID:");
      while((*ssid != '\0') && (*ssid != '\n')){
         *temp_ssid++ = *ssid++;
      }
      *temp_ssid++ = '\0';

      temp_signal = &ssids[i+1][0];
      signal = strstr(src, "Signal level:");
      if(signal == NULL){
         ret = -1;
	 break;
      }
      signal += strlen("Signal level:");
      *temp_signal++ = *signal++;
      *temp_signal++ = *signal++;
      *temp_signal++ = *signal++;
      *temp_signal++ = '\0';

      ALOGI("ESSID: %s, Signal level: %s\n", &ssids[i], &ssids[i+1]);
      if(!strcmp(&ssids[i], DEFAULT_ESSID)){
          //temp_signal += 1;  /*skip '-' (like -80 dBm)*/
	  temp_signal = temp_signal - 3;
          dbm = atoi(temp_signal);
	  ALOGI("temp_singal %s, dbm: %d", temp_signal, dbm);
          if(dbm < 90){
             ret = 1;
	     ALOGI("wifi test successfully\n");
	     break;
	  }else{
             ret = -1;
	  }
      }
      i = i+1; 
      src = signal;
   }

   return ret;
}

int wifi_test(int uart_fd, uart_msg* msg)
{
  int ret = 0;
  int fd;
  pid_t pid;
  int status;
  char *buf;
  char *p;
  char *temp;
  char ssid[MAX_AP_COUNT][MAX_ESSID_LEN];
  int k = 0;
  int has = 0;
  //uart_msg msg;
  char data;

//while(1){
  //receive_msg(uart_fd, &msg, 8);
  if(msg->order == WIFI_TEST){
    data = *msg->data;
    if(data == 0xFF){
       //break;
    }
    status = system("sh /system/usr/share/wifi_scan.sh");
    //waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 1) {   
      ALOGE("Error(Status %d)\n", WEXITSTATUS(status));
      return -1;  
    } 

    fd = open(SCAN_RESULTS, O_RDONLY);
    if(fd < 0){
      ALOGE("open scan_resuts file failed\n");
      return -1;
    }
    buf = malloc(BUF_SIZE);
    if(buf == NULL){
      ALOGE("malloc failed\n");
      return -1;
    }
    memset(buf, 0, BUF_SIZE);
    ret = read(fd, buf, BUF_SIZE);
    if(ret < 0){
      ALOGE("read scan_results file failed\n");
      return -1;
    }
    memset(ssid, 0, sizeof(ssid));

    ret = process_ssid_signal(buf);
    ALOGI("wifi signal: %d", dbm);
    send_msg(uart_fd, WIFI_TEST, (char*)&dbm, sizeof(dbm));
    free(buf);
  }
//}

  ALOGI("Wifi test end");
  /*
  while((*buf != '\0')&&(k < 32)){
    p = strstr(buf, "ESSID:");
    if(p == NULL){
      printf("no ap found\n");
      break;
    }
    temp = &ssid[k][0];
    p += strlen("ESSID:");
    //printf("p:%s\n", p);
    while((*p != '\0') && (*p != '\n')){
      *temp++ = *p++;
    }
    *temp++ = '\0';
    buf = p;
    printf("ap:%s\n", &ssid[k]);
    ui_print("%s\n", &ssid[k]);
    
    if(!strcmp(&ssid[k], DEFAULT_ESSID)){
        has = 1;
    }
    k++;
  }
  close(fd);

  ui_print("\n");

  if(!has || !k){
    ui_print("%s\n", NO_CONNECTABLE_AP);
    return -1;
  }

  ui_print("%s\n", WIFI_CONNECTING);
  status = system("sh system/usr/share/wifi_connect.sh");                                                                                                               
  if(!WIFEXITED(status) || WEXITSTATUS(status) != 0){                                                                                                                   
      printf("connect shell script failed\n");                                                                                                                            
      return -1;                                                                                                                                                          
  }

  ret = get_ping_results();
  */

  return ret;
}
