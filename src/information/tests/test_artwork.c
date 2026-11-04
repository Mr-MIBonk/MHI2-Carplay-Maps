#define _POSIX_C_SOURCE 200809L
#define COVER_OUT "/tmp/cpmap-artwork-host.jpg"
#define COVER_TMP "/tmp/cpmap-artwork-host.tmp"
#define ACTIVE_PATH "/tmp/cpmap-artwork-host.active"
#define ID_PATH "/tmp/cpmap-artwork-host.id"
#define ID_TMP "/tmp/cpmap-artwork-host.id.tmp"
#define LOCK_PATH "/tmp/cpmap-artwork-host.lock"
#define main artwork_daemon_main
#include "../native/carplay_artwork_helper.c"
#undef main
#include <assert.h>
#include <sys/wait.h>
void *iap2_connect(const char *p,int f){(void)p;(void)f;return NULL;}
int iap2_filexfer_read(void*h,int i,void*b,int n){(void)h;(void)i;(void)b;(void)n;return 0;}
void iap2_disconnect(void*h){(void)h;}
struct encoded {unsigned char bytes[65536];int count;};
static void memory_jpeg(void *context,void *data,int size){
    struct encoded *e=context;assert(size>=0&&e->count+size<(int)sizeof(e->bytes));
    memcpy(e->bytes+e->count,data,(size_t)size);e->count+=size;
}
int main(void){
    uint8_t pixels[32*16*3];memset(pixels,210,sizeof(pixels));
    struct encoded jpeg={{0},0};
    assert(stbi_write_jpg_to_func(memory_jpeg,&jpeg,32,16,3,pixels,75));
    assert(write_cover_resized(jpeg.bytes,jpeg.count)==0);
    FILE*f=fopen(COVER_OUT,"rb");assert(f);unsigned char output[65536];
    int bytes=(int)fread(output,1,sizeof(output),f);fclose(f);int w,h,c;
    assert(stbi_info_from_memory(output,bytes,&w,&h,&c)&&w==128&&h==128);
    assert(write_cover_resized((const uint8_t*)"bad",3)==-1);
    /* Increase the JPEG SOF height to 4096: rejected before pixel allocation. */
    int changed=0;
    for(int i=0;i+8<jpeg.count;i++)if(jpeg.bytes[i]==0xff&&jpeg.bytes[i+1]==0xc0){jpeg.bytes[i+5]=0x10;jpeg.bytes[i+6]=0;changed=1;break;}
    assert(changed&&write_cover_resized(jpeg.bytes,jpeg.count)==-1);
    int fd=open("/dev/full",O_WRONLY);assert(fd>=0);struct jpeg_output sink={fd,0};
    jpeg_sink(&sink,pixels,10);assert(sink.failed&&fcntl(fd,F_GETFD)!=-1);close(fd);
    int lock=take_lock();assert(lock>=0);cleanup_files();
    pid_t pid=fork();assert(pid>=0);if(!pid){int second=take_lock();_exit(second<0?0:1);}
    int status;waitpid(pid,&status,0);assert(WIFEXITED(status)&&WEXITSTATUS(status)==0);
    close(lock);pid=fork();assert(pid>=0);if(!pid){int second=take_lock();_exit(second>=0?0:1);}
    waitpid(pid,&status,0);assert(WIFEXITED(status)&&WEXITSTATUS(status)==0);
    char sig[SIG_MAX];assert(build_sig("PlaybackAppBundleID::maps\nMediaItemTitle::A\nMediaItemArtist::B\nMediaItemAlbumTitle::C\n",sig,sizeof(sig))==0);
    assert(!strcmp(sig,"maps\x01""A\x01""B\x01""C"));
    cleanup_files();unlink(LOCK_PATH);
    puts("ARTWORK_HOST_PASS decode_bounds=1 output=128x128 failed_write_keeps_fd=1 crash_safe_singleton=1 signature=1 real_vehicle=0");return 0;
}
