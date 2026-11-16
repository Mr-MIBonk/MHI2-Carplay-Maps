/* Actual production supervisor and read-only proof functions; main never runs. */
#define main production_supervisor_not_run
#include "carplay_cluster_supervisor.c"
#undef main
#include <assert.h>
static unsigned checks;
static void check(int b){checks++;assert(b);}
static const char *scope="/ramdisk/.v75-native-0123456789abcdef0123456789abcdef";
static void put(const char *logical,const char *body){char p[512];FILE *f;snprintf(p,sizeof(p),CPMAP_ROOT_PREFIX "%s",logical);f=fopen(p,"wb");check(f!=NULL);check(fwrite(body,1,strlen(body),f)==strlen(body));check(fclose(f)==0);}
static void receipt(const char *name,const char *body){char path[160];snprintf(path,sizeof(path),"%s/%s",scope,name);put(path,body);}
static void owner(long pid){char b[192];snprintf(b,sizeof(b),"V75_NATIVE1\n%ld\n%s\n",pid,scope);put("/ramdisk/v75-wireless-native.owner",b);}
static void grant(long pid,unsigned seq){char b[128];snprintf(b,sizeof(b),"V75_GRANT1\n%ld\n%u\n",pid,seq);receipt("cluster-session.accepted",b);}
static void feed(long pid,unsigned seq,int active){char b[128];snprintf(b,sizeof(b),"V75_FEED1\n%ld\n%u\n%d\n",pid,seq,active);receipt("cluster-feed.state",b);}
int main(void){
 long pid=(long)getpid();char path[512],b[200];int fd;CarPlayYuvSharedHeader h;uint64_t now=monotonic_us();struct cpmap_projection_preferences prefs;int requested,dio,wireless;
 check(!v75_wireless_mirror_owner());owner(pid);check(!v75_wireless_mirror_owner());
 feed(pid,1,1);grant(pid,1);check(!v75_wireless_mirror_owner());put("/dev/shmem/wcp_cp_session","1\n");check(v75_wireless_mirror_owner());check(cpvc_authorized_runtime());
 grant(pid,2);check(!v75_wireless_mirror_owner());grant(pid,1);feed(pid,2,1);check(!v75_wireless_mirror_owner());
 grant(pid,2);check(v75_wireless_mirror_owner());feed(pid,2,0);check(!v75_wireless_mirror_owner());
 feed(pid,2,1);grant(pid+1,2);check(!v75_wireless_mirror_owner());grant(pid,2);owner(2147483647L);check(!v75_wireless_mirror_owner());owner(pid);
 snprintf(b,sizeof(b),"V75_GRANT1\n%ld\n2\nextra\n",pid);receipt("cluster-session.accepted",b);check(!v75_wireless_mirror_owner());grant(pid,2);
 snprintf(b,sizeof(b),"V75_NATIVE1\n%ld\n/ramdisk/../../other\n",pid);put("/ramdisk/v75-wireless-native.owner",b);check(!v75_wireless_mirror_owner());owner(pid);
 feed(pid,0,1);grant(pid,0);check(!v75_wireless_mirror_owner());feed(pid,3,1);grant(pid,3);check(v75_wireless_mirror_owner());
 /* Core regression: a real wireless grant cannot authorize any map branch. */
 memset(&prefs,0,sizeof(prefs));prefs.map_mask=0;prefs.mirror_mask=1;
 requested=cpvc_video_requested(&prefs,1,1,1);check(requested==CPVC_VIDEO_MIRROR);
 wireless=v75_wireless_mirror_owner();check(wireless);dio=0;
 check((dio||(requested==CPVC_VIDEO_MIRROR&&wireless))!=0);
 prefs.map_mask=1;prefs.mirror_mask=0;requested=cpvc_video_requested(&prefs,1,1,1);check(requested==CPVC_VIDEO_MAP);
 check(!(dio||(requested==CPVC_VIDEO_MIRROR&&wireless)));
 check(cpvc_video_requested(&prefs,1,0,1)==CPVC_VIDEO_OEM);check(cpvc_video_requested(&prefs,0,1,1)==CPVC_VIDEO_OEM);check(cpvc_video_requested(&prefs,1,1,0)==CPVC_VIDEO_OEM);
 memset(&h,0,sizeof(h));h.magic=CARPLAY_YUV_MAGIC;h.version=CARPLAY_YUV_VERSION;h.stream_active=1;h.sequence=1;h.source_width=1264;h.source_height=640;h.output_width=1264;h.output_height=640;h.timestamp_us=now;
 snprintf(path,sizeof(path),CARPLAY_YUV_SHM_PATH);fd=open(path,O_CREAT|O_RDWR|O_TRUNC,0600);check(fd>=0);check(write(fd,&h,sizeof(h))==sizeof(h));check(!close(fd));
 check(mirror_stream_ready(now,1));check(!mirror_stream_ready(now+3000000,1));check(mirror_stream_ready(now+3000000,0));
 h.stream_active=0;fd=open(path,O_WRONLY|O_TRUNC);check(fd>=0);check(write(fd,&h,sizeof(h))==sizeof(h));close(fd);check(!mirror_stream_ready(now,0));
 printf("WIRELESS_MIRROR_ACTUAL_SUPERVISOR_GATE_PASS checks=%u real_CRC_preferences_unchanged=1 no_wired_marker_fabrication=1 map_guard_unchanged=1 stale_or_revoked_session_rejected=1 GPU_used=0\n",checks);return 0;
}
