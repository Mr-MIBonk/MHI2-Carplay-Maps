/* Narrow compatibility shim for two exact, locally pinned custom programs.
 * Does NOT hide USB from the factory driver, HMI, mm-ipod, SSH or other apps.
 * Only their existing "give way to USB" probes honor the explicit RAM lease.
 */
#define _GNU_SOURCE
#include "transport_preference.h"
#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "transport_callsite.h"
static int enabled;
static int (*real_access)(const char *,int);
static DIR *(*real_opendir)(const char *);
static int caller_allowed(void *caller,int directory){
    Dl_info info;uintptr_t pc=(uintptr_t)caller;
    if(!enabled||!dladdr(caller,&info)||!info.dli_fname)return 0;
    return v75_transport_callsite(info.dli_fname,(uintptr_t)info.dli_fbase,pc,directory);
}
int access(const char *path,int mode){
    int result,saved;
    if(!real_access){errno=ENOSYS;return -1;}
    result=real_access(path,mode);
    saved=errno;
    if(result==0&&mode==F_OK&&!strcmp(path,"/dev/ipod0")&&
        caller_allowed(__builtin_return_address(0),0)&&v75_transport_override()){
        errno=ENOENT;return -1;
    }
    errno=saved;return result;
}
DIR *opendir(const char *path){
    if(!real_opendir){errno=ENOSYS;return NULL;}
    if((!strcmp(path,"/fs")||!strcmp(path,"/ramdisk/pps/device"))&&
        caller_allowed(__builtin_return_address(0),1)&&v75_transport_override()){
        errno=ENOENT;return NULL;
    }
    return real_opendir(path);
}
__attribute__((constructor))static void initialize(void){
    const char *value=getenv("V75_WCP_TRANSPORT");
    real_access=dlsym(RTLD_NEXT,"access");real_opendir=dlsym(RTLD_NEXT,"opendir");
    enabled=value&&!strcmp(value,"1")&&real_access&&real_opendir;
}
