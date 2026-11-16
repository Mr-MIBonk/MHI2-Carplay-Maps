/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
#include "interface_core.h"
#include <string.h>

static unsigned be16(const uint8_t *p){return ((unsigned)p[0]<<8)|p[1];}
static void put16(uint8_t *p,unsigned n){p[0]=(uint8_t)(n>>8);p[1]=(uint8_t)n;}
static int tlvs(const uint8_t *p,size_t n){
    size_t at=0;
    while(at<n){unsigned len;if(n-at<4)return 0;len=be16(p+at);if(len<4||len>n-at)return 0;at+=len;}
    return at==n;
}
int v75_csm_valid(const uint8_t *p,size_t n){
    return p&&n>=6&&n<=V75_CSM_MAX&&p[0]==0x40&&p[1]==0x40&&be16(p+2)==n&&tlvs(p+6,n-6);
}
static int add(uint8_t *d,size_t cap,size_t *at,unsigned id,const void *v,size_t n){
    if(n>65531||*at>cap||n+4>cap-*at)return 0;
    put16(d+*at,(unsigned)n+4);put16(d+*at+2,id);
    if(n)memcpy(d+*at+4,v,n);
    *at+=n+4;return 1;
}
static int has_id(const uint8_t *p,size_t n,unsigned id){
    size_t at;for(at=0;at+2<=n;at+=2)if(be16(p+at)==id)return 1;return 0;
}
size_t v75_prepare_csm(const uint8_t *p,size_t n,uint8_t *d,size_t cap){
    size_t at=6,i;unsigned msg;int component=0,sent=0,received=0;
    static const unsigned tx[]={0x5200,0x5203};
    static const unsigned rx[]={0x5201,0x5202,0x5204};
    if(!v75_csm_valid(p,n)||!d||p==d||cap<6||cap>V75_CSM_MAX)return 0;
    msg=be16(p+4);memcpy(d,p,6);
    if(msg==0x5200){
        const uint8_t id[]={0,1};
        /* The receiver previously sent an empty subscription. Reuse the
         * route-state shape for map/card geometry, with no information sink. */
        if(n!=6){if(n>cap)return 0;memcpy(d,p,n);return n;}
        if(!add(d,cap,&at,0,id,2)||!add(d,cap,&at,1,0,0)||!add(d,cap,&at,2,0,0)||!add(d,cap,&at,3,0,0))return 0;
    }else if(msg==0x1d01){
        for(i=6;i<n;){
            unsigned len=be16(p+i),id=be16(p+i+2);const uint8_t *v=p+i+4;size_t vn=len-4;
            if(id==6||id==7){
                const unsigned *wanted=id==6?tx:rx;unsigned count=id==6?2:3,j;
                uint8_t ids[512];size_t used=vn;
                if(vn>sizeof(ids)-6||(vn&1))return 0;
                memcpy(ids,v,vn);
                for(j=0;j<count;j++)if(!has_id(ids,used,wanted[j])){put16(ids+used,wanted[j]);used+=2;}
                if(!add(d,cap,&at,id,ids,used))return 0;
                if(id==6)sent++;else received++;
            }else{
                if(id==0x1e)component++;
                if(!add(d,cap,&at,id,v,vn))return 0;
            }
            i+=len;
        }
        if(sent!=1||received!=1||component>1)return 0;
        if(!component){
            static const char name[]="omonob Wireless Maps";
            uint8_t c[128],value[2];size_t z=0;unsigned k;
            put16(value,1);if(!add(c,sizeof(c),&z,0,value,2)||!add(c,sizeof(c),&z,1,name,sizeof(name)))return 0;
            for(k=2;k<=8;k++){put16(value,(k==6||k==8)?6:0x100);if(!add(c,sizeof(c),&z,k,value,2))return 0;}
            if(!add(d,cap,&at,0x1e,c,z))return 0;
        }
    }else{
        if(n>cap)return 0;
        memcpy(d,p,n);return n;
    }
    put16(d+2,(unsigned)at);return at;
}
int v75_button_command(const char *s,unsigned *key,unsigned *down){
    /* Do not interpret arbitrary text, long presses or diagnostic input as
     * media commands. State is key-down=0, key-up=1 in the native WcpCp path. */
    unsigned k,d;size_t z=0;
    if(!s||strncmp(s,"btn ",4))return 0;
    s+=4;
    if(s[0]=='9'){k=9;z=1;}else if(s[0]=='1'&&s[1]=='0'){k=10;z=2;}else return 0;
    if(s[z]!=' '||(s[z+1]!='0'&&s[z+1]!='1')||s[z+2])return 0;
    d=s[z+1]-'0';*key=k;*down=d;return 1;
}
