/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
/* Exact accepted MU1102 receiver only. All static entry points are guarded
 * independently and are never invoked by the isolated codec/libc test. */
#include "map_zoom.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

typedef const void *(*retain_fn)(const void *);
typedef void (*release_fn)(const void *);
typedef void *(*dict_fn)(void *, int, const void *, const void *);
typedef void *(*string_fn)(void *, const char *, unsigned);
typedef int (*object_fn)(void *, const void *, const void *);
typedef int (*int_fn)(void *, const void *, int64_t);
typedef void (*dispatch_fn)(void *, void *, void (*)(void *));
typedef const void *(*get_object_fn)(const void *,const void *);
typedef int64_t (*get_int_fn)(const void *,const void *,int *);

/* Permit exact original HMI datagrams only; no shell, requestUI or arbitrary
 * protocol dictionaries are accepted from this socket. */
int v75_zoom_command(const char *data,size_t n){
    if(n==7&&!memcmp(data,"zoom-in",7))return 0;
    if(n==8&&!memcmp(data,"zoom-out",8))return 1;
    return -1;
}

#ifndef V75_ZOOM_CORE_ONLY
static int zoomfd=-1;
static size_t zoomlogged;
/* Defined by the receiver interface; wired selection always has priority. */
extern int v75_interface_live(void);
extern const char *v75_interface_scope(void);

static void note(const char *kind,int direction,int rc){
    char line[160];int n=snprintf(line,sizeof(line),"V75_MAP_ZOOM %s direction=%d rc=%d\n",kind,direction,rc);
    if(zoomfd>=0&&n>0&&zoomlogged+(size_t)n<=65536&&write(zoomfd,line,(size_t)n)==n)zoomlogged+=(size_t)n;
}
static const void *session_acquire(void){
    const void *session;
    pthread_mutex_lock((pthread_mutex_t *)0x1109ec);
    session=*(const void *volatile *)0x112a6c;
    if(session)((retain_fn)0x51ce4)(session);
    pthread_mutex_unlock((pthread_mutex_t *)0x1109ec);
    return session;
}
static void *new_dictionary(void){
    return ((dict_fn)0x52060)(NULL,0,(void *)0x20104,(void *)0x2011c);
}
static void *new_string(const char *value){
    return ((string_fn)0x52220)(NULL,value,0x08000100);
}
static void release(const void *value){if(value)((release_fn)0x51ce8)(value);}
static int put_string(void *dict,const char *name,const char *value){
    void *key=new_string(name),*text=new_string(value);int rc=-3;
    if(key&&text)rc=((object_fn)0x520d0)(dict,key,text);
    release(key);release(text);return rc;
}
static int put_object(void *dict,const char *name,const void *value){
    void *key=new_string(name);int rc=-3;
    if(key)rc=((object_fn)0x520d0)(dict,key,value);
    release(key);return rc;
}
static int put_int(void *dict,const char *name,int value){
    void *key=new_string(name);int rc=-3;
    if(key)rc=((int_fn)0x5ada0)(dict,key,(int64_t)value);
    release(key);return rc;
}
static int native_abi_ok(void){
    return *(const uint32_t *)0x52060==0xe92d4800&&*(const uint32_t *)0x52220==0xe92d4070&&
           *(const uint32_t *)0x5ada0==0xe92d4070&&*(const uint32_t *)0x5e790==0xe92d4070&&
           *(const uint32_t *)0xd4774==0xe92d48f0;
}
static int fill_request(void *request,void *params,int direction){
    int rc=put_string(request,"type","changeMapZoomLevel");
    if(!rc)rc=put_string(params,"uuid","b2c3d4e5-0000-4000-8000-00000000c1a5");
    if(!rc)rc=put_int(params,"zoomDirection",direction);
    if(!rc)rc=put_object(request,"params",params);
    return rc;
}
/* Isolated native receiver fixture only. It exercises the actual CFL ABI but
 * never acquires a live session, opens the HMI port, or sends a phone command. */
int v75_zoom_selftest(void){
    int direction,rc=0;void *request=NULL,*params=NULL,*key=NULL;
    if(!native_abi_ok())return -10;
    for(direction=0;direction<=1;direction++){
        int status=-1;int64_t value;
        request=new_dictionary();params=new_dictionary();if(!request||!params){rc=-11;goto done;}
        rc=fill_request(request,params,direction);if(rc)goto done;
        key=new_string("params");if(!key||((get_object_fn)0x520ac)(request,key)!=params){rc=-12;goto done;}
        release(key);key=new_string("zoomDirection");if(!key){rc=-13;goto done;}
        value=((get_int_fn)0x5ad5c)(params,key,&status);if(status||value!=direction){rc=-14;goto done;}
        release(key);key=NULL;release(params);params=NULL;release(request);request=NULL;
    }
done:release(key);release(params);release(request);return rc;
}
/* Exact original native send-command context, same queue/callback used by the
 * receiver's HID/Siri commands. A null completion avoids the old AirPlay
 * library's ambiguity when several completion-bearing commands overlap. */
struct send_context {const void *session,*request;void *completion,*context;int status;};
static __attribute__((noinline)) int send_zoom(int direction){
    const void *session=NULL;void *request=NULL,*params=NULL;struct send_context job;int rc=-1;
    if(direction<0||direction>1||!v75_interface_live())return -1;
    session=session_acquire();if(!session)return -2;
    request=new_dictionary();params=new_dictionary();
    if(!request||!params){rc=-3;goto done;}
    /* Use this receiver's actual alt-display UUID, not the wired display UUID.
     * The UUID object at 0x3fb88 was verified against its display constructor. */
    rc=fill_request(request,params,direction);
    if(!rc){
        /* Do not race teardown or dereference a missing event queue. */
        if(!v75_interface_live()||!*((const unsigned char *)session+0x3b6)||!*(void **)((const char *)session+0x3b8))rc=-4;
        else{
            job.session=session;job.request=request;job.completion=NULL;job.context=NULL;job.status=-5;
            ((dispatch_fn)0x5e790)(*(void **)((const char *)session+0x3b8),&job,(void (*)(void *))0xd4774);
            rc=job.status;
        }
    }
done:release(params);release(request);release(session);return rc;
}
static void *worker(void *unused){
    int fd=-1;struct sockaddr_in address;struct pollfd event;
    (void)unused;
    for(;;){
        if(!v75_interface_live()){
            if(fd>=0){close(fd);fd=-1;note("released_for_inactive_or_wired",-1,0);}
            poll(NULL,0,200);continue;
        }
        if(fd<0){
            fd=socket(AF_INET,SOCK_DGRAM,0);if(fd<0){poll(NULL,0,1000);continue;}
            memset(&address,0,sizeof(address));address.sin_family=AF_INET;
            address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);address.sin_port=htons(7032);
            /* Never share this port with the original wired handler. */
            if(bind(fd,(struct sockaddr *)&address,sizeof(address))){close(fd);fd=-1;poll(NULL,0,1000);continue;}
            note("loopback_ready",-1,0);
        }
        event.fd=fd;event.events=POLLIN;event.revents=0;
        if(poll(&event,1,100)<=0)continue;
        if(event.revents&POLLIN){
            char data[33];struct sockaddr_in peer;socklen_t length=sizeof(peer);ssize_t n;
            int direction,rc;
            n=recvfrom(fd,data,sizeof(data),0,(struct sockaddr *)&peer,&length);
            if(n<=0||length!=sizeof(peer)||peer.sin_family!=AF_INET||ntohl(peer.sin_addr.s_addr)!=INADDR_LOOPBACK)continue;
            direction=v75_zoom_command(data,(size_t)n);if(direction<0){note("ignored_non_zoom",-1,0);continue;}
            rc=send_zoom(direction);note("submitted",direction,rc);
        }
    }
    return NULL;
}
int v75_zoom_start(void){
    char path[100];pthread_t thread;int rc;
    /* Independent exact-instruction guards; the main interface has a full
     * receiver SHA gate before deployment, these are defense in depth. */
    if(!native_abi_ok())return -10;
    snprintf(path,sizeof(path),"%s/zoom.events",v75_interface_scope());
    zoomfd=open(path,O_WRONLY|O_CREAT|O_EXCL,0600);if(zoomfd<0)return -11;
    rc=pthread_create(&thread,NULL,worker,NULL);
    if(rc){close(zoomfd);zoomfd=-1;return rc;}
    pthread_detach(thread);note("worker_started",-1,0);return 0;
}
#endif
