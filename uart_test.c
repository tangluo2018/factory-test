#include "factorytest.h"
#include <pthread.h>
#include <fcntl.h>

static int uart1_fd;
static int uart3_fd;
static int uart1_result = 0;
static int uart3_result = 0;

void* process_uart1_send(void* data)
{
  char test = 'T';
  sleep(1);
  send_msg(uart1_fd, UART_TEST, (char*)&test, sizeof(test));
  ALOGI("uart1 send msg end");
  return NULL;
}

void* process_uart1_recv(void* data)
{
   uart_msg msg;
   receive_msg(uart1_fd, &msg, 8);
   ALOGI("uart1 order: %d", msg.order);
   if(msg.order == UART_TEST){
      uart1_result = 1;
   } 
   return NULL;
}

void* process_uart3_send(void* data)
{
  char test = 'T';
  sleep(1);
  send_msg(uart3_fd, UART_TEST, (char*)&test, sizeof(test));
  return NULL;
}

void* process_uart3_recv(void* data)
{
   uart_msg msg;
   receive_msg(uart3_fd, &msg, 8);
   if(msg.order == UART_TEST){
      uart3_result = 1;
   }
   return NULL;
}

int uart_test(int uart4_fd, uart_msg *msg)
{
  int ret = 0;
  pthread_t uart1_send;
  pthread_t uart1_recv;
  pthread_t uart3_send;
  pthread_t uart3_recv;
  //uart_msg msg;
  char data;

//while(1){
  //receive_msg(uart4_fd, &msg, 8);
  if(msg->order == UART_TEST){
     data = *msg->data;
     if(data == 0xFF){
        //break;
     }
     /*Uart1 test*/
     uart1_fd = open(UART_TTYS1, O_RDWR);
     if(uart1_fd < 0){
        ALOGE("Open %s failed", UART_TTYS1);
	return -1;
     }

     uart3_fd = open(UART_TTYS3, O_RDWR);
     if(uart3_fd < 0){
        ALOGE("Open %s failed", UART_TTYS3);
	return -1;
     }

     init_uart(uart1_fd);
     init_uart_115200(uart3_fd);
     ALOGI("Create uart test pthreads");
     pthread_create(&uart1_send, NULL, process_uart1_send, NULL);
     pthread_create(&uart1_recv, NULL, process_uart1_recv, NULL);
     pthread_create(&uart3_send, NULL, process_uart3_send, NULL);
     pthread_create(&uart3_recv, NULL, process_uart3_recv, NULL);

     pthread_join(uart1_send, NULL);
     pthread_join(uart1_recv, NULL);
     pthread_join(uart3_send, NULL);
     pthread_join(uart3_recv, NULL);
     ALOGI("%s, Uart phtreads end", __FUNCTION__);
     char result;
     if(uart1_result == 1){
        if(uart3_result == 1){
           result = 0x01;
	}else{
           result = 0x03;
	}
     }else{
        if(uart3_result == 1){
           result = 0x02;
	}else{
           result = 0x04;
	}
     }
     send_msg(uart4_fd, UART_TEST, (char*)&result, sizeof(result));
  }
//}
  ALOGI("Uart test end");
  return ret;
}
