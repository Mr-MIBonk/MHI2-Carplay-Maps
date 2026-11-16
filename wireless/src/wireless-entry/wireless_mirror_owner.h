#ifndef V75_WIRELESS_MIRROR_OWNER_H
#define V75_WIRELESS_MIRROR_OWNER_H
/* Read-only wireless ownership proof. Never fabricates the wired dio marker,
 * a guard receipt, a video frame or a preference. Only mirror mode may use it.
 */
static int v75_mirror_read(const char *path,char *out,size_t cap){
 struct stat st;FILE *f;size_t n;int bad;
 if(lstat(path,&st)||!S_ISREG(st.st_mode)||st.st_size<1||st.st_size>=(off_t)cap)return 0;
 f=fopen(path,"rb");if(!f)return 0;n=fread(out,1,cap-1,f);bad=ferror(f);if(fclose(f)||bad||n!=(size_t)st.st_size||memchr(out,0,n))return 0;out[n]=0;return 1;
}
static int v75_wireless_mirror_owner(void){
 char b[192],scope[54],path[160],canonical[192];long pid,feedpid,grantpid;unsigned seq,grantseq;int used=0,n;unsigned i;
 if(!v75_mirror_read(CPMAP_ROOT_PREFIX "/ramdisk/v75-wireless-native.owner",b,sizeof(b))||sscanf(b,"V75_NATIVE1\n%ld\n%53[^\n]\n%n",&pid,scope,&used)!=2||used!=(int)strlen(b)||pid<2||pid>2147483647L||strlen(scope)!=53||strncmp(scope,"/ramdisk/.v75-native-",21))return 0;
 for(i=21;i<53;i++)if(!strchr("0123456789abcdef",scope[i]))return 0;
 n=snprintf(canonical,sizeof(canonical),"V75_NATIVE1\n%ld\n%s\n",pid,scope);if(n<0||strcmp(canonical,b))return 0;
 if(kill((pid_t)pid,0)&&errno!=EPERM)return 0;
 if(snprintf(path,sizeof(path),CPMAP_ROOT_PREFIX "%s/cluster-feed.state",scope)>=(int)sizeof(path)||!v75_mirror_read(path,b,sizeof(b)))return 0;
 used=0;if(sscanf(b,"V75_FEED1\n%ld\n%u\n1\n%n",&feedpid,&seq,&used)!=2||used!=(int)strlen(b)||feedpid!=pid||!seq||seq>2147483647U)return 0;
 n=snprintf(canonical,sizeof(canonical),"V75_FEED1\n%ld\n%u\n1\n",pid,seq);if(n<0||strcmp(canonical,b))return 0;
 if(snprintf(path,sizeof(path),CPMAP_ROOT_PREFIX "%s/cluster-session.accepted",scope)>=(int)sizeof(path)||!v75_mirror_read(path,b,sizeof(b)))return 0;
 used=0;if(sscanf(b,"V75_GRANT1\n%ld\n%u\n%n",&grantpid,&grantseq,&used)!=2||used!=(int)strlen(b)||grantpid!=pid||grantseq!=seq)return 0;
 n=snprintf(canonical,sizeof(canonical),"V75_GRANT1\n%ld\n%u\n",pid,seq);if(n<0||strcmp(canonical,b))return 0;
 return access(CPMAP_ROOT_PREFIX "/dev/shmem/wcp_cp_session",F_OK)==0;
}
#endif
