#include <factorytest.h> 

int mipi_test(int fd, uart_msg *msg)
{
   //uart_msg msg;
   int len;
   char buf[128];
   char data;
   int ret;
   char data_buf[16];
   int data_len = 0;
   char rdata = 0x01;  //return back data

   ret = graphics_init("fb0");
   if(ret < 0){
      ALOGE("Graphics init failed");
      return -1;
   }
  
   //while(1){ 
     //memset(&msg, 0, sizeof(msg));  
     //if((len = read(fd, buf, sizeof(buf))) > 0){
       #if 0
       int i = 0;
       for(i=0; i<len; i++){
          ALOGI("uart4 mipi test: 0x%x", buf[i]);
       }
       #endif

       //ALOGI("len: %d", len);
       //memcpy(data_buf + data_len, buf, len);
       //data_len += len;
       //ALOGI("data_len: %d", data_len);      
       //if(data_len >= 8){
       //   parse_msg(data_buf, &msg);
       //}else{
       //   continue;
       //}
       ALOGI("Test order: %d", msg->order);
       if(msg->order == MIPI_TEST){
          data = *msg->data;
	  ALOGI("Data: %x", data);
	  data_len = 0;
          if(data != 0xFF){
            draw_screen(data);
	    rdata = 0x01;
	    send_msg(fd, MIPI_TEST, (char*)&rdata, sizeof(rdata));   
	  }else{
	    ALOGI("MIPI Test end");
	    send_msg(fd, MIPI_TEST, (char*)&rdata, sizeof(rdata));
            //break;
	  }
       }
     //}
   //}
   graphics_close();
   if(msg->data != NULL){
      free(msg->data);
   }

   return 0;
}

int mipi_hdmi_test(int fd, uart_msg *msg)
{
   //uart_msg msg;
   int len;
   char buf[128];
   char data;
   int ret;
   char data_buf[16];
   int data_len = 0;
   char rdata = 0x01;

   ret = graphics_init("fb4");
   if(ret < 0){
      return -1;
   }
  
   //while(1){ 
   //  memset(&msg, 0, sizeof(msg));  
     //if((len = read(fd, buf, sizeof(buf))) > 0){
      #if 0
       int i = 0;
       for(i=0; i<len; i++){
          ALOGI("uart4 mipi test: 0x%x", buf[i]);
       }
       #endif
      
       //memcpy(data_buf + data_len, buf, len);
       //data_len += len;
       //if(data_len >= 8){
       //  parse_msg(data_buf, &msg);
       //}else{
       //  continue;
       //}
       if(msg->order == MIPI_HDMI_TEST){
          data = *msg->data;
	  data_len = 0;
          if(data != 0xFF){
            draw_screen(data);
	    rdata = 0x01;
	    send_msg(fd, MIPI_HDMI_TEST, (char*)&rdata, sizeof(rdata));
	  }else{
	    ALOGI("MIPI_HDMI Test end");
	    send_msg(fd, MIPI_HDMI_TEST, (char*)&rdata, sizeof(rdata));
            //break;
	  }
       }
     //}
   //}
   graphics_close();
   if(msg->data != NULL){
      free(msg->data);
   }

   return 0;
}
