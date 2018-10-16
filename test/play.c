#include <stdio.h>
#include <tinyalsa/asoundlib.h>

#define AUDIO_FILE "/etc/1k.pcm"

void audio_play()
{
   struct pcm_config config;
   FILE *fp;
   struct pcm* pcm;
   char* buffer;
   int size;
   int num_read = 0;

   config.channels = 2;
   config.rate = 8000;
   config.format = PCM_FORMAT_S16_LE;
   config.start_threshold = 0;
   config.stop_threshold = 0;
   config.silence_threshold = 0;
   config.period_size = 1024;
   config.period_count = 4;

   printf("pcm open\n");
   pcm = pcm_open(1, 0, PCM_OUT, &config);
   if(!pcm || !pcm_is_ready(pcm)){
       printf("Unable to open playback PCM device (%s)", pcm_get_error(pcm));
       return;
   }
   size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
   buffer = malloc(size);
   if(!buffer){
      printf("Malloc buffer failed\n");
      free(buffer);
      pcm_close(pcm);
      return;
   }

   fp = fopen(AUDIO_FILE, "rb");
   if(!fp){
      printf("Open file failed\n");
   }

   printf("audio play\n");
   while(1){
     num_read = fread(buffer, 1, size, fp);
     if(num_read > 0){
        if(pcm_write(pcm, buffer, num_read)){
            printf("Error playing sample\n");
	    break;
	}
     }else{
        break;
     }
   }
   fclose(fp);
   pcm_close(pcm);
   free(buffer);
}


int main()
{
   printf("play ....\n");
   audio_play();
   printf("play end ...\n");
   return 0;
} 
