#include <stdio.h>
#include "factorytest.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
  adjust_backlight(200);

  ALOGI("***********************************************");
  ALOGI("Factory Test");
  ALOGI("***********************************************");
  int ret = 0;
  int fd;
  uart_msg msg;
  unsigned char order;

  fd = open(UART_TTYS4, O_RDWR);
  if(fd < 0){
    ALOGE("Open %s failed", UART_TTYS4);
    goto end;
  }
  init_uart(fd);

  while(1){
     receive_msg(fd, &msg, 8);
     order = msg.order;
     switch(order){
        case MIPI_TEST:
	   mipi_test(fd, &msg);
	   break;
	case MIPI_HDMI_TEST:
	   mipi_hdmi_test(fd, &msg);
	   break;
        case MIC_SPDIF_TEST:
	   mic_spdif_test(fd, &msg);
	   break;

	case MEMORY_TEST:
	   memory_test(fd, &msg);
	   break;
	case WIFI_TEST:
	   wifi_test(fd, &msg);
	   break;
	case EMMC_TEST:
	   emmc_test(fd, &msg);
	   break;
	case UART_TEST:
	   uart_test(fd, &msg);
           break;
	case CAMERA_TEST:
	   camera_test(fd, &msg);
	   break;
	case TP_TEST:
	   tp_test(fd, &msg);
	   break;
	case USB_TEST:
	   usb_test(fd, &msg);
           break;
	default:
	   ALOGI("Wrong test order");
	   break;

     }
  }

  #if 0
  ALOGI("Mipi test");
  ret = mipi_test(fd);
  if(ret < 0){
     ALOGI("Mipi test failed");
  }
  #endif

  #if 0
  ALOGI("Mipi hdmi test");
  ret = mipi_hdmi_test(fd);
  if(ret < 0){
    ALOGI("Mipi hdmi test failed");
  }
  #endif

  #if 0
  ALOGI("Memory test");
  ret = memory_test(fd);
  if(ret < 0){
     ALOGI("Memory test failed");
  }

  ALOGI("Wifi test");
  ret = wifi_test(fd);
  if(ret < 0){
     ALOGI("Wifi test failed");
  }

  ALOGI("Emmc test");
  ret = emmc_test(fd);
  if(ret < 0){
     ALOGI("Emmc test failed");
  }

  ALOGI("Uart test");
  ret = uart_test(fd);
  if(ret < 0){
     ALOGI("Uart test failed");
  }

  ALOGI("Tp test");
  ret = tp_test(fd);
  if(ret < 0){
     ALOGI("Tp test failed");
  }

  ALOGI("Usb test");
  ret = usb_test(fd);
  if(ret < 0){
    ALOGI("Usb test failed");
  }
  #endif

  #if 0
  ALOGI("Mic spdif test");
  ret = mic_spdif_test(fd);
  if(ret < 0){
    ALOGI("Mic spdif test failed");
  }

  //ALOGI("Camera test");
  //...

  ALOGI("***********************************************");
  ALOGI("Factory Test end");
  ALOGI("***********************************************");
  #endif
  return 0;

  end:
     ALOGE("Factory Test Failed!!!");
     return -1;
}
