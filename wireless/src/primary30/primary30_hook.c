#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "primary30.h"
#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef CPMAP_HMI_STARTUP
#define CPMAP_HMI_STARTUP "/mnt/app/eso/hmi/lsd/lsd.sh"
#endif

/* This library must follow the existing alternate-screen/security modules
 * in LD_PRELOAD. Its RTLD_NEXT is still the stock function; the surrounding
 * project hook can add the unchanged alternate display to the returned copy.
 * No polling thread, encoder/decoder/DisplayManager or media ownership hook. */
typedef void *(*CopyInfo)(void *, const void *, const void *, int32_t *);
/* HMI and dio_manager may have different launcher parents. A bounded read
 * of the committed owned UI block therefore supplies the same opt-in even
 * when the HMI environment is not inherited. Never source/execute the file. */
static int number(const char *name)
{
    const char *s = getenv(name);
    int n=0, i=0;
    if (!s || !*s) return 0;
    for (; s[i] && i<5; ++i) {
        if (s[i] < '0' || s[i] > '9') return 0;
        n = n*10+s[i]-'0';
    }
    return s[i] ? 0 : n;
}
static void load_api(V74CF *c)
{
    memset(c,0,sizeof(*c));
#define LOAD(field, symbol) *(void **)(&c->field) = dlsym(RTLD_NEXT,symbol)
    LOAD(type,"CFGetTypeID"); LOAD(dict_type,"CFDictionaryGetTypeID");
    LOAD(array_type,"CFArrayGetTypeID"); LOAD(string,"CFStringCreateWithCString");
    LOAD(get,"CFDictionaryGetValue"); LOAD(dict_count,"CFDictionaryGetCount");
    LOAD(dict_apply,"CFDictionaryApplyFunction"); LOAD(dict_new,"CFDictionaryCreateMutable");
    LOAD(set,"CFDictionarySetValue"); LOAD(set_int,"CFDictionarySetInt64");
    LOAD(get_int,"CFDictionaryGetInt64"); LOAD(array_count,"CFArrayGetCount");
    LOAD(item,"CFArrayGetValueAtIndex"); LOAD(array_copy,"CFArrayCreateMutableCopy");
    LOAD(array_set,"CFArraySetValueAtIndex"); LOAD(release,"CFRelease");
#undef LOAD
    c->dict_keys = dlsym(RTLD_NEXT,"kCFLDictionaryKeyCallBacksCFLTypes");
    c->dict_values = dlsym(RTLD_NEXT,"kCFLDictionaryValueCallBacksCFLTypes");
}
void *AirPlayCopyServerInfo(void *server, const void *device,
                            const void *interface_address, int32_t *error)
{
    CopyInfo real;
    V74CF cf;
    void *original, *copy;
    int saved_errno, result;
    const char *enabled;
    *(void **)(&real) = dlsym(RTLD_NEXT,"AirPlayCopyServerInfo");
    if (!real) { if (error) *error=-1; return NULL; }
    original = real(server,device,interface_address,error);
    saved_errno = errno;
    enabled = getenv("CPMAP_PRIMARY_30FPS");
    if (!original || (error && *error) ||
        (!enabled || strcmp(enabled,"1")!=0)) {
        errno=saved_errno; return original;
    }
    load_api(&cf);
    copy = v74_primary30_copy(&cf,original,1,
        getenv("CPMAP_PRIMARY_WIDTH") ? number("CPMAP_PRIMARY_WIDTH") : 0,
        getenv("CPMAP_PRIMARY_HEIGHT") ? number("CPMAP_PRIMARY_HEIGHT") : 0,&result);
    if (copy) {
        cf.release(original); /* transfer the Copy API's +1, not a borrowed child */
        original=copy;
    }
    errno=saved_errno;
    return original;
}
