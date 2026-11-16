/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
/* omonob, source date 2026-08-29.
 * Input binding only. The existing CarPlay ring, animation player, TS mux,
 * supervisor and MOST writer are reused. No audio or authentication code.
 */
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static char ring_path[512]="/ramdisk/carplay-alt-ring.bin";
static char ready_path[512]="/ramdisk/carplay-altstream.ready";
static char owner_path[512]="/ramdisk/carplay-dio-manager.pid";
static char idr_path[512]="/ramdisk/carplay-alt-force-idr.request";
static char cache_path[512]="/ramdisk/cpmap-video-cache.lock";
#define ALT_FRAME_RING_PATH ring_path
#define ALT_FRAME_FORCE_IDR_PATH idr_path
#define CPMAP_CACHE_LOCK cache_path
#include "cpmap_resources.h"
#include "altscreen_frame_protocol.h"
#include "existing_ring_publisher.h"
#include "cpmap_projection_preferences.h"
#include "cpvc_authorized_runtime.h"
#include "wireless_session_gate.h"

#define NAL_MAX ALT_FRAME_RING_SLOT_DATA_MAX
#define PARAM_MAX 4096U
#ifndef CPMAP_IDR_FRAMES
#define CPMAP_IDR_FRAMES 180UL
#endif
#if CPMAP_IDR_FRAMES != 5 && CPMAP_IDR_FRAMES != 180
#error Only the two approved cluster IDR profiles may be built
#endif
static volatile sig_atomic_t stopped;
static int offline;

typedef struct Input {
    unsigned char pending[NAL_MAX+16U],au[NAL_MAX];
    unsigned char sps[PARAM_MAX],pps[PARAM_MAX];
    size_t pending_size,scan,au_size,sps_size,pps_size;
    int vcl,key,config_dirty,waiting_idr,active,connection,lease,map_enabled;
    uint32_t source_sequence;
    unsigned long frames,keys,rejected,configurations;
    AltFrameRing *ring;
} Input;

static void stop_signal(int value){(void)value;stopped=1;}
static uint64_t monotonic_us(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*1000000ULL+(uint64_t)t.tv_nsec/1000ULL;}
static uint64_t input_timestamp(void){uint64_t u=monotonic_us();return (u/1000000ULL<<32)|((u%1000000ULL<<32)/1000000ULL);}
static int write_text(const char *path,const char *text){int fd=open(path,O_WRONLY|O_CREAT|O_TRUNC,0600);size_t n=strlen(text);ssize_t r;if(fd<0)return -1;r=write(fd,text,n);close(fd);return r==(ssize_t)n?0:-1;}
static long read_pid(const char *path){long p=0;FILE *f=fopen(path,"r");if(!f)return 0;if(fscanf(f,"%ld",&p)!=1)p=0;fclose(f);return p;}
static int owns_output(void){long p=read_pid(owner_path);return p==0||p==(long)getpid()||(kill((pid_t)p,0)!=0&&errno==ESRCH);}
static int map_requested(void){struct cpmap_projection_preferences p;if(offline)return 1;return cpvc_authorized_runtime()&&cpmap_read_projection_preferences("/mnt/icab/persistent/cpmap-ui",&p)>=0&&p.map&&v75_gate_allowed();}
static void request_idr(Input *p){if(p->connection>=0){const char k='K';(void)send(p->connection,&k,1,MSG_DONTWAIT);}}
static void retire(Input *p){
    if(p->active&&read_pid(owner_path)==(long)getpid()){unlink(ready_path);unlink(owner_path);}
    p->active=0;p->waiting_idr=1;
}
#include "wireless_map_resume.h"
static void reject_frame(Input *p){++p->rejected;p->au_size=0;p->vcl=0;p->key=0;p->waiting_idr=1;request_idr(p);}
static size_t prefix(const unsigned char *b,size_t n,size_t at){
    if(at+4U<=n&&!b[at]&&!b[at+1]&&!b[at+2]&&b[at+3]==1)return 4;
    if(at+3U<=n&&!b[at]&&!b[at+1]&&b[at+2]==1)return 3;
    return 0;
}
/* Only the first unsigned Exp-Golomb bit is needed: first_mb_in_slice==0
 * begins with 1. Emulation-prevention cannot precede that first RBSP bit. */
static int first_slice(const unsigned char *nal,size_t n){return n>1U&&(nal[1]&128U)!=0;}
static int publish_config(Input *p){
    unsigned char data[PARAM_MAX*2U+16U];size_t at=0;
    if(p->sps_size<4U||!p->pps_size)return -1;
    data[at++]=1;data[at++]=p->sps[1];data[at++]=p->sps[2];data[at++]=p->sps[3];data[at++]=255;data[at++]=225;
    data[at++]=(unsigned char)(p->sps_size>>8);data[at++]=(unsigned char)p->sps_size;memcpy(data+at,p->sps,p->sps_size);at+=p->sps_size;
    data[at++]=1;data[at++]=(unsigned char)(p->pps_size>>8);data[at++]=(unsigned char)p->pps_size;memcpy(data+at,p->pps,p->pps_size);at+=p->pps_size;
    if(forward_ring_message(p->ring,ALT_FRAME_TYPE_CONFIG,0,0,0,data,at)<0)return -1;
    p->config_dirty=0;++p->configurations;return 0;
}
static void flush_au(Input *p){
    char pid[32];
    if(!p->vcl||!p->au_size)return;
    if(!map_requested()||!owns_output()){retire(p);goto discard;}
    if(p->waiting_idr&&!p->key)goto discard;
    if(!p->sps_size||!p->pps_size)goto discard;
    if(!p->ring){
        if(!p->lease){if(cpmap_cache_acquire()){reject_frame(p);return;}p->lease=1;}
        p->ring=create_forward_ring();if(!p->ring){cpmap_cache_release();p->lease=0;reject_frame(p);return;}p->config_dirty=1;
    }
    /* The existing bridge skips the buffered intro interval and asks for a
     * new IDR. Re-publish avcC before every IDR, as the original producer does. */
    if((p->config_dirty||p->key)&&publish_config(p)){reject_frame(p);return;}
    if(forward_ring_message(p->ring,ALT_FRAME_TYPE_VIDEO,++p->source_sequence,p->key?ALT_FRAME_FLAG_KEYFRAME:0,input_timestamp(),p->au,p->au_size)<0){reject_frame(p);return;}
    ++p->frames;if(p->key)++p->keys;p->waiting_idr=0;
    if(!p->active){snprintf(pid,sizeof(pid),"%ld\n",(long)getpid());if(!write_text(owner_path,pid)&&!write_text(ready_path,"1\n")){p->active=1;fprintf(stderr,"OMONOB_791_INPUT_ACTIVE generation=%u video_only=1\n",p->ring->generation);}}
    if(p->frames%CPMAP_IDR_FRAMES==0)request_idr(p);
discard:p->au_size=0;p->vcl=0;p->key=0;
}
static void nal(Input *p,const unsigned char *b,size_t n){
    unsigned type;
    if(!n||n>NAL_MAX||(b[0]&128U)){reject_frame(p);return;}type=b[0]&31U;
    if(type==9U){flush_au(p);return;}
    if(type==7U||type==8U){
        unsigned char *target=type==7U?p->sps:p->pps;size_t *size=type==7U?&p->sps_size:&p->pps_size;
        flush_au(p);if(n>PARAM_MAX||(type==7U&&n<4U)){reject_frame(p);return;}
        if(*size!=n||memcmp(target,b,n)){memcpy(target,b,n);*size=n;p->config_dirty=1;p->waiting_idr=1;}return;
    }
    if(type==6U&&p->vcl)flush_au(p);
    if(type==1U||type==5U){if(n<2U){reject_frame(p);return;}if(p->vcl&&first_slice(b,n))flush_au(p);p->vcl=1;if(type==5U)p->key=1;}
    else if(type!=6U)return;
    if(n>NAL_MAX-4U||p->au_size>NAL_MAX-4U-n){reject_frame(p);return;}
    p->au[p->au_size++]=(unsigned char)(n>>24);p->au[p->au_size++]=(unsigned char)(n>>16);p->au[p->au_size++]=(unsigned char)(n>>8);p->au[p->au_size++]=(unsigned char)n;
    memcpy(p->au+p->au_size,b,n);p->au_size+=n;
}
static void input_bytes(Input *p,const unsigned char *data,size_t n){
    size_t first,k,i;
    if(n>sizeof(p->pending)-p->pending_size){p->pending_size=0;p->scan=0;reject_frame(p);return;}
    memcpy(p->pending+p->pending_size,data,n);p->pending_size+=n;
    for(;;){
        first=prefix(p->pending,p->pending_size,0);
        if(!first){
            for(i=0;i+3U<=p->pending_size&&!prefix(p->pending,p->pending_size,i);++i){}
            if(i){memmove(p->pending,p->pending+i,p->pending_size-i);p->pending_size-=i;}
            p->scan=0;if(!prefix(p->pending,p->pending_size,0))return;continue;
        }
        if(p->scan<first)p->scan=first;
        for(i=p->scan;i+3U<=p->pending_size;++i)if(prefix(p->pending,p->pending_size,i))break;
        if(i+3U>p->pending_size){p->scan=i;return;}
        k=i-first;if(k)nal(p,p->pending+first,k);
        memmove(p->pending,p->pending+i,p->pending_size-i);p->pending_size-=i;p->scan=0;
    }
}
static void input_finish(Input *p){size_t k=prefix(p->pending,p->pending_size,0);if(k&&p->pending_size>k)nal(p,p->pending+k,p->pending_size-k);p->pending_size=0;flush_au(p);}
static Input *new_input(int client){Input *p=(Input *)calloc(1,sizeof(*p));if(p){p->connection=client;p->waiting_idr=1;}return p;}
static void free_input(Input *p){if(!p)return;retire(p);if(p->ring)munmap(p->ring,sizeof(*p->ring));if(p->lease)cpmap_cache_release();free(p);}
static int paths(const char *directory){
    if(snprintf(ring_path,sizeof(ring_path),"%s/ring.bin",directory)>=(int)sizeof(ring_path)||snprintf(ready_path,sizeof(ready_path),"%s/ready",directory)>=(int)sizeof(ready_path)||snprintf(owner_path,sizeof(owner_path),"%s/owner",directory)>=(int)sizeof(owner_path)||snprintf(idr_path,sizeof(idr_path),"%s/idr",directory)>=(int)sizeof(idr_path)||snprintf(cache_path,sizeof(cache_path),"%s/cache.lock",directory)>=(int)sizeof(cache_path))return -1;return 0;
}
static int selftest(const char *directory,const char *fixture){
    unsigned char buf[4096];size_t n;FILE *f;Input *p;unsigned long expected=0;unsigned checks=0,round;size_t steps[]={1,3,7,31,257,4096};
    if(mkdir(directory,0700)||paths(directory))return 2;offline=1;
    for(round=0;round<sizeof(steps)/sizeof(steps[0]);++round){
        p=new_input(-1);if(!p)return 2;f=fopen(fixture,"rb");if(!f)return 2;
        while((n=fread(buf,1,steps[round],f))>0)input_bytes(p,buf,n);if(ferror(f)||fclose(f))return 2;input_finish(p);
        if(!round)expected=p->frames;
        assert(p->frames==expected&&expected>100UL&&p->keys>0&&p->configurations==p->keys&&p->rejected==0);++checks;
        assert(forward_ring_message(p->ring,ALT_FRAME_TYPE_VIDEO,1,0,0,buf,0)==-1);++checks;
        assert(forward_ring_message(p->ring,ALT_FRAME_TYPE_VIDEO,1,0,0,buf,NAL_MAX+1U)==-1);++checks;
        assert(forward_ring_message(NULL,ALT_FRAME_TYPE_VIDEO,1,0,0,buf,1)==-1);++checks;
        assert(p->ring->slots[(p->ring->write_sequence-1U)%ALT_FRAME_RING_SLOT_COUNT].committed_sequence==p->ring->write_sequence);++checks;
        free_input(p);
    }
    p=new_input(-1);assert(p);nal(p,(const unsigned char *)"\377bad",4);assert(p->rejected==1&&!p->frames);++checks;
    p->au_size=NAL_MAX-2U;nal(p,(const unsigned char *)"\145\200\000",3);assert(p->rejected==2&&p->au_size==0);++checks;free_input(p);
    fprintf(stdout,"OMONOB_791_ENTRY_SELFTEST_PASS checks=%u frames=%lu chunk_sizes=1,3,7,31,257,4096 original_ring_publisher=1 audio_opened=0 MOST_opened=0\n",checks,expected);return 0;
}
static int serve(unsigned port){
    int listener,client,one=1;struct sockaddr_in address;
    listener=socket(AF_INET,SOCK_STREAM,0);if(listener<0)return 2;(void)setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));
    memset(&address,0,sizeof(address));address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);address.sin_port=htons((uint16_t)port);
    if(bind(listener,(struct sockaddr *)&address,sizeof(address))||listen(listener,1)){close(listener);return 2;}
    fprintf(stderr,"OMONOB_791_INPUT_LISTEN port=%u player=existing_CarPlay video_only=1\n",port);
    while(!stopped){
        struct pollfd poller;Input *p;poller.fd=listener;poller.events=POLLIN;poller.revents=0;
        if(poll(&poller,1,250)<=0)continue;client=accept(listener,NULL,NULL);if(client<0)continue;
        if(v75_gate_begin()){fprintf(stderr,"V75_CLUSTER_FEED_REJECTED invalid_generation=1\n");close(client);continue;}
        p=new_input(client);if(!p){v75_gate_end();close(client);continue;}request_idr(p);
        while(!stopped){
            unsigned char data[16384];ssize_t got;poller.fd=client;poller.events=POLLIN;poller.revents=0;
            v75_map_resume(p,map_requested(),v75_gate_take_idr());
            if(access(idr_path,F_OK)==0){request_idr(p);unlink(idr_path);}
            if(poll(&poller,1,100)<=0)continue;
            got=recv(client,data,sizeof(data),0);if(got==0)break;if(got<0){if(errno==EINTR)continue;break;}
            input_bytes(p,data,(size_t)got);
        }
        /* Disconnect must discard an incomplete NAL/AU, never publish it. */
        fprintf(stderr,"OMONOB_791_INPUT_END frames=%lu keys=%lu rejected=%lu partial_discarded=1\n",p->frames,p->keys,p->rejected);
        free_input(p);v75_gate_end();close(client);
    }
    close(listener);return 0;
}
int main(int argc,char **argv){
    signal(SIGPIPE,SIG_IGN);signal(SIGTERM,stop_signal);signal(SIGINT,stop_signal);
    if(argc==4&&!strcmp(argv[1],"--selftest"))return selftest(argv[2],argv[3]);
    if(argc==2&&!strcmp(argv[1],"--serve"))return serve(9997U);
    fprintf(stderr,"Usage: --selftest OWNED_DIRECTORY H264_FIXTURE | --serve\n");return 2;
}
