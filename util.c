#include "factorytest.h"
#include <termios.h>

void parse_msg(void *src, void *dst)
{
   ALOGI("parse_msg");
   uart_msg *msg;
   char * buf = (char*)src;
   msg = (uart_msg*)dst;
   memcpy(&(msg->head_l), buf, 1);
   //ALOGI("heal_l: 0x%x", msg->head_l);
   memcpy(&(msg->head_h), buf + 1, 1);
   memcpy(&(msg->dlen), buf + 2, 1);
   memcpy(&(msg->order), buf + 3, 1);
   //ALOGI("head_h %x order %x len: %x", msg->head_h, msg->order,  msg->dlen);
   msg->data = (char*)malloc(msg->dlen);
   memset(msg->data, 0, msg->dlen);
   memcpy(msg->data, buf + 4, msg->dlen);
   //ALOGI("");
   memcpy(&(msg->check), buf + 4 + msg->dlen, 1);
   memcpy(&(msg->tail_l), buf + 4 + msg->dlen + 1, 1);
   //ALOGI("");
   memcpy(&(msg->tail_h), buf + 4 + msg->dlen + 2, 1);

   ALOGI("Data: %x %x %x %x %s %x %x %x", 
         msg->head_l, 
	 msg->head_h,
         msg->dlen, 
	 msg->order, 
	 msg->data, 
	 msg->check, 
	 msg->tail_l, 
	 msg->tail_h);
}

void init_uart(int fd)
{
   struct termios terminfo;

   memset(&terminfo, 0, sizeof(struct termios));
   tcgetattr(fd, &terminfo);
   cfmakeraw(&terminfo);
   cfsetospeed(&terminfo, B9600);
   cfsetispeed(&terminfo, B9600);
   tcsetattr(fd, TCSANOW, &terminfo);
}

void init_uart_115200(int fd)
{
   struct termios terminfo;

   memset(&terminfo, 0, sizeof(struct termios));
   tcgetattr(fd, &terminfo);
   cfmakeraw(&terminfo);
   cfsetospeed(&terminfo, B115200);
   cfsetispeed(&terminfo, B115200);
   tcsetattr(fd, TCSANOW, &terminfo);
}

void wrap_msg(void *dst, char order, char *data, char dlen)
{
   char *buf = (char *)dst;
   char check;
   char dcheck = 0x00;
   int i = 0;
   char head_l = 0x44;
   char head_h = 0x57;
   char tail_l = 0x0D;
   char tail_h = 0x0A;

   memcpy(buf, &head_l, 1);
   memcpy(buf + 1, &head_h, 1);
   memcpy(buf + 2, &dlen, 1);
   memcpy(buf + 3, &order, 1);
   memcpy(buf + 4, data, dlen);
   for(i; i<dlen; i++){
      dcheck += data[i];
   }
   check = dcheck + order;
   memcpy(buf + 4 + dlen, &check, 1);
   memcpy(buf + 4 + dlen + 1, &tail_l, 1);
   memcpy(buf + 4 + dlen + 2, &tail_h, 1);
}

void receive_msg(int fd, void *data, int msg_len)
{
   uart_msg *msg;
   char buf[128];
   int len;
   char data_buf[16];
   int data_len = 0;
   msg = (uart_msg *)data;
   memset(msg, 0, sizeof(uart_msg));
   while(1){
      if((len = read(fd, buf, sizeof(buf))) > 0){
         memcpy(data_buf + data_len, buf, len);
	 data_len += len;
	 if(data_len >= msg_len){
           parse_msg(data_buf, msg);
	 }else{
           continue;
	 }
	 break;
      }
   }
}

void send_msg(int fd, char order, char *data, int dlen)
{
   uart_msg msg;
   char buf[128];
   
   memset(buf, 0, sizeof(buf));
   wrap_msg(buf, order, data, dlen);  
   write(fd, buf, dlen + 7);
}
