#include <string.h>
#include <log/log.h>

#define LOG_TAG "FactoryTest"

#define UART_TTYS1   "/dev/ttyS1"
#define UART_TTYS3   "/dev/ttyS3"
#define UART_TTYS4   "/dev/ttyS4"

/*uart protocal order*/
#define MIPI_TEST          0x01
#define MIPI_HDMI_TEST     0x02
#define MIC_SPDIF_TEST     0x03
#define MEMORY_TEST        0x04
#define WIFI_TEST          0x05
#define EMMC_TEST          0x06
#define UART_TEST          0x07
#define CAMERA_TEST        0x08
#define TP_TEST            0x09
#define USB_TEST           0x10
#define WAN_TEST           0x11

/*MIPI and HDMI test color*/
#define MSG_RED    0x01
#define MSG_GREEN  0x02
#define MSG_BLUE   0x03
#define MSG_WHITE  0x04
#define MSG_BLACK  0x05

typedef struct uart_msg_s{
  unsigned char   head_l;
  unsigned char   head_h;
  unsigned char   dlen;
  unsigned char   order;
  char*           data;
  unsigned char   check;
  unsigned char   tail_l;
  unsigned char   tail_h;
}uart_msg;

/*MIPI and HMDI test*/
int mipi_hdmi_test(int, uart_msg*);

int mipi_test(int, uart_msg*);

int memory_test(int, uart_msg*);

int emmc_test(int, uart_msg*);

int wifi_test(int, uart_msg*);

int uart_test(int, uart_msg*);

int tp_test(int, uart_msg*);

void adjust_backlight(int);

void init_uart(int);

int graphics_init(char*);

void graphics_close();

void parse_msg(void*, void*);

void send_msg(int, char, char*, int);

void receive_msg(int, void*, int);

void draw_screen(char);

int camera_test(int, uart_msg*);


//#ifdef __cplusplus
//extern "C" {
//#endif
extern int camera_preview();
extern int camera_save_picture();
extern void camera_preview_stop();
//#ifdef __cplusplus
//}
//#endif
