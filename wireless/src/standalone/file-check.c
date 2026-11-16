/* Local standalone experiment. SHA-256 is integrity, NOT firmware authorization. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
typedef struct {uint32_t h[8];uint64_t n;unsigned char b[64];size_t used;} SHA;
static uint32_t rr(uint32_t x,int n){return (x>>n)|(x<<(32-n));}
static const uint32_t k[64]={
0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
static void block(SHA *s,const unsigned char *b){
 uint32_t w[64],a,c,d,e,f,g,h,t1,t2,bb;int i;
 for(i=0;i<16;i++)w[i]=(uint32_t)b[i*4]<<24|(uint32_t)b[i*4+1]<<16|(uint32_t)b[i*4+2]<<8|b[i*4+3];
 for(i=16;i<64;i++)w[i]=w[i-16]+(rr(w[i-15],7)^rr(w[i-15],18)^(w[i-15]>>3))+w[i-7]+(rr(w[i-2],17)^rr(w[i-2],19)^(w[i-2]>>10));
 a=s->h[0];bb=s->h[1];c=s->h[2];d=s->h[3];e=s->h[4];f=s->h[5];g=s->h[6];h=s->h[7];
 for(i=0;i<64;i++){t1=h+(rr(e,6)^rr(e,11)^rr(e,25))+((e&f)^((~e)&g))+k[i]+w[i];t2=(rr(a,2)^rr(a,13)^rr(a,22))+((a&bb)^(a&c)^(bb&c));h=g;g=f;f=e;e=d+t1;d=c;c=bb;bb=a;a=t1+t2;}
 s->h[0]+=a;s->h[1]+=bb;s->h[2]+=c;s->h[3]+=d;s->h[4]+=e;s->h[5]+=f;s->h[6]+=g;s->h[7]+=h;
}
static void init(SHA *s){static const uint32_t iv[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};memset(s,0,sizeof(*s));memcpy(s->h,iv,sizeof(iv));}
static void update(SHA *s,const unsigned char *p,size_t n){size_t take;s->n+=n;while(n){take=64-s->used;if(take>n)take=n;memcpy(s->b+s->used,p,take);s->used+=take;p+=take;n-=take;if(s->used==64){block(s,s->b);s->used=0;}}}
static void finish(SHA *s,char hex[65]){unsigned char pad[128]={0x80};uint64_t bits=s->n*8;size_t n=s->used<56?56-s->used:120-s->used;int i;update(s,pad,n);for(i=0;i<8;i++)pad[i]=(unsigned char)(bits>>(56-i*8));update(s,pad,8);for(i=0;i<8;i++)sprintf(hex+i*8,"%08x",s->h[i]);hex[64]=0;}
static int hash_file(const char *name,char hex[65]){FILE *f=fopen(name,"rb");unsigned char b[8192];size_t n;SHA s;if(!f)return 1;init(&s);while((n=fread(b,1,sizeof(b),f)))update(&s,b,n);if(ferror(f)){fclose(f);return 1;}fclose(f);finish(&s,hex);return 0;}
static int safe_relative(const char *p){const char *q=p;if(!*p||*p=='/'||strchr(p,'\\'))return 0;while(*q){const char *start=q;size_t n;while(*q&&*q!='/'){if((unsigned char)*q<32)return 0;q++;}n=(size_t)(q-start);if(!n||(n==1&&start[0]=='.')||(n==2&&start[0]=='.'&&start[1]=='.'))return 0;if(*q)q++;}return q[-1]!='/';}
static int regular_below(char *path,size_t root_length){
 struct stat s;size_t n;
 if(lstat(path,&s)||!S_ISREG(s.st_mode))return 0;
 for(n=root_length;path[n];n++)if(path[n]=='/'){
  path[n]=0;
  if(lstat(path,&s)||!S_ISDIR(s.st_mode)){path[n]='/';return 0;}
  path[n]='/';
 }
 return 1;
}
static int manifest(const char *root,const char *name){
 FILE *f=fopen(name,"rb");char line[2048],path[4096],actual[65];char *seen[4096];int count=0,rc=0,i;
 if(!f)return 1;
 while(fgets(line,sizeof(line),f)){
  size_t n=strlen(line);
  if(n<68||line[n-1]!='\n'||line[64]!=' '||line[65]!=' '||count==4096){rc=2;break;}
  line[n-1]=0;
  for(i=0;i<64;i++)if(!((line[i]>='0'&&line[i]<='9')||(line[i]>='a'&&line[i]<='f'))){rc=2;break;}
  if(rc)break;
  if(!safe_relative(line+66)||snprintf(path,sizeof(path),"%s/%s",root,line+66)>=(int)sizeof(path)||!regular_below(path,strlen(root))){rc=2;break;}
  for(i=0;i<count;i++)if(!strcmp(seen[i],line+66)){rc=2;break;}
  if(rc)break;
  if(hash_file(path,actual)||strncmp(actual,line,64)){fprintf(stderr,"INTEGRITY_FAIL=%s\n",line+66);rc=3;break;}
  seen[count]=(char*)malloc(strlen(line+66)+1);if(!seen[count]){rc=4;break;}
  strcpy(seen[count++],line+66);
 }
 if(ferror(f)||!count)rc=2;
 fclose(f);for(i=0;i<count;i++)free(seen[i]);
 if(!rc)printf("LOCAL_MANIFEST_PASS files=%d\n",count);
 return rc;
}
typedef struct {char *relative;char digest[65];} TreeFile;
typedef struct {TreeFile files[4096];size_t count;} Tree;
static void tree_free(Tree *t){size_t i;for(i=0;i<t->count;i++)free(t->files[i].relative);}
static int tree_order(const void *a,const void *b){return strcmp(((const TreeFile*)a)->relative,((const TreeFile*)b)->relative);}
static int tree_scan(Tree *t,const char *root,const char *relative,unsigned depth){
 char path[4096],child[2048];struct stat st;struct dirent *entry;DIR *d;int rc=0;
 if(depth>16||snprintf(path,sizeof(path),"%s%s%s",root,*relative?"/":"",relative)>=(int)sizeof(path)||lstat(path,&st)||!S_ISDIR(st.st_mode))return 2;
 d=opendir(path);if(!d)return 2;
 for(;;){
  size_t size;
  errno=0;entry=readdir(d);if(!entry){if(errno)rc=2;break;}
  if(!strcmp(entry->d_name,".")||!strcmp(entry->d_name,".."))continue;
  if(snprintf(child,sizeof(child),"%s%s%s",relative,*relative?"/":"",entry->d_name)>=(int)sizeof(child)||!safe_relative(child)||
     snprintf(path,sizeof(path),"%s/%s",root,child)>=(int)sizeof(path)||lstat(path,&st)){rc=2;break;}
  if(S_ISDIR(st.st_mode)){if(tree_scan(t,root,child,depth+1)){rc=2;break;}continue;}
  if(!S_ISREG(st.st_mode)||t->count==4096){rc=2;break;}
  size=strlen(child)+1;t->files[t->count].relative=(char*)malloc(size);if(!t->files[t->count].relative){rc=4;break;}
  memcpy(t->files[t->count].relative,child,size);
  if(hash_file(path,t->files[t->count].digest)){free(t->files[t->count].relative);rc=3;break;}
  t->count++;
 }
 if(closedir(d))rc=2;return rc;
}
static int tree_manifest(const char *root,const char *name,int verify){
 Tree *t=(Tree*)calloc(1,sizeof(Tree));FILE *f;size_t i,count=0;int rc;char line[2048];
 if(!t)return 4;rc=tree_scan(t,root,"",0);
 if(rc||!t->count){tree_free(t);free(t);return rc?rc:2;}
 qsort(t->files,t->count,sizeof(t->files[0]),tree_order);
 if(verify){
  rc=manifest(root,name);if(rc){tree_free(t);free(t);return rc;}
  f=fopen(name,"rb");if(!f){tree_free(t);free(t);return 2;}
  while(fgets(line,sizeof(line),f))count++;rc=ferror(f)||count!=t->count?3:0;fclose(f);
 }else{
  f=fopen(name,"wb");if(!f){tree_free(t);free(t);return 2;}
  for(i=0;i<t->count;i++)if(fprintf(f,"%s  %s\n",t->files[i].digest,t->files[i].relative)<0){rc=3;break;}
  if(fclose(f))rc=3;
 }
 tree_free(t);free(t);return rc;
}
int main(int argc,char **argv){
 char actual[65];
 if(argc==3&&!strcmp(argv[1],"--filesystem")){struct statvfs v;if(statvfs(argv[2],&v))return 2;puts((v.f_flag&ST_RDONLY)?"read-only":"read-write");return 0;}
 if(argc==3&&!strcmp(argv[1],"--free-bytes")){struct statvfs v;if(statvfs(argv[2],&v))return 2;printf("%llu\n",(unsigned long long)v.f_bavail*(unsigned long long)v.f_frsize);return 0;}
 if(argc==2&&!strcmp(argv[1],"--self-test")){SHA s;init(&s);update(&s,(const unsigned char*)"abc",3);finish(&s,actual);return strcmp(actual,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");}
 if(argc==3&&!strcmp(argv[1],"--hash")){if(hash_file(argv[2],actual))return 1;puts(actual);return 0;}
 if(argc==4&&!strcmp(argv[1],"--manifest"))return manifest(argv[2],argv[3]);
 if(argc==4&&!strcmp(argv[1],"--tree-hash"))return tree_manifest(argv[2],argv[3],0);
 if(argc==4&&!strcmp(argv[1],"--tree-check"))return tree_manifest(argv[2],argv[3],1);
 fprintf(stderr,"usage: file-check --self-test | --hash FILE | --manifest ROOT MANIFEST\n");return 2;
}
