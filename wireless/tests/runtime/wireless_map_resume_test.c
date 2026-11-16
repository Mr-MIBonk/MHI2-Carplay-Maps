/* Actual candidate parser/publisher and socket, no HMI/MOST/audio or phone. */
#define main entry_program_main
#include "wireless_791_entry.c"
#undef main
#include <sys/stat.h>
static unsigned checks;
#define CHECK(x) do{assert(x);++checks;}while(0)
static size_t keys(int fd){
    char b[64];ssize_t n;size_t count=0,i;
    while((n=recv(fd,b,sizeof(b),MSG_DONTWAIT))>0){
        for(i=0;i<(size_t)n;i++)CHECK(b[i]=='K');
        count+=(size_t)n;
    }
    CHECK(n<0&&(errno==EAGAIN||errno==EWOULDBLOCK));return count;
}
int main(int argc,char **argv){
    int pair[2];Input *p;unsigned i,round;unsigned long frame_before;
    const unsigned char sps[]={0x67,0x42,0xc0,0x1f,0xaa};
    const unsigned char pps[]={0x68,0xce,0x3c,0x80};
    const unsigned char idr[]={0x65,0x80,0x11},delta[]={0x41,0x80,0x22};
    if(argc!=2)return 64;
    signal(SIGPIPE,SIG_IGN);
    CHECK(!mkdir(argv[1],0700));CHECK(!paths(argv[1]));offline=1;
    CHECK(!socketpair(AF_UNIX,SOCK_STREAM,0,pair));p=new_input(pair[0]);CHECK(p);
    nal(p,sps,sizeof(sps));nal(p,pps,sizeof(pps));
    /* Pending or disabled map never produces a key request or readiness. */
    for(i=0;i<200;i++)v75_map_resume(p,0,i==0);
    CHECK(!p->map_enabled&&p->waiting_idr&&!p->active);CHECK(keys(pair[1])==0);
    /* First grant and first menu edge coalesce into a single request. */
    v75_map_resume(p,1,1);CHECK(keys(pair[1])==1);
    for(i=0;i<200;i++)v75_map_resume(p,1,0);
    CHECK(keys(pair[1])==0);CHECK(p->waiting_idr&&!p->active);
    nal(p,delta,sizeof(delta));flush_au(p);CHECK(p->frames==0);
    nal(p,idr,sizeof(idr));flush_au(p);CHECK(p->frames==1&&p->active&&!p->waiting_idr);
    CHECK(read_pid(owner_path)==(long)getpid()&&access(ready_path,F_OK)==0);
    CHECK(keys(pair[1])==0);
    /* Same accepted socket, no gate regrant: reproduce the failed menu edge.
     * Publish only after a fresh IDR, keeping old cached buffers protected. */
    for(round=0;round<50;round++){
        frame_before=p->frames;v75_map_resume(p,0,0);
        CHECK(!p->active&&p->waiting_idr&&!p->map_enabled);
        CHECK(access(ready_path,F_OK)!=0&&access(owner_path,F_OK)!=0);
        for(i=0;i<50;i++)v75_map_resume(p,0,0);
        CHECK(keys(pair[1])==0);
        v75_map_resume(p,1,0);CHECK(keys(pair[1])==1);
        for(i=0;i<50;i++)v75_map_resume(p,1,0);
        CHECK(keys(pair[1])==0);
        nal(p,delta,sizeof(delta));flush_au(p);CHECK(p->frames==frame_before&&!p->active);
        nal(p,idr,sizeof(idr));flush_au(p);
        CHECK(p->frames==frame_before+1&&p->active&&!p->waiting_idr);
        CHECK(p->ring&&p->ring->generation==1);
        CHECK(keys(pair[1])==(p->frames%CPMAP_IDR_FRAMES==0?1U:0U));
    }
    /* Continued mode does not add requests: retain exactly IDR5 or IDR180. */
    for(i=0;i<360;i++){
        v75_map_resume(p,1,0);nal(p,idr,sizeof(idr));flush_au(p);
        CHECK(keys(pair[1])==(p->frames%CPMAP_IDR_FRAMES==0?1U:0U));
    }
    /* Revocation followed by rejection cannot resurrect readiness. */
    v75_map_resume(p,0,0);CHECK(!p->active&&access(ready_path,F_OK)!=0);
    CHECK(keys(pair[1])==0);
    close(pair[1]);v75_map_resume(p,1,0);CHECK(p->waiting_idr);
    free_input(p);close(pair[0]);
    printf("V75_MAP_RESUME_PASS checks=%u transitions=50 idr=%lu no_MOST=1 no_audio=1\n",checks,(unsigned long)CPMAP_IDR_FRAMES);
    return 0;
}
