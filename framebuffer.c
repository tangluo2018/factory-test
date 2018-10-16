#include <stdio.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "factorytest.h"

static struct fb_var_screeninfo fb_var;
static struct fb_fix_screeninfo fb_fix;
static short GRAPHICS_INITIED = 0;

static int xres;
static int yres;
unsigned short *fb_mem;
static int fd;
static int width;
static int height;

int graphics_init(char *device)
{
    char fb_dev[24];
    sprintf(fb_dev, "%s%s", "/dev/graphics/", device);
    ALOGI("fb_dev: %s", fb_dev);
    fd = open(fb_dev, O_RDWR);
    if(fd < 0){
      ALOGE("Open %s failed\n", fb_dev);
      return -1;
    }

    if(ioctl(fd, FBIOGET_VSCREENINFO, &fb_var) < 0){
       ALOGE("Get var_screeninfo failed\n");
       close(fd);
       return -1;
    }

           #if 1
           fb_var.bits_per_pixel = 16;
           fb_var.red.offset    = 0;
           fb_var.green.offset  = 5;
           fb_var.blue.offset   = 11;
           fb_var.red.length    = 5;
           fb_var.green.length  = 6;
           fb_var.blue.length   = 5;
     
           fb_var.blue.msb_right = 0;
           fb_var.green.msb_right = 0;
          fb_var.red.msb_right = 0;
           fb_var.transp.offset = 0;
           fb_var.transp.length = 0;
           fb_var.vmode = FB_VMODE_NONINTERLACED;
           fb_var.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
           if(ioctl(fd, FBIOPUT_VSCREENINFO, &fb_var) < 0){
              printf("failed to put var_screeninfo info\n");
              close(fd);
              return -1;
           }
         #endif


	xres = fb_var.xres;
	yres = fb_var.yres;
	width = fb_var.xres;
	height = fb_var.yres;

    if(ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix) < 0){
      ALOGE("Get fix_screeninfo failed\n");
      close(fd);
      return -1;
    }
    ALOGE("fixscreeninfo smem_len: %d\n", fb_fix.smem_len);
    GRAPHICS_INITIED = 1;
    fb_mem = mmap(0, fb_fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(fb_mem == MAP_FAILED){
       ALOGE("mmap fd failed");
       close(fd);
       return -1;
    }
    return fd;
}

void graphics_close()
{
   close(fd);
   //free(fb_mem);
}

void draw_background(int r, int g, int b)
{
   ALOGI("draw_background");
   int w, h;
   for(h=0; h<height; h++){
     for(w=0; w<width; w++){
        fb_mem[width*h+w] = ((r>>3)<<11)|((g>>2)<<5)|((b>>3));
     }
   }
}

void draw_screen(char color)
{
    ALOGI("draw_screen, color: 0x%x", color);
    switch(color){
      case MSG_RED:
        draw_background(255, 0, 0);        
        break;
      case MSG_GREEN:
        draw_background(0, 255, 0);
	break;
      case MSG_BLUE:
        draw_background(0, 0, 255);
	break;
      case MSG_WHITE:
        draw_background(255, 255, 255);
        break;
      case MSG_BLACK:
        draw_background(0, 0, 0);
	break;
      default:
        break;
    }
}
