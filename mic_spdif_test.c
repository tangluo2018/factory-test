#include <tinyalsa/asoundlib.h>
#include <pthread.h>
#include <math.h>
#include "factorytest.h"
#include "audio_queue.h"
#include "fftw3.h"

#define MIC_INIT    0x0B
#define SPDIF_OUT   0x0A
#define CHECK_CHN1  0x01
#define CHECK_CHN2  0x02
#define CHECK_CHN3  0x03
#define CHECK_CHN4  0x04
#define CHECK_CHN5  0x05
#define CHECK_CHN6  0x06
#define CHECK_CHN7  0x07
#define CHECK_CHN8  0X08
#define TEST_END    0xFF

#define AUDIO_SAMPLERATE 44100
#define AUDIO_1K_AMP   3000
#define AUDIO_THD_AMP  500

//#define TEST_AUDIO_FILE  "/etc/1k.pcm"
#define TEST_AUDIO_FILE "/etc/test.pcm"
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
static char chn_result = 0;

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
      ALOGE("Unable to open recored PCM device (%s)", pcm_get_error(check_pcm));
      return -1;
   }

   check_size = pcm_frames_to_bytes(check_pcm, pcm_get_buffer_size(check_pcm));
   //audio_queue_init(&queue);
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
   //pthread_join(chk_pid, NULL);
}

static int channel_check(int uart_fd)
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
   char amps[16];
   fftw_complex *in, *out;
   fftw_plan p;

   ALOGI("channel_check");
   tsize = queue.size;

   #if 1
   FILE *fp = NULL;
   fp = fopen("/sdcard/channel.pcm", "wb+");
   ALOGI("..........................");
   if(!fp){
      ALOGE("Open /sdcard/channel.pcm failed");
   }
   #endif

   /*
   tbuf = (char*)malloc(tsize);
   if(!tbuf){
      ALOGE("channel check, Malloc tbuf failed\n");
      return -1;
   }
   memset(tbuf, 0 , tsize);
   */

   chn_size = tsize/8;        /*8 channel*/
   char audioData[8][chn_size];
   ALOGI("tsize: %d, chn_size: %d", tsize, chn_size);
   while(1){
       if(queue.size <= 0){
           break;
       }
      /*Get record audio data*/
      audio_queue_get(&queue, &buf);
      #if 1
      fwrite(buf.data, buf.size, 1, fp);
      #endif
      //memcpy(tbuf + pos, buf.data, buf.size);
      index = 0;
      data = (char*)(buf.data);
      tmp_size = buf.size;
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
      audio_data_free(&buf);
   }
   //data = (int*)tbuf;
   N = chn_size >> 2;
   abuf = (double*)malloc(sizeof(double)*N);

   for(j=0; j<8; j++){
      memset(abuf, 0, sizeof(double)*N);
      int *tmpData = (int*)audioData[j];
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

      /*Check 1000Hz 500Hz 2000Hz */
      f = 1000 * N/16000;
      a1 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
      ALOGI("%d: a1: %f", f, a1);
      f = 2000 * N/16000;
      a2 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
      ALOGI("%d: a2: %f", f, a2);
      f = 3000 * N/16000;
      a3 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
      ALOGI("%d: a3: %f", f, a3);
      f = 4000 * N/16000;
      a4 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
      ALOGI("%d: a4: %f", f, a4);
      //a1=2897.653478;
      //a2=0.002276;
      //a3=1.894321;
      //a4=0.983421;
      int amp1 = (int)a1;
      int amp2 = (int)a2;
      int amp3 = (int)a3;
      int amp4 = (int)a4;
      memset(amps, 0, sizeof(amps));
      memcpy(amps,    &amp1, 4);
      memcpy(amps+4,  &amp2, 4);
      memcpy(amps+8,  &amp3, 4);
      memcpy(amps+12, &amp4, 4);
      send_msg(uart_fd, MIC_SPDIF_TEST, (char*)amps, 16);

      #if 0
      if((a1 > AUDIO_1K_AMP) && (a2 < AUDIO_THD_AMP) && (a3 < AUDIO_THD_AMP)){
         ALOGI("Mic channel test OK");
         chn_result = 0x1;
	 fftw_destroy_plan(p);
	 fftw_free(in);
	 break;
      }else{
         chn_result = 0x2;
     }
     #endif
     fftw_destroy_plan(p);
     fftw_free(in);
   }
   /*
   free(tbuf);
   */
   free(abuf);
   return 0;
}

int audio_card_init()
{
   config.channels = 2;
   config.rate = 44100;
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

static int audio_play_2(char *audio_data, int len)
{
   int num_read;
   unsigned int wlen;
   char *data;

   audio_card_init();
   num_read = 0;
   wlen = size; //1536;
   data = audio_data;

   #if 0
   FILE *fp = fopen("/sdcard/audio_play.pcm", "wb+");
   if(!fp){
      ALOGE("Open audio_play.pcm failed");
   }
   #endif
   
   #if 1
   while(num_read < len){
         #if 0
	 fwrite(data, wlen, 1, fp);
         #endif
         if(pcm_write(pcm, data, wlen)){
            ALOGE("Error playing test sample");
            break;
	 }
	 data = data + wlen;
	 num_read += wlen;
	 if((num_read + wlen) >= len){
            wlen = len - num_read;
	 }
	 
	 //ALOGI("num_read %d, wlen %d", num_read, wlen);
   }
   #endif
   #if 0
   fclose(fp);
   #endif 
   //ALOGI("audio play end");
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
   char *fname = "/etc/1k.pcm";
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
   f = 1000 * N/44100;
   A1 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A1: %f", f, A1);
   f = 500 * N/44100;
   A2 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A2: %f", f, A2);
   f = 2000 * N/44100;
   A3 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A3: %f", f, A3);
   //f = 7900 * N/16000;
   //A4 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   //ALOGI("%d: A4: %f", f, A4);

   fftw_destroy_plan(p);
   fftw_free(in);
   fclose(fp);
   return 0;
}

int audio_fft_2(short *data, int sampleNum)
{
   fftw_complex *in, *out;
   fftw_plan p;
   int N;
   int i;
   int f;
   
   double buf[sampleNum];
   
   ALOGI("audio_fft_2");

   N = sampleNum;
   #define SHORT_MAX 0x7fff
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

   /*Get frequency of data point 1000Hz */
   f = 1000 * N/44100;
   A1 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A1: %f", f, A1);
   f = 500 * N/44100;
   A2 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A2: %f", f, A2);
   f = 2000 * N/44100;
   A3 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   ALOGI("%d: A3: %f", f, A3);

   fftw_destroy_plan(p);
   fftw_free(in);
   return 0;

}

#define PI 3.14159265358979323846
/*create 16bit specified requence audio*/
int create_audio(short **audio_data, int samplerate, int freq, int len)
{
    double rad;
    double t;
    int i;
    short *data;
    
    data = (short*)malloc(len*sizeof(short));
    if(!data){
       ALOGI("create_audio, malloc failed");
       return -1;
    }
    memset(data, 0, len*sizeof(short));
    for(i=0; i<len; i++){
      t =1.0 * i / samplerate;
      rad = 2 * PI * freq * t;
      data[i] = 0x7FFF*sin(rad);
    }
    *audio_data = data;
    #if 0
    FILE *fp = NULL;
    fp = fopen("/sdcard/1k.pcm", "wb+");
    if(!fp){
      ALOGE("Open /sdcard/1k.pcm faile");
    }
    fwrite((char*)data, len * 2, 1, fp);
    fclose(fp);
    #endif 
    return 0;
}

void create_2chn_audio(short *dst, short *src, int len)
{
   int i = 0, j = 0;

   while(i<len){
      dst[j] = src[i];
      dst[j+1] = src[i];
      i++;
      j=j+2;
   }
}

short *audio_data = NULL;
short *audio_data_2ch = NULL;
//static int isCreated = 0;
int mic_spdif_test(int uart_fd, uart_msg *msg)
{
   int ret = -1;
   char data;
   //uart_msg msg;
   //audio_fft();
   //audio_queue_init(&queue);
   
   //while(1){
     
     //receive_msg(uart_fd, &msg, 8);
     if(msg->order == MIC_SPDIF_TEST){
        data = *msg->data;
        if(data == 0xFF){
     //      break;
        }
     
        if(data == MIC_INIT){
          //init mic

        }else if(data == SPDIF_OUT){

     
        }else if((data == CHECK_CHN1) || (data == CHECK_CHN2) 
	         || (data == CHECK_CHN3) || (data == CHECK_CHN4)
		 || (data == CHECK_CHN5) || (data == CHECK_CHN6)
		 || (data == CHECK_CHN7) || (data == CHECK_CHN8)){

	   create_audio(&audio_data,  44100, 1000, 44100*5);
           audio_fft_2(audio_data, 44100*5);
           audio_data_2ch = (short*)malloc(44100*5*2*sizeof(short));
           if(!audio_data_2ch){
              ALOGE("Malloc audio_data_2ch failed");
              return -1;
           }
           create_2chn_audio(audio_data_2ch, audio_data, 44100*5);

	   ALOGI("Mic test channel record");
	   channel_record();
	   usleep(20*1000);
	   //audio_play();
           audio_play_2((char*)audio_data_2ch, 44100*2*5*2);
	   ALOGI("audio play end");

           ALOGI("Mic test %x channel check", data);
	   //if(!audio_data_2ch || !check_pcm){
           //   ALOGI("Send play order first!!");
	   //   return -1;
	   //}

           chn_end = 1;
	   usleep(200*1000); // wait for the record thread
	   channel_check(uart_fd);
           //send_msg(uart_fd, MIC_SPDIF_TEST, (char*)&chn_result, sizeof(chn_result));
           ALOGI("Mic spdif channle %x test end, %x", data, chn_result);
           pcm_close(pcm);
           free(buffer);
           buffer = NULL;
           pcm_close(check_pcm);
           free(check_buffer);
           free(audio_data);
           free(audio_data_2ch);
           check_buffer = NULL;
           audio_data = NULL;
           audio_data_2ch = NULL;
           audio_queue_free(&queue);
  
	}
     }
   //}
   ret = 0;

   return ret;
}
