#include <stdio.h>
#include <sys/vfs.h>
#include <log/log.h>

#define GSIZE (1024*1024*1024)
#define MSIZE (1024*1024)

int main()
{
  long long total_blocks,free_blocks, bfree, f_bsize;
  double system_size; 
  double cache_size;
  double data_size;
  double internal_size;
  double total_size;
  double root_size;
  struct statfs fs;

    if(statfs("/system", &fs)<0)
    {
       ALOGE("statfs /system failed");
       return -1;
    }
  
    printf("Type of fs:%x\n", fs.f_type); //type of filesystem
    printf("Total blocks:%lld\n", fs.f_blocks); //total data blocks in file system
    printf("Free blocks:%lld\n", fs.f_bfree); //free blocks in fs
    printf("Free avail to non-supperuser:%lld\n", fs.f_bavail); // free blocks avail to non-supperuser
    printf("Fs id:%x\n", fs.f_fsid);//filesystem id
 
    free_blocks = fs.f_bfree;
    total_blocks = fs.f_blocks;

    printf("block size: %ld\n", fs.f_bsize);
    f_bsize = fs.f_bsize;

    system_size= (double)total_blocks*f_bsize/(long long)GSIZE;
    ALOGI("/system size %llf", system_size);

    printf("Total size(GB): %llf\n", (double)total_blocks*f_bsize/(long long)GSIZE);
    printf("Free size(GB): %llf\n", (double)free_blocks*f_bsize/(long long)GSIZE);

    if(statfs("/cache", &fs) < 0){
      ALOGE("statfs /cache failed");
      return -1;
    }
    cache_size = (double)fs.f_blocks * fs.f_bsize/(long long)GSIZE;
    ALOGI("/cache size %llf", cache_size);

    if(statfs("/data", &fs) < 0){
      ALOGE("statfs /data failed");
      return -1;
    }
    data_size = (double)fs.f_blocks * fs.f_bsize/(long long)GSIZE;
    ALOGI("/data size %llf", data_size);
    
    if(statfs("/mnt/internal_sd", &fs) < 0){
      ALOGE("statfs /mnt/internal_sd failed");
      return -1;
    } 
    internal_size = (double)fs.f_blocks * fs.f_bsize/(long long)GSIZE;
    ALOGI("/mnt/internal_sd size %llf", internal_size);
    total_size = root_size + system_size + cache_size + data_size + internal_size;
    
    int total = (int)(total_size + 0.5);
    ALOGI("total size %llf, total %d", total_size, total);
  return 0;
}
