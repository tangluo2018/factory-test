#include <tinyalsa/asoundlib.h>
#include "../factorytest.h"

#define MIC_INIT   0x01
#define SPDIF_OUT  0x02
#define CHECK_CHN  0x03
#define TEST_END   0xFF

#define TEST_AUDIO_FILE  "/system/usr/share/8k.raw"

static struct pcm* pcm;
static struct pcm_config config;
static char* buffer;
static unsigned int size;

static void set_volume(void *data, int size, int ratio)
{
   short sample;
   short *snd_data;
   int i;

   snd_data = (short*)data;
   for(i=0; i<size/2; i++){
      snd_data[i] = snd_data[i] >> ratio;
   }
}

int audio_card_init()
{
   config.channels = 2;
   config.rate = 8000;
   config.format = PCM_FORMAT_S16_LE;
   config.start_threshold = 0;
   config.stop_threshold = 0;
   config.silence_threshold = 0;
   config.period_size = 1536;
   config.period_count = 8;

   pcm = pcm_open(1, 0, PCM_OUT, &config);
   if(!pcm || !pcm_is_ready(pcm)){
      ALOGE("Unable to open playback PCM device (%s)", pcm_get_error(pcm));
      return -1;
   }

   size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
   buffer = malloc(size);
   if(!buffer){
      ALOGE("Malloc buffer failed");
      pcm_close(pcm);
      return -1;
   }
   return 0;
}

int audio_play()
{
   FILE *fp;
   unsigned int num_read;
    
   fp = fopen(TEST_AUDIO_FILE, "rb");
   if(!fp)
      return-1;

   audio_card_init();

   while(1){
      num_read = fread(buffer, 1, size, fp);
      if(num_read > 0){
         set_volume(buffer, size, 4);
         if(pcm_write(pcm, buffer, num_read)){
            ALOGE("Error playing test sample");
            break;
	 }
      }else{
         break;
      }
   }
   fclose(fp);
   return 0;
}

int mic_spdif_test(int uart_fd)
{
   int ret;
   char data;
   uart_msg msg;
   
   audio_play();

   return 0;
}

int main()
{
   mic_spdif_test(NULL);
   sleep(1);
   pcm_close(pcm);
   return 0;
}
