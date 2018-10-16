#include <tinyalsa/asoundlib.h>
#include <pthread.h>
#include <math.h>
#include "factorytest.h"
#include "audio_queue.h"
#include "fftw3.h"

#define MIC_INIT   0x01
#define SPDIF_OUT  0x02
#define CHECK_CHN  0x03
#define END_CHN    0x04
#define TEST_END   0xFF

#define TEST_AUDIO_FILE  "/system/usr/share/8k.raw"

static struct pcm* pcm;
static struct pcm_config config;
static char* buffer;
static unsigned int size;

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
static int chn_result = 0;

int audio_record()
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
      ALOGE("Unable to open recored PCM device (%s)", pcm_get_error(check_pcm));
      return -1;
   }

   check_size = pcm_frames_to_bytes(check_pcm, pcm_get_buffer_size(check_pcm));
   audio_queue_init(&queue);
   check_buffer = malloc(check_size);
   if(check_buffer){
      memset(check_buffer, check_size, 0);
   }else{
      ALOGE("Malloc check_buffer failed");
      return -1;
   }

   while(1){
      if(chn_end){
         chn_end = 0;
	 break;
      }
      ret = pcm_read(check_pcm, check_buffer, check_size);
      if(ret){
         ALOGE("Record pcm read data failed, ret %d", ret);
	 break;
      }

      audio_queue_put(&queue, check_buffer, check_size);
   }
   return ret;
}

void channel_record()
{ 
   pthread_t chk_pid;

   pthread_create(&chk_pid, NULL, audio_record, NULL);
   pthread_join(chk_pid, NULL);
}

int channel_check()
{
   audio_buffer buf;
   unsigned int tsize;        /*total size of audio data*/
   char *tbuf;
   unsigned int pos = 0;
   short *data;
   unsigned int i, N, f;
   double *abuf;
   fftw_complex *in, *out;
   fftw_plan p;

   tsize = queue.size;
   tbuf = (char*)malloc(tsize);
   if(!tbuf){
      ALOGE("channel check, Malloc tbuf failed\n");
      return -1;
   }
   memset(tbuf, 0 , tsize);

   while(pos < tsize){
      /*Get record audio data*/
      audio_queue_get(&queue, &buf);
      memcpy(tbuf + pos, buf.data, buf.size);
      pos += buf.size;
   }
   data = (short*)tbuf;
   N = tsize >> 1;
   abuf = (double*)malloc(sizeof(double)*N);
   for(i=0; i<N; i++){
      abuf[i] = data[i]*1.0/0x7fff;         /*short to double*/
   }
   
   in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
   out = in;

   for(i=0; i<N; i++){
      in[i][0] = abuf[i];
      in[i][1] = 0.0;
   }
   
   p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    
   fftw_execute(p);

   /*Check 1024Hz 4096Hz 7600Hz 7900Hz*/
   f = 1024 * N/16000;
   a1 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A1: %f", f, a1);
   f = 4096 * N/16000;
   a2 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A2: %f", f, a2);
   f = 7600 * N/16000;
   a3 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A3: %f", f, a3);
   f = 7900 * N/16000;
   a4 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A4: %f", f, a4);
   if((abs(a1-A1) < 2) && (abs(a2-A2) < 2) && (abs(a3-A3) < 2) && (abs(a4-A4) < 2)){
      ALOGI("Mic channel test OK");
      chn_result = 1;
   }else{
      chn_result = 0;
   }

   fftw_destroy_plan(p);
   fftw_free(in);
   free(tbuf);
   free(abuf);
   return 0;
}

int audio_card_init()
{
   config.channels = 2;
   config.rate = 8000;
   config.format = PCM_FORMAT_S32_LE;
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

static int audio_fft()
{
   fftw_complex *in, *out;
   fftw_plan p;
   int N;
   int i;
   int f;
   
   FILE *fp;
   long flen;
   long fsta, fen, fend;
   char *fname = "/system/usr/share/8k_32768.raw";
   fp = fopen(fname, "rb");
   if(!fp){
      ALOGE("open file %s failed\n", fname);
      return -1;
   }
   fsta = ftell(fp);
   fseek(fp, 0, SEEK_END);
   fend = ftell(fp);
   flen = fend -fsta;
   ALOGI("Audio file for fft len: %ld\n", flen);
   short *data;
   double buf[flen/2];
   char temp[flen];
   #define SHORT_MAX 0x7fff
   fseek(fp, 0, SEEK_SET);
   fread(temp, 1, flen, fp);
   data = (short*)temp;
   memset(buf, 0, sizeof(buf));
   N = flen/2;
   for(i=0; i<N; i++){
      buf[i] = data[i]*1.0/0x7fff;
   }

   in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    
   out = in;
   for(i=0; i<N; i++){
       in[i][0] = buf[i];
       in[i][1] = 0.0;
   }
 
   p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

   fftw_execute(p);
   //for(i=0; i<24; i++){
   //   printf("%d: %f%fi\n", i, out[i][0], out[i][1]);
   //}

   /*Get frequency of data point 1024Hz 4096Hz 7900Hz 8000Hz */
   f = 1024 * N/16000;
   A1 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A1: %f", f, A1);
   f = 4096 * N/16000;
   A2 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A2: %f", f, A2);
   f = 7600 * N/16000;
   A3 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A3: %f", f, A3);
   f = 7900 * N/16000;
   A4 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A4: %f", f, A4);

   fftw_destroy_plan(p);
   fftw_free(in);
   return 0;
}

int mic_spdif_test(int uart_fd)
{
   int ret = -1;
   char data;
   uart_msg msg;

   audio_fft();
   
   while(1){
     receive_msg(uart_fd, &msg, 8);
     if(msg.order == MIC_SPDIF_TEST){
        data = *msg.data;
        if(data == MIC_INIT){
          //init mic

        }else if(data == SPDIF_OUT){
           audio_play();
        }else if(data == CHECK_CHN){
           channel_record();      
        }else if(data == END_CHN){
	   chn_end = 1;
	   sleep(1);
           channel_check();
	   send_msg(uart_fd, MIC_SPDIF_TEST, (char*)&chn_result, sizeof(chn_result));
	}else{
           ALOGI("Mic spdif test end");
	   pcm_close(pcm);
           free(buffer);
           pcm_close(check_pcm);
           free(check_buffer);
	   ret = 0;
	   break;
        }
     }else{
       break;
     }
   }
   return ret;
}
