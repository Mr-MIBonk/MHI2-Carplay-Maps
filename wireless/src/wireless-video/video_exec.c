/* Separate opt-in video launcher. Never overwrite the known-good decoder.
 * A missing or damaged experimental hook falls back to the exact stock copy.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CPMAP_SHA256_EMBEDDED
#include "sha256.c"
#include "video_hashes.h"
#ifndef V75_VIDEO_ROOT
#define V75_VIDEO_ROOT ""
#endif
#define BASE V75_VIDEO_ROOT "/mnt/app/carplaymaps/"
static int matches(const char *p,const char *wanted){
    unsigned char b[32];char text[65];unsigned i;
    if(hash_file(p,b))return 0;
    for(i=0;i<32;i++){text[2*i]="0123456789abcdef"[b[i]>>4];text[2*i+1]="0123456789abcdef"[b[i]&15];}
    text[64]=0;return !strcmp(text,wanted);
}
int main(int argc,char **argv){
    int candidate=matches(BASE "wcp_video_tap",V75_VIDEO_SHA)&&matches(BASE "libv75-wireless-video-tap.so",V75_TAP_SHA);
    const char *target;
    if(!matches(BASE "wcp_video_stock",V75_STOCK_VIDEO_SHA))return 3;
    if(argc==2&&!strcmp(argv[1],"--status"))return candidate?0:11;
    if(getenv("LD_PRELOAD")&&*getenv("LD_PRELOAD"))return 4;
    if(candidate){
        if(setenv("LD_PRELOAD",BASE "libv75-wireless-video-tap.so",1)||setenv("V75_WCP_VIDEO_TAP","1",1))return 5;
        target=BASE "wcp_video_tap";
    }else{
        if(unsetenv("V75_WCP_VIDEO_TAP"))return 5;
        target=BASE "wcp_video_stock";
    }
    argv[0]=(char *)target;execv(target,argv);return 6;
}
