/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
/* Independent exact-receiver candidate. An explicit V75_WCP_CARD profile is
 * required. Read preferences only; no decoder, bus, primary-screen or startup
 * configuration writes. Original primary requestUI/showUI is left untouched.
 */
#include "card_runtime.h"
#include "card_channel.h"
#include "card_session_gate.h"
#include "cpmap_card_preferences.h"
#include "cpmap_projection_preferences.h"
#include "view_area.h"
#include "route_observer.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef const void *(*retain_fn)(const void *);
typedef void (*release_fn)(const void *);
typedef void (*completion_fn)(int,const void *,void *);
typedef void (*dispatch_fn)(void *,void *,void (*)(void *));
typedef const void *(*get_fn)(const void *,const void *);
extern int v75_interface_live(void);
extern const char *v75_interface_scope(void);
#ifndef V75_CARD_COMPILED_CLUSTER
#error An exact receiver profile must be selected by the coupled build
#endif

static pthread_mutex_t cards_lock=PTHREAD_MUTEX_INITIALIZER;
static struct v75_card_channel channel;
static int cards_started,eventfd=-1,baseline_valid,route_observed;
static int phone_route_valid,phone_route;
static uint32_t baseline_sequence;
static size_t event_bytes;
static struct v75_card_cf api;

static void release(const void *p){if(p)((release_fn)0x51ce8)(p);}
static int abi_ok(void)
{
    return *(const uint32_t *)0x52060==0xe92d4800&&*(const uint32_t *)0x52220==0xe92d4070&&
           *(const uint32_t *)0x51d1c==0xe92d4800&&*(const uint32_t *)0x58fb0==0xe92d4800&&
           *(const uint32_t *)0x5ada0==0xe92d4070&&*(const uint32_t *)0x5e790==0xe92d4070&&
           *(const uint32_t *)0xd4774==0xe92d48f0;
}
static void fill_api(struct v75_card_cf *a)
{
    a->dictionary=(void *(*)(void *,int,const void *,const void *))0x52060;
    a->string=(void *(*)(void *,const char *,unsigned))0x52220;
    a->object=(int (*)(void *,const void *,const void *))0x520d0;
    a->integer=(int (*)(void *,const void *,int64_t))0x5ada0;
    a->array=(void *(*)(void *,int,const void *))0x51d1c;
    a->number=(void *(*)(int64_t))0x58fb0;
    a->append=(int (*)(void *,const void *))0x51db4;
    a->release=(void (*)(const void *))0x51ce8;
    a->dictionary_keys=(void *)0x20104;a->dictionary_values=(void *)0x2011c;
    a->array_values=(void *)0x200f0;
}
/* Isolated actual-CFL selftest only. No session lookup, socket, thread, file,
 * live HMI, or phone command. Invoke only with the exact receiver mapped. */
__attribute__((visibility("default"))) int v75_cards_selftest(int cluster)
{
    struct v75_card_cf local;int pos,op,view;void *d,*k;const void *type;
    if(cluster!=V75_CARD_COMPILED_CLUSTER||!v75_card_count(cluster)||!abi_ok())return -10;
    fill_api(&local);
    for(pos=0;pos<4;pos++)for(op=1;op<=3;op++)for(view=0;view<v75_card_count(cluster);view++){
        d=v75_card_request(&local,cluster,op,pos,view);if(!d)return -11;
        k=local.string(NULL,"type",0x08000100);if(!k){release(d);return -12;}
        type=((get_fn)0x520ac)(d,k);release(k);release(d);
        if(!type)return -13;
    }
    return 0;
}
/* All log writes are serialized by cards_lock. Never unbounded RAM logging. */
static void note(const char *kind,uint32_t token,int status,int operation)
{
    char line[240];int n=snprintf(line,sizeof(line),"V75_CARD %s token=%u status=%d operation=%d position=%d navigating=%d view=%d direct_route=%d\n",kind,token,status,operation,channel.position,channel.navigating,channel.policy.desired_view,phone_route_valid);
    if(eventfd>=0&&n>0&&(size_t)n<sizeof(line)&&event_bytes+(size_t)n<=65536&&write(eventfd,line,(size_t)n)==n)event_bytes+=(size_t)n;
}
/* Route state comes directly from the current phone session. No V53
 * parser/cache or navigation-information-to-cluster output is installed. */
static int read_route(int *route,uint32_t *sequence){*route=0;*sequence=0;return 0;}
void v75_cards_new_session(void)
{
    int route,valid;uint32_t seq=0;
    if(!cards_started)return;
    valid=read_route(&route,&seq);
    pthread_mutex_lock(&cards_lock);
    v75_card_channel_session(&channel,0);baseline_valid=valid;baseline_sequence=seq;route_observed=0;
    phone_route_valid=0;phone_route=0;
    note("session_edge",0,0,0);pthread_mutex_unlock(&cards_lock);
}
void v75_cards_route_observed(void)
{
    if(!cards_started)return;
    pthread_mutex_lock(&cards_lock);route_observed=1;pthread_mutex_unlock(&cards_lock);
}
void v75_cards_route_packet(const unsigned char *packet,size_t size)
{
    int route,changed;
    if(!cards_started||!v75_route_packet_state(packet,size,&route))return;
    pthread_mutex_lock(&cards_lock);
    /* This is the current receiver's decoded control packet, not the shared
     * V53 cache. Centering must work even when navigation INFORMATION is off,
     * the parser is catching up, or its sequence restarts at the same value. */
    changed=!phone_route_valid||phone_route!=(route>0);
    phone_route=route>0;phone_route_valid=1;
    if(changed)note("phone_route",0,route,0);
    pthread_mutex_unlock(&cards_lock);
}
static const void *session_acquire(void)
{
    const void *p;
    pthread_mutex_lock((pthread_mutex_t *)0x1109ec);
    p=*(const void *volatile *)0x112a6c;if(p)((retain_fn)0x51ce4)(p);
    pthread_mutex_unlock((pthread_mutex_t *)0x1109ec);
    return p;
}
static int session_ready(const void *p)
{
    return p&&v75_interface_live()&&v75_card_session_allowed(v75_interface_scope(),(long)getpid())&&
      *(const void *volatile *)0x112a6c==p&&
      *((const volatile unsigned char *)p+0x3b6)&&
      *(void *volatile *)((const char *)p+0x3b8)&&*(void *volatile *)((const char *)p+0x3bc);
}
/* ABI established from the retained receiver's 0xdbd20 completion. Its native
 * HTTP message owns the retained session until after this callback returns.
 * context is a nonrecycled integer, NOT a pointer to a dispatch stack frame.
 */
static void completed(int status,const void *response,void *context)
{
    uint32_t token=(uint32_t)(uintptr_t)context;int saved=errno,result;
    (void)response;
    pthread_mutex_lock(&cards_lock);
    result=v75_card_channel_completed(&channel,token,status);
    note(result?"completed":"stale_or_early_callback",token,status,0);
    pthread_mutex_unlock(&cards_lock);errno=saved;
}
struct native_job {const void *session,*request;completion_fn completion;void *context;int status;};
typedef char native_job_size_must_be_20[sizeof(struct native_job)==20?1:-1];
typedef char native_job_callback_offset_must_be_8[offsetof(struct native_job,completion)==8?1:-1];
typedef char native_job_context_offset_must_be_12[offsetof(struct native_job,context)==12?1:-1];
typedef char native_job_status_offset_must_be_16[offsetof(struct native_job,status)==16?1:-1];
static __attribute__((noinline)) int send_command(const void *session,const struct v75_card_dispatch *d)
{
    struct native_job job;void *request;int still_current,rc;
    pthread_mutex_lock(&cards_lock);
    still_current=channel.pending==d->token&&channel.session==d->session&&channel.epoch==d->epoch;
    pthread_mutex_unlock(&cards_lock);
    if(!still_current||!session_ready(session))return -4;
    request=v75_card_request(&api,channel.cluster,d->operation,d->position,d->view);
    if(!request)return -3;
    job.session=session;job.request=request;job.completion=completed;
    job.context=(void *)(uintptr_t)d->token;job.status=-5;
    /* Do not hold cards_lock across dispatch: completion may be synchronous.
     * Original worker serializes the dictionary before dispatch_sync returns;
     * its HTTP object copies completion/context and retains the session.
     */
    if(!session_ready(session))rc=-4;
    else{
        ((dispatch_fn)0x5e790)(*(void *volatile *)((const char *)session+0x3b8),&job,(void (*)(void *))0xd4774);
        rc=job.status;
    }
    release(request);return rc;
}
static void *worker(void *unused)
{
    (void)unused;
    for(;;){
        struct cpmap_projection_preferences prefs;struct cpmap_card_preference card;
        struct v75_card_dispatch dispatch;const void *session=session_acquire();
        int ready=session_ready(session),route=0,valid,observed,old_valid,active=0,take;
        uint32_t seq=0,old_seq;
        pthread_mutex_lock(&cards_lock);
        if(channel.session!=(ready?(uintptr_t)session:0))v75_card_channel_session(&channel,ready?(uintptr_t)session:0);
        old_valid=baseline_valid;old_seq=baseline_sequence;observed=route_observed;
        pthread_mutex_unlock(&cards_lock);
        valid=read_route(&route,&seq)&&v75_view_fresh(old_valid,old_seq,seq,observed);
        if(ready&&cpmap_read_projection_preferences("/mnt/icab/persistent/cpmap-ui",&prefs)>=0&&prefs.map&&!prefs.mirror&&
           cpmap_card_read("/mnt/icab/persistent/cpmap-ui",&card)>=0)active=1;
        pthread_mutex_lock(&cards_lock);
        /* A 1d01 edge may have reset this channel while the files were read. */
        if(!ready||channel.session!=(uintptr_t)session)active=0;
        /* A temporarily unreadable atomic cache is not an "exit navigation"
         * event. Preserve this session's last valid route until a fresh PPS
         * snapshot explicitly reports the new state. */
        if(active)v75_card_channel_desire(&channel,1,card.position,phone_route_valid?phone_route:(valid?route>0:channel.navigating),card.generation);
        else v75_card_channel_desire(&channel,0,channel.position<0?3:channel.position,0,channel.preference_generation);
        take=v75_card_channel_take(&channel,&dispatch);
        if(take==1)note("dispatch_begin",dispatch.token,0,dispatch.operation);
        pthread_mutex_unlock(&cards_lock);
        if(take==1){
            int rc=send_command(session,&dispatch);
            pthread_mutex_lock(&cards_lock);
            v75_card_channel_dispatched(&channel,dispatch.token,rc);
            note("dispatch_return",dispatch.token,rc,dispatch.operation);
            pthread_mutex_unlock(&cards_lock);
        }
        release(session);poll(NULL,0,100);
    }
    return NULL;
}
int v75_cards_start(void)
{
    const char *on=getenv("V75_WCP_CARD");pthread_t thread;char path[100];int cluster,rc,route;
    uint32_t seq=0;
    if(!on)return 0;
    cluster=!strcmp(on,"791")?791:!strcmp(on,"790")?790:0;
    if(cluster!=V75_CARD_COMPILED_CLUSTER||!abi_ok())return -10;
    if(cards_started)return 0;
    fill_api(&api);if(v75_card_channel_init(&channel,cluster))return -11;
    baseline_valid=read_route(&route,&seq);baseline_sequence=seq;
    snprintf(path,sizeof(path),"%s/card.events",v75_interface_scope());
    eventfd=open(path,O_WRONLY|O_CREAT|O_EXCL,0600);if(eventfd<0)return -12;
    cards_started=1;
    rc=pthread_create(&thread,NULL,worker,NULL);
    if(rc){cards_started=0;close(eventfd);eventfd=-1;return rc;}
    pthread_detach(thread);pthread_mutex_lock(&cards_lock);note("worker_started",0,0,cluster);pthread_mutex_unlock(&cards_lock);
    return 0;
}
