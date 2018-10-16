#include <tinyalsa/asoundlib.h>
#include <pthread.h>
#include <math.h>
#include "../audio_queue.h"
#include "fftw3.h"

static struct pcm* check_pcm;
static struct pcm_config check_config;
static char* check_buffer;
static unsigned int check_size;

static int chn_end = 0;
static audio_queue queue;
static double A1;
static double A2;
static double A3;
static double A4;
static double a1, a2, a3, a4;

static int audio_record()
{
   int ret;

   check_config.channels = 2;
   check_config.rate = 64000;
   check_config.format = PCM_FORMAT_S32_LE;
   check_config.start_threshold = 0;
   check_config.stop_threshold = 0;
   check_config.silence_threshold = 0;
   check_config.period_size = 1536;
   check_config.period_count = 8;

   check_pcm = pcm_open(0, 0, PCM_IN, &check_config);
   if(!check_pcm || !pcm_is_ready(check_pcm)){
      printf("Unable to open recored PCM device (%s)", pcm_get_error(check_pcm));
      return -1;
   }

   check_size = pcm_frames_to_bytes(check_pcm, pcm_get_buffer_size(check_pcm));
   audio_queue_init(&queue);
   check_buffer = malloc(check_size);
   if(check_buffer){
      memset(check_buffer, check_size, 0);
   }else{
      printf("Malloc check_buffer failed");
      return -1;
   }

   while(1){
      if(chn_end){
         printf("audio record end\n");
	 break;
      }
      ret = pcm_read(check_pcm, check_buffer, check_size);
      if(ret){
         printf("Record pcm read data failed, ret %d", ret);
	 break;
      }

      audio_queue_put(&queue, check_buffer, check_size);
   }
   return ret;
}

static int channel_check()
{
   audio_buffer buf;
   unsigned int tsize;        /*total size of audio data*/
   char *tbuf;
   unsigned int pos = 0;
   char *data;
   unsigned int i, j, N, f;
   double *abuf;
   unsigned int chn_size = 0;
   unsigned int index = 0;
   unsigned int tmp_size = 0;
   unsigned int fwd = 0;
   fftw_complex *in, *out;
   fftw_plan p;
   FILE *fp, *fp1, *fp2, *fp3, *fp4, *fp5, *fp6, *fp7, *fp8;

   printf("channel_check\n");
   fp = fopen("/sdcard/channel.pcm", "wb+");
   fp1 = fopen("/sdcard/channel1.pcm", "wb+");
   fp2 = fopen("/sdcard/channel2.pcm", "wb+");
   fp3 = fopen("/sdcard/channel3.pcm", "wb+");
   fp4 = fopen("/sdcard/channel4.pcm", "wb+");
   fp5 = fopen("/sdcard/channel5.pcm", "wb+");
   fp6 = fopen("/sdcard/channel6.pcm", "wb+");
   fp7 = fopen("/sdcard/channel7.pcm", "wb+");
   fp8 = fopen("/sdcard/channel8.pcm", "wb+");


   tsize = queue.size;
   tbuf = (char*)malloc(tsize);
   if(!tbuf){
      printf("channel check, Malloc tbuf failed\n");
      return -1;
   }
   memset(tbuf, 0 , tsize);

   chn_size = tsize/8;        /*8 channel*/
   char audioData[8][chn_size];
   printf("tsize: %d, chn_size: %d\n", tsize, chn_size);
   memset(audioData, 0, tsize);
   while(1){
      if(queue.size <= 0)
          break;
      audio_queue_get(&queue, &buf);
      fwrite(buf.data, buf.size, 1, fp);
      index = 0;
      data = (char*)(buf.data);
      tmp_size = buf.size;
      printf("buf size: %d\n", tmp_size);
      while(tmp_size > 0){
        memcpy(audioData[0] + fwd,  data + index, 4);
        memcpy(audioData[1] + fwd,  data + index + 4, 4);
        memcpy(audioData[2] + fwd,  data + index + 8, 4);
        memcpy(audioData[3] + fwd,  data + index + 12, 4);
        memcpy(audioData[4] + fwd,  data + index + 16, 4);
	memcpy(audioData[5] + fwd,  data + index + 20, 4);
        memcpy(audioData[6] + fwd,  data + index + 24, 4);
        memcpy(audioData[7] + fwd,  data + index + 28, 4);
	index += 32;
	tmp_size -= 32;
	fwd += 4;
      }
      printf("%d ", fwd);
      audio_data_free(&buf);
   }
   fwrite(audioData[0], chn_size, 1, fp1);
   fwrite(audioData[1], chn_size, 1, fp2);
   fwrite(audioData[2], chn_size, 1, fp3);
   fwrite(audioData[3], chn_size, 1, fp4);
   fwrite(audioData[4], chn_size, 1, fp5);
   fwrite(audioData[5], chn_size, 1, fp6);
   fwrite(audioData[6], chn_size, 1, fp7);
   fwrite(audioData[7], chn_size, 1, fp8);

   N = chn_size >> 2;
   abuf = (double*)malloc(sizeof(double)*N);

for(j=0; j<8; j++){
   memset(abuf, 0, sizeof(double)*N);
   int *tmpData = audioData[j];
   for(i=0; i<N; i++){
      abuf[i] = tmpData[i]*1.0/0x7fffffff;         /*int to double*/
   }
   
   in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
   out = in;

   for(i=0; i<N; i++){
      in[i][0] = abuf[i];
      in[i][1] = 0.0;
   }
   
   p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    
   fftw_execute(p);

   /*Check 50Hz 100Hz 150Hz*/
   f = 50 * N/16000;
   a1 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   printf("%d: [%d]A1: %f\n", f, j, a1);
   f = 100 * N/16000;
   a2 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   printf("%d: [%d]A2: %f\n", f, j, a2);
   f = 150 * N/16000;
   a3 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);  
   printf("%d: [%d]A3: %f\n", f, j, a3);
   
   fftw_destroy_plan(p);
   fftw_free(in);
}
   free(tbuf);
   free(abuf);
   return 0;
}


int main()
{
   int ret = -1;
   pthread_t chk_pid;

   pthread_create(&chk_pid, NULL, audio_record, NULL);
   printf("before sleep\n");
   sleep(2);
   chn_end = 1;
   printf("chn_end: %d\n", chn_end);
   pthread_join(chk_pid, NULL);
   printf("before channel check\n");
   channel_check();

   
   printf("Mic spdif test end\n");
   pcm_close(check_pcm);
   free(check_buffer);
   audio_queue_free(&queue);
   ret = 0;

   return ret;
}
