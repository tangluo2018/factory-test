#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

int main()
{
   int fd;
   char buf[16];
   int vsda;
   int vsdl;
   int vint;
   int vrst;

   fd = open("/dev/tp_test", O_RDWR);
   if(fd < 0){
      printf("open tp_test device failed\n");
      return -1;
   }
   memset(buf, 0 , sizeof(buf));
   read(fd, buf, 16);
   printf("buf: %s\n", buf);
   memcpy(&vsda, buf, 4);
   memcpy(&vsdl, buf + 4, 4);
   memcpy(&vint, buf + 8, 4);
   memcpy(&vrst, buf + 12, 4);
   printf("vsda: %d, vsdl: %d\n", vsda, vsdl);
   printf("vint: %d, vrst: %d\n", vint, vrst);
   return 0;
}
