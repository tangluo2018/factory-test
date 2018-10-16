#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include "factorytest.h"

int tp_test(int uart_fd, uart_msg *msg)
{
   int fd;
   char buf[16];
   int vsda;
   int vsdl;
   int vint;
   int vrst;
   int vsda_h;
   int vsdl_h;
   int vint_h;
   int vrst_h;
   int ret = -1;
   char result;
   int result_l;
   int result_h;
   //uart_msg msg;
   struct timeval tv;
   unsigned long time_interval = 5 * 1000000;
   unsigned long long int now;
   unsigned long elapse;
   char data;

//while(1){
   //receive_msg(uart_fd, &msg, 8);
   if(msg->order == TP_TEST){
     data = *msg->data;
     if(data == 0xFF){
        //break;
     }
     fd = open("/dev/tp_test", O_RDWR);
     if(fd < 0){
       ALOGE("open tp_test device failed\n");
       return -1;
     }

     gettimeofday(&tv, NULL);
     now = tv.tv_sec * 1000000 + tv.tv_usec;
     while(1){
       memset(buf, 0 , sizeof(buf));
       read(fd, buf, 16);
       memcpy(&vsda, buf, 4);
       memcpy(&vsdl, buf + 4, 4);
       memcpy(&vint, buf + 8, 4);
       memcpy(&vrst, buf + 12, 4);
       ALOGI("vsda: %d, vsdl: %d\n", vsda, vsdl);
       ALOGI("vint: %d, vrst: %d\n", vint, vrst);
       if((vsda == 0) && (vsdl == 0) && (vint == 0) && (vrst == 0)){
          result_l = 0x01;
	  break;
       }else{
          result_l = 0x02;
       }
       usleep(200*1000);
       gettimeofday(&tv, NULL);
       elapse = (tv.tv_sec * 1000000 + tv.tv_usec) - now;
       if(elapse >= time_interval){
          result_l = 0x02;
	  break;
       }
     }
     
     gettimeofday(&tv, NULL);
     now = tv.tv_sec * 1000000 + tv.tv_usec;
     while(1){
        memset(buf, 0, sizeof(buf));
        read(fd, buf, 16);
        memcpy(&vsda_h, buf, 4);
        memcpy(&vsdl_h, buf + 4, 4);
        memcpy(&vint_h, buf + 8, 4);
        memcpy(&vrst_h, buf + 12, 4);
        ALOGI("vsda_h: %d, vsdl_h: %d\n", vsda_h, vsdl_h);
        ALOGI("vint_h: %d, vrst_h: %d\n", vint_h, vrst_h);

       if((vsda_h == 1) && (vsdl_h == 1) && (vint_h == 1) && (vrst_h == 1)){
          result_h = 0x01;
	  break;
       }else{
          result_h = 0x02;
       }
       usleep(200*1000);
       gettimeofday(&tv, NULL);
       elapse = (tv.tv_sec * 1000000 + tv.tv_usec) - now;
       if(elapse >= time_interval){
          result_h = 0x02;
	  break;
       }
     }

     if((result_l == 0x01) && (result_h == 0x01)){
        result = 0x01;
     }else{
        result = 0x02;
     }
     send_msg(uart_fd, TP_TEST, (char*)&result, sizeof(result));
     ret = 0;
   }
   if(msg->data != NULL){
     free(msg->data);
   }
//}
   ALOGI("Tp test end");
   return ret;
}
