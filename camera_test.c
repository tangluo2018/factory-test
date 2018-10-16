#include "factorytest.h"

pthread_t preview_t;
volatile int isThread = 0;

int camera_test(int fd, uart_msg *msg)
{
     char result;
     char data;
    if(msg->order == CAMERA_TEST){
         data = *msg->data;
         if(data == 0xFF){
	    /*
            camera_preview_stop();
	    result = 0x01;
	    send_msg(fd, CAMERA_TEST, (char*)&result, sizeof(result));
            isThread = 0;
	    return 0;
	    */
	 }else{
	     result = 0x01;
	     //send_msg(fd, CAMERA_TEST, (char*)&result, sizeof(result));
	     //if(isThread){
	     //  ALOGI("preview is running");
	     //  return 0;
	     //}
	     //pthread_create(&preview_t, NULL, camera_preview, NULL);
             //camera_preview();
	     camera_save_picture();
	     send_msg(fd, CAMERA_TEST, (char*)&result, sizeof(result));
	     //isThread = 1;
	 }
     }

     return 0;
}
