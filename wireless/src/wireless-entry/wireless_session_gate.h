/* Owned wireless feed / accepted-session contract. No changes to wired ring,
 * MOST writer, dimensions, IDR interval, navigation, music, UI or SSH. */
#ifndef V75_WIRELESS_SESSION_GATE_H
#define V75_WIRELESS_SESSION_GATE_H
#include <limits.h>
#ifndef V75_GATE_PREFIX
#define V75_GATE_PREFIX ""
#endif
static struct {
    char scope[54];long receiver;unsigned long sequence;
    int valid,allowed,request_idr;uint64_t checked;
} v75_gate;
static int v75_gate_read(const char *logical,char *data,size_t cap){
    char path[512];int fd;ssize_t n,extra,i;char last;
    if(snprintf(path,sizeof(path),V75_GATE_PREFIX "%s",logical)>=(int)sizeof(path))return -1;
    fd=open(path,O_RDONLY);if(fd<0)return errno==ENOENT?0:-1;
    n=read(fd,data,cap-1);extra=read(fd,&last,1);close(fd);
    if(n<=0||extra!=0)return -1;
    for(i=0;i<n;i++)if((data[i]<32&&data[i]!='\n')||(unsigned char)data[i]>126)return -1;
    data[n]=0;return (int)n;
}
static int v75_gate_number(const char *text,unsigned long *value){
    unsigned long n=0;const char *p=text;
    if(*p<'1'||*p>'9')return -1;
    while(*p){if(*p<'0'||*p>'9'||n>(2147483647UL-(unsigned)(*p-'0'))/10UL)return -1;n=n*10UL+(unsigned)(*p-'0');p++;}
    *value=n;return n?0:-1;
}
static int v75_gate_fields(char *data,char **fields,int count){
    int i;char *p=data;
    for(i=0;i<count;i++){char *end=strchr(p,'\n');if(!end)return -1;fields[i]=p;*end=0;p=end+1;}
    return *p?-1:0;
}
static int v75_gate_owner(char scope[54],long *pid){
    char data[101],*fields[3];unsigned long number;unsigned i;
    if(v75_gate_read("/ramdisk/v75-wireless-native.owner",data,sizeof(data))<=0||v75_gate_fields(data,fields,3)||strcmp(fields[0],"V75_NATIVE1")||v75_gate_number(fields[1],&number))return -1;
    if(strlen(fields[2])!=53||strncmp(fields[2],"/ramdisk/.v75-native-",21))return -1;
    for(i=21;i<53;i++)if(!strchr("0123456789abcdef",fields[2][i]))return -1;
    memcpy(scope,fields[2],54);*pid=(long)number;return 0;
}
static int v75_gate_write_state(int active){
    char logical[100],path[512],pending[512],body[100];int fd,n;ssize_t z;
    if(!v75_gate.valid)return -1;
    snprintf(logical,sizeof(logical),"%s/cluster-feed.state",v75_gate.scope);
    snprintf(path,sizeof(path),V75_GATE_PREFIX "%s",logical);
    snprintf(pending,sizeof(pending),V75_GATE_PREFIX "%s/cluster-feed.next",v75_gate.scope);
    n=snprintf(body,sizeof(body),"V75_FEED1\n%ld\n%lu\n%d\n",v75_gate.receiver,v75_gate.sequence,active);
    fd=open(pending,O_WRONLY|O_CREAT|O_EXCL,0600);if(fd<0)return -1;
    z=write(fd,body,(size_t)n);
    if(z!=n||fsync(fd)){close(fd);unlink(pending);return -1;}
    if(close(fd)||rename(pending,path)){unlink(pending);return -1;}return 0;
}
static int v75_gate_begin(void){
    char logical[100],data[101],*fields[4];unsigned long receiver,sequence=0;int n;
    memset(&v75_gate,0,sizeof(v75_gate));
    if(v75_gate_owner(v75_gate.scope,&v75_gate.receiver))return -1;
    snprintf(logical,sizeof(logical),"%s/cluster-feed.state",v75_gate.scope);
    n=v75_gate_read(logical,data,sizeof(data));if(n<0)return -1;
    if(n>0){
        if(v75_gate_fields(data,fields,4)||strcmp(fields[0],"V75_FEED1")||v75_gate_number(fields[1],&receiver)||receiver!=(unsigned long)v75_gate.receiver||v75_gate_number(fields[2],&sequence)||(strcmp(fields[3],"0")&&strcmp(fields[3],"1")))return -1;
    }
    if(sequence>=2147483647UL)return -1;
    v75_gate.sequence=sequence+1;v75_gate.valid=1;
    if(v75_gate_write_state(1)){v75_gate.valid=0;return -1;}
    fprintf(stderr,"V75_CLUSTER_FEED_BEGIN receiver=%ld sequence=%lu policy=pending\n",v75_gate.receiver,v75_gate.sequence);return 0;
}
static int v75_gate_allowed(void){
    char logical[100],data[101],current[54],*fields[3];unsigned long receiver,sequence;
    struct timespec time;uint64_t now=0;long owner;int previous=v75_gate.allowed,allowed=0;
    if(!v75_gate.valid)return 0;
    if(!clock_gettime(CLOCK_MONOTONIC,&time))now=(uint64_t)time.tv_sec*1000000ULL+(uint64_t)time.tv_nsec/1000ULL;
    /* At most 10 small receipt reads/sec, never per-frame shape validation. */
    if(now&&v75_gate.checked&&now>=v75_gate.checked&&now-v75_gate.checked<100000ULL)return previous;
    v75_gate.checked=now;
    if(!v75_gate_owner(current,&owner)&&owner==v75_gate.receiver&&!strcmp(current,v75_gate.scope)){
        snprintf(logical,sizeof(logical),"%s/cluster-session.accepted",v75_gate.scope);
        if(v75_gate_read(logical,data,sizeof(data))>0&&!v75_gate_fields(data,fields,3)&&!strcmp(fields[0],"V75_GRANT1")&&!v75_gate_number(fields[1],&receiver)&&receiver==(unsigned long)owner&&!v75_gate_number(fields[2],&sequence)&&sequence==v75_gate.sequence)allowed=1;
    }
    if(allowed&&!previous)v75_gate.request_idr=1;
    if(allowed!=previous)fprintf(stderr,"V75_CLUSTER_FEED_POLICY sequence=%lu accepted=%d\n",v75_gate.sequence,allowed);
    v75_gate.allowed=allowed;return allowed;
}
static int v75_gate_take_idr(void){int result=v75_gate.request_idr;v75_gate.request_idr=0;return result;}
static void v75_gate_end(void){
    if(v75_gate.valid)(void)v75_gate_write_state(0);
    v75_gate.valid=0;v75_gate.allowed=0;v75_gate.request_idr=0;
}
#endif
