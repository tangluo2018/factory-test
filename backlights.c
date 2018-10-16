#include <fcntl.h>
#include <factorytest.h>
#define BACKLIGHT_PATH "/sys/class/backlight/rk28_bl/brightness"

void adjust_backlight(int brightness)
{
   int fd;

   fd = open(BACKLIGHT_PATH, O_RDWR);
   if(fd < 0){
      ALOGI("Open backlight adjust fd failed");
      return;
   }

   char buffer[20];
   int bytes = sprintf(buffer, "%d\n", brightness);
   write(fd, buffer, bytes);
   close(fd);
}
