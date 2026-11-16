/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29.
 * Bounded, preserving OEM JSON-with-comments editor. No shell evaluation.
 * Only known CarPlay and btstack execution paths may change. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CAP 262144
#define TOK 16384
typedef struct {int begin,end,next,kind;} Token;
static char text[CAP+1];static Token tokens[TOK];static int length,pos,count;
static int error;
static void space(void){
    for(;;){
        while(pos<length&&isspace((unsigned char)text[pos]))pos++;
        if(pos<length&&text[pos]=='#'){while(pos<length&&text[pos]!='\n')pos++;continue;}
        if(pos+1<length&&text[pos]=='/'&&text[pos+1]=='/'){
            while(pos<length&&text[pos]!='\n')pos++;continue;
        }
        if(pos+1<length&&text[pos]=='/'&&text[pos+1]=='*'){
            pos+=2;while(pos+1<length&&!(text[pos]=='*'&&text[pos+1]=='/'))pos++;
            if(pos+1>=length){error=1;return;}pos+=2;continue;
        }
        return;
    }
}
static int parse(int depth){
    int id,kind;char closing;space();
    if(error||depth>64||pos>=length||count>=TOK){error=1;return -1;}
    id=count++;tokens[id].begin=pos;kind=(unsigned char)text[pos];tokens[id].kind=kind;
    if(kind=='"'){
        pos++;
        while(pos<length&&text[pos]!='"'){
            unsigned char c=(unsigned char)text[pos++];
            if(c<32){error=1;return -1;}
            if(c=='\\'){
                if(pos>=length){error=1;return -1;}c=(unsigned char)text[pos++];
                if(c=='u'){int i;for(i=0;i<4;i++)if(pos>=length||!isxdigit((unsigned char)text[pos++])){error=1;return -1;}}
                else if(!strchr("\"\\/bfnrt",c)){error=1;return -1;}
            }
        }
        if(pos>=length){error=1;return -1;}pos++;
    }else if(kind=='{'||kind=='['){
        closing=kind=='{'?'}':']';pos++;space();
        if(pos<length&&text[pos]!=closing)for(;;){
            if(kind=='{'){
                if(pos>=length||text[pos]!='"'||parse(depth+1)<0){error=1;return -1;}
                space();if(pos>=length||text[pos++]!=':'){error=1;return -1;}
            }
            if(parse(depth+1)<0)return -1;space();
            if(pos<length&&text[pos]==','){pos++;space();continue;}
            break;
        }
        if(pos>=length||text[pos++]!=closing){error=1;return -1;}
    }else{
        int begin=pos;
        while(pos<length&&!isspace((unsigned char)text[pos])&&!strchr(",]}#",text[pos]))pos++;
        if(pos==begin){error=1;return -1;}
        /* OEM fields not edited here, but malformed punctuation is refused. */
        if(!(text[begin]=='-'||isdigit((unsigned char)text[begin])||
             (pos-begin==4&&!memcmp(text+begin,"true",4))||
             (pos-begin==5&&!memcmp(text+begin,"false",5))||
             (pos-begin==4&&!memcmp(text+begin,"null",4)))){error=1;return -1;}
    }
    tokens[id].end=pos;tokens[id].next=count;return id;
}
static int eq(int id,const char *value){
    size_t n=strlen(value);return id>=0&&tokens[id].kind=='"'&&tokens[id].end-tokens[id].begin==(int)n+2&&
        !memcmp(text+tokens[id].begin+1,value,n);
}
static int member(int object,const char *name){
    int i,found=-1;if(object<0||tokens[object].kind!='{')return -1;
    for(i=object+1;i<tokens[object].next;){
        int value=i+1;
        if(eq(i,name)){if(found>=0)return -2;found=value;}
        i=tokens[value].next;
    }
    return found;
}
static int named_object(const char *name){
    int i,j,found=-1;
    for(i=0;i<count;i++)if(tokens[i].kind=='{'){
        for(j=i+1;j<tokens[i].next;){
            int value=j+1;
            if(eq(j,name)&&tokens[value].kind=='{'){if(found>=0)return -2;found=value;}
            j=tokens[value].next;
        }
    }
    return found;
}
typedef struct {int begin,end;const char *replacement;} Edit;
static Edit edits[4];static int edit_count;
static void replace(int id,const char *replacement){
    if(id<0||edit_count>=4){error=1;return;}
    edits[edit_count].begin=tokens[id].begin;edits[edit_count].end=tokens[id].end;
    edits[edit_count++].replacement=replacement;
}
static int compare(const void *a,const void *b){return ((const Edit*)a)->begin-((const Edit*)b)->begin;}
static char env[CAP];
int main(int argc,char **argv){
    FILE *f;int root,object,exec,path,vars,i,at=0;size_t n,used=1;int kept=0;
    if(argc!=4||(!strcmp(argv[2],argv[3])))return 2;
    f=fopen(argv[2],"rb");if(!f)return 3;n=fread(text,1,CAP,f);
    if(ferror(f)||!feof(f)){fclose(f);return 3;}fclose(f);length=(int)n;
    root=parse(0);space();if(root<0||error||pos!=length||tokens[root].kind!='{')return 4;
    if(!strcmp(argv[1],"connectivity")){
        object=named_object("btstack");exec=member(object,"exec");
        if(!eq(exec,"/eso/bin/apps/btstack")&&!eq(exec,"/mnt/ota/btstack_wrap.sh"))return 5;
        replace(exec,"\"/mnt/ota/btstack_wrap.sh\"");
    }else if(!strcmp(argv[1],"si")){
        object=named_object("carplay");exec=member(object,"exec");path=member(object,"path");vars=member(object,"envs");
        if((!eq(exec,"dio_manager")&&!eq(exec,"dio-manager-plain.sh"))||
           (!eq(path,"/eso/bin/apps")&&!eq(path,"/mnt/app/eso/bin/apps")&&!eq(path,"/mnt/app/carplaymaps")))return 5;
        replace(exec,"\"dio_manager\"");replace(path,"\"/mnt/app/eso/bin/apps\"");
        if(vars<0||tokens[vars].kind!='[')return 5;
        env[0]='[';
        for(i=vars+1;i<tokens[vars].next;i=tokens[i].next){
            int size=tokens[i].end-tokens[i].begin;
            if(tokens[i].kind!='"')return 5;
            if(eq(i,"LD_PRELOAD=/mnt/ota/libcpmap-primary60.so")||eq(i,"CPMAP_PRIMARY_60FPS=1"))continue;
            if(size>12&&!memcmp(text+tokens[i].begin+1,"LD_PRELOAD=",11))return 5;
            if(used+(size_t)size+4>=sizeof(env))return 6;
            if(kept++)env[used++]=',';
            memcpy(env+used,text+tokens[i].begin,(size_t)size);used+=(size_t)size;
        }
        env[used++]=']';env[used]=0;replace(vars,env);
    }else return 2;
    if(error)return 6;
    qsort(edits,(size_t)edit_count,sizeof(edits[0]),compare);
    for(i=0;i<edit_count;i++){if(edits[i].begin<at)return 6;at=edits[i].end;}
    f=fopen(argv[3],"wb");if(!f)return 7;at=0;
    for(i=0;i<edit_count;i++){
        size_t size=(size_t)(edits[i].begin-at);
        if(fwrite(text+at,1,size,f)!=size||fputs(edits[i].replacement,f)==EOF){fclose(f);remove(argv[3]);return 7;}
        at=edits[i].end;
    }
    if(fwrite(text+at,1,(size_t)(length-at),f)!=(size_t)(length-at)||fclose(f)){remove(argv[3]);return 7;}
    return 0;
}
