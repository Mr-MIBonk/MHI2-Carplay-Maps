/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
#ifndef V75_CARD_SESSION_GATE_H
#define V75_CARD_SESSION_GATE_H
/* Read the existing connection-policy grant; never manufacture a session.
 * A paired AirPlay control connection alone is not accepted CarPlay.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifndef V75_CARD_GATE_ROOT
#define V75_CARD_GATE_ROOT ""
#endif
static int v75_card_gate_read(const char *logical,char body[160])
{
    char path[512];struct stat st;FILE *f;size_t n;int bad;
    if(snprintf(path,sizeof(path),V75_CARD_GATE_ROOT "%s",logical)>=(int)sizeof(path)||
       lstat(path,&st)||!S_ISREG(st.st_mode)||st.st_size<1||st.st_size>=160)return 0;
    f=fopen(path,"rb");if(!f)return 0;
    n=fread(body,1,159,f);bad=ferror(f);if(fclose(f)||bad||n!=(size_t)st.st_size||memchr(body,0,n))return 0;
    body[n]=0;return 1;
}
static int v75_card_gate_number(const char *s,unsigned long *out)
{
    unsigned long n=0;if(*s<'1'||*s>'9')return 0;
    for(;*s;s++){unsigned digit=(unsigned char)*s-'0';if(digit>9||n>(2147483647UL-digit)/10UL)return 0;n=n*10+digit;}
    *out=n;return 1;
}
static int v75_card_gate_fields(char *body,char **parts,int count)
{
    int i;char *p=body;for(i=0;i<count;i++){char *e=strchr(p,'\n');if(!e)return 0;*e=0;parts[i]=p;p=e+1;}return !*p;
}
static __attribute__((noinline)) int v75_card_session_allowed(const char *scope,long pid)
{
    char body[160],path[128],*p[4];unsigned long owner,feed,grant;size_t i;
    if(!scope||strlen(scope)!=53||strncmp(scope,"/ramdisk/.v75-native-",21)||pid<1||pid>2147483647L)return 0;
    for(i=21;i<53;i++)if(!strchr("0123456789abcdef",scope[i]))return 0;
    if(!v75_card_gate_read("/ramdisk/v75-wireless-native.owner",body)||!v75_card_gate_fields(body,p,3)||
       strcmp(p[0],"V75_NATIVE1")||!v75_card_gate_number(p[1],&owner)||owner!=(unsigned long)pid||strcmp(p[2],scope))return 0;
    snprintf(path,sizeof(path),"%s/cluster-feed.state",scope);
    if(!v75_card_gate_read(path,body)||!v75_card_gate_fields(body,p,4)||strcmp(p[0],"V75_FEED1")||
       !v75_card_gate_number(p[1],&owner)||owner!=(unsigned long)pid||!v75_card_gate_number(p[2],&feed)||strcmp(p[3],"1"))return 0;
    snprintf(path,sizeof(path),"%s/cluster-session.accepted",scope);
    if(!v75_card_gate_read(path,body)||!v75_card_gate_fields(body,p,3)||strcmp(p[0],"V75_GRANT1")||
       !v75_card_gate_number(p[1],&owner)||owner!=(unsigned long)pid||!v75_card_gate_number(p[2],&grant)||grant!=feed)return 0;
    return 1;
}
#endif
