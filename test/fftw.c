#include <fftw3.h>
#include <string.h>

int main()
{

   fftw_complex *in, *out;
   fftw_plan p;
   int N;
   int i;
   int f;
   
   FILE *fp;
   long flen;
   long fsta, fen, fend;
   char *fname = "/system/usr/share/8k.raw";
   fp = fopen(fname, "rb");
   if(!fp){
      printf("open file %s failed\n", fname);
      return -1;
   }
   fsta = ftell(fp);
   fseek(fp, 0, SEEK_END);
   fend = ftell(fp);
   flen = fend -fsta;
   printf("file len: %ld\n", flen);
   short *data;
   double buf[flen/2];
   char temp[flen];
   #define SHORT_MAX 0x7fff
   fseek(fp, 0, SEEK_SET);
   fread(temp, 1, flen, fp);
   data = (short*)temp;
   memset(buf, 0, sizeof(buf));
   N = flen/2;
   N = 56000;
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
   for(i=0; i<24; i++){
      printf("%d: %f%fi\n", i, out[i][0], out[i][1]);
   }

   f = 800 * N/16000;
   double A1;
   A1 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   printf("%d, A1: %f\n", f, A1);   
   f = 4096 * N/16000;
   double A2;
   A2 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   printf("%d, A1: %f\n", f, A2);   
   f = 7600 * N/16000;
   double A3;
   A3 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   printf("%d, A1: %f\n", f, A3);   
   f = 7900 * N/16000;
   double A4;
   A4 = sqrt(out[f][0]*out[f][0] + out[f][1]*out[f][1]);
   printf("%d, A1: %f\n", f, A4);   



   fftw_destroy_plan(p);
   fftw_free(in);
   return 0;
}
