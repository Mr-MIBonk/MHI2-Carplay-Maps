/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
#include "interface_core.h"
#include "map_zoom.h"
#include "card_runtime.h"
#include "transport_preference.h"
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

__attribute__((visibility("hidden"))) void *v75_real_fprintf;
__attribute__((visibility("hidden"))) void *v75_real_memcpy;
__attribute__((visibility("hidden"))) void *v75_real_free;
static int enabled,logfd=-1,hid_enabled=1,hid_ready;
static char scope[64];
static int live(void){return enabled&&((access("/dev/ipod0",F_OK)!=0&&access("/dev/io-cdd/ipod0",F_OK)!=0)||v75_transport_override());}
__attribute__((visibility("hidden"))) int v75_interface_live(void){return live();}
__attribute__((visibility("hidden"))) const char *v75_interface_scope(void){return scope;}

__attribute__((visibility("hidden"))) void v75_add_hid_regs(uint32_t *r){
    typedef int (*add_t)(void **,unsigned,const char *,unsigned,unsigned,unsigned,const void *,unsigned,const void *);
    static const uint8_t descriptor[]={0x05,0x0c,0x09,0x01,0xa1,0x01,0x15,0x00,0x25,0x01,0x09,0xb5,0x09,0xb6,0x75,0x01,0x95,0x02,0x81,0x02,0x75,0x06,0x95,0x01,0x81,0x03,0xc0};
    int saved_errno=errno,rc;
    /* r points to the wrapper's 56-byte register save. Array is in the
     * original property callback frame, not in this wrapper's C frame. */
    if(live()&&hid_enabled&&r[13]==0xe4824&&r[4]==0&&r[5]==0x3fb41){
        void **array=(void **)((uint8_t *)r+56+0x40);
        if(*array){
            rc=((add_t)0xdad24)(array,0x56373503,"omonob Map Controls",0,0,0,descriptor,sizeof(descriptor),(void *)0x3fb41);
            if(rc==0)hid_ready=1;
            {char s[100];int n=snprintf(s,sizeof(s),"V75_HID_ADVERTISE rc=%d uid=56373503 bytes=%u\n",rc,(unsigned)sizeof(descriptor));if(logfd>=0)write(logfd,s,n);}
        }
    }
    errno=saved_errno;
}
static int forward_button(unsigned key,unsigned state){
    typedef const void *(*retain_t)(const void *);
    typedef void (*release_t)(const void *);
    typedef int (*send_t)(const void *,unsigned,const void *,size_t);
    const void *session;uint8_t report;int rc;
    if(!live()||!hid_enabled||!hid_ready)return -1;
    /* Same mutex/session lifetime recipe as original touch sender 0xead34.
     * SendHIDReport serializes onto the receiver session's own queue. */
    pthread_mutex_lock((pthread_mutex_t *)0x1109ec);
    session=*(const void *volatile *)0x112a6c;
    if(session)((retain_t)0x51ce4)(session);
    pthread_mutex_unlock((pthread_mutex_t *)0x1109ec);
    if(!session)return -2;
    report=state==0?(key==9?1:2):0;
    rc=((send_t)0xdac24)(session,0x56373503,&report,1);
    ((release_t)0x51ce8)(session);return rc;
}


/* Only map/card route state is consumed. No music journal, artwork files,
 * V53 input, BAP/PPS information output or metadata subscription is added. */
__attribute__((visibility("hidden"))) void v75_observe_regs(uint32_t *r){
    int saved_errno=errno;
    if(live()){
        if(r[13]==0xecbcc&&r[5]>=0x134608&&r[5]<0x135608&&r[9]<=4090){
            const uint8_t *p=(const uint8_t *)(uintptr_t)r[5];size_t n=r[9]+6;
            if(r[5]+n<=0x135608&&v75_csm_valid(p,n)&&r[4]==0x5201)
                v75_cards_route_packet(p,n);
        }else if(r[13]==0xeba38&&r[1]==0x1bad1){
            const char *command=(const char *)(uintptr_t)r[2];unsigned key,state;
            if(v75_button_command(command,&key,&state))(void)forward_button(key,state);
        }
    }
    errno=saved_errno;
}

__attribute__((visibility("hidden"))) void v75_copy_link(void *dst,const void *src,size_t n,uint32_t *r){
    typedef void *(*copy_t)(void *,const void *,size_t);
    uint8_t next[V75_TX_MAX],*p=dst;size_t length=0;int saved_errno=errno;
    if(v75_real_memcpy)((copy_t)v75_real_memcpy)(dst,src,n);
    else{size_t i;for(i=0;i<n;i++)p[i]=((const uint8_t *)src)[i];}
    if(live()&&r[13]==0xedd9c&&r[7]==0x133e08&&r[0]==0x133e11&&r[5]==n&&r[4]==n+10&&r[8]==n+10&&n<=V75_TX_MAX){
        if(n>=6&&((p[4]==0x1d&&p[5]==1)||(p[4]==0x52&&p[5]==0)))
            length=v75_prepare_csm(p,n,next,sizeof(next));
        if(length){
            uint8_t *header=p-9;unsigned i,sum=0;
            for(i=0;i<length;i++)p[i]=next[i];
            header[2]=(uint8_t)((length+10)>>8);header[3]=(uint8_t)(length+10);
            for(i=0;i<8;i++)sum+=header[i];header[8]=(uint8_t)(0-sum);
            r[5]=length;r[4]=length+10;r[8]=length+10;
            if(p[4]==0x1d&&p[5]==1)v75_cards_new_session();
        }
    }
    errno=saved_errno;
}

static void publish_owner(void){
    char pending[100],value[110];int fd,n;
    snprintf(pending,sizeof(pending),"%s/boot-owner.next",scope);
    fd=open(pending,O_WRONLY|O_CREAT|O_EXCL,0600);if(fd<0)return;
    n=snprintf(value,sizeof(value),"V75_NATIVE1\n%ld\n%s\n",(long)getpid(),scope);
    if(write(fd,value,(size_t)n)!=n||fsync(fd)){close(fd);unlink(pending);return;}close(fd);
    if(rename(pending,"/ramdisk/v75-wireless-native.owner"))unlink(pending);
}

__attribute__((constructor)) static void init(void){
    const char *on=getenv("V75_WCP_NATIVE"),*directory=getenv("V75_WCP_SCOPE");size_t i,n;
    v75_real_memcpy=dlsym(RTLD_NEXT,"memcpy");v75_real_fprintf=dlsym(RTLD_NEXT,"fprintf");v75_real_free=dlsym(RTLD_NEXT,"free");
    if(!on||strcmp(on,"1")||!directory||!v75_real_memcpy||!v75_real_fprintf||!v75_real_free)return;
    n=strlen(directory);if(n!=53||strncmp(directory,"/ramdisk/.v75-native-",21))return;
    for(i=21;i<n;i++)if(!strchr("0123456789abcdef",directory[i]))return;
    if(*(const uint32_t *)0xecbc8!=0xeb000c18||*(const uint32_t *)0xedd98!=0xeb0006cc||*(const uint32_t *)0xeddd8!=0xe1a03000)return;
    if(*(const uint32_t *)0xe4820!=0xeb002c4a||*(const uint32_t *)0xdad24!=0xe92d4ff0)return;
    memcpy(scope,directory,n+1);enabled=1;
    {char path[100];snprintf(path,sizeof(path),"%s/native.events",scope);logfd=open(path,O_WRONLY|O_CREAT|O_EXCL,0600);}
    (void)v75_zoom_start();(void)v75_cards_start();publish_owner();
}

/* Retain the R26 thread-local main/cluster classification fix exactly. */
#include "stream_identity.c"
