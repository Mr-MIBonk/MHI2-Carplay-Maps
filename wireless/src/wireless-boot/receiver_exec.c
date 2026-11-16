/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29.
 * Hash-coupled maps-only receiver wrapper. No music, cover or V53 startup. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define CPMAP_SHA256_EMBEDDED
#include "sha256.c"
#include "maps_hashes.h"
#ifndef OMONOB_BOOT_ROOT
#define OMONOB_BOOT_ROOT ""
#endif
#define RECEIVER OMONOB_BOOT_ROOT "/dev/shmem/wcp_receiver"
#define INTERFACE OMONOB_BOOT_ROOT "/mnt/app/carplaymaps/libomonob-wireless-maps.so"
#define TRANSPORT OMONOB_BOOT_ROOT "/mnt/app/carplaymaps/libv75-transport.so"

static int matches(const char *file,const char *wanted){
    uint8_t value[32];char hex[65];unsigned i;
    if(hash_file(file,value))return 0;
    for(i=0;i<32;i++){hex[2*i]="0123456789abcdef"[value[i]>>4];hex[2*i+1]="0123456789abcdef"[value[i]&15];}
    hex[64]=0;memset(value,0,sizeof(value));return !strcmp(hex,wanted);
}
static int verify(void){
    return matches(RECEIVER,OMONOB_RECEIVER_SHA)&&matches(INTERFACE,OMONOB_INTERFACE_SHA)&&matches(TRANSPORT,OMONOB_TRANSPORT_SHA);
}
static int prepare_scope(char value[80]){
    struct timespec now;unsigned attempt;
    if(clock_gettime(CLOCK_MONOTONIC,&now))return -1;
    for(attempt=0;attempt<128;attempt++){
        int n=snprintf(value,80,OMONOB_BOOT_ROOT "/ramdisk/.v75-native-%08x%08x%08x%08x",
            (unsigned)getpid(),(unsigned)now.tv_sec,(unsigned)now.tv_nsec,attempt);
        if(n<1||n>=80)return -1;
        if(!mkdir(value,0700))return 0;
        if(errno!=EEXIST)return -1;
    }
    return -1;
}
int main(int argc,char **argv){
    char scope[80];const char *old=getenv("LD_PRELOAD");
    if(argc==2&&!strcmp(argv[1],"--transport-status"))return matches(TRANSPORT,OMONOB_TRANSPORT_SHA)?0:10;
    if(argc==2&&!strcmp(argv[1],"--verify-only"))return verify()?0:10;
    if(argc!=1||(old&&*old)||!verify())return 10;
    if(prepare_scope(scope))return 11;
    if(setenv("V75_WCP_NATIVE","1",1)||setenv("V75_WCP_SCOPE",scope,1)||
       setenv("V75_WCP_CARD",OMONOB_CLUSTER,1)||setenv("V75_WCP_TRANSPORT","1",1)||
       setenv("WCP_FPS","30",1)||setenv("WCP_VIDEO_FPS","30",1)||
       setenv("LD_PRELOAD",TRANSPORT ":" INTERFACE,1)){
        rmdir(scope);return 12;
    }
    execl(RECEIVER,RECEIVER,(char *)NULL);
    rmdir(scope);return 127;
}
