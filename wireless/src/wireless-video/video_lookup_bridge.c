/* The pinned wireless video binary imports this unique alias instead of
 * dlsym. We never interpose the system-wide dlsym function itself.
 * All unhandled lookups are delegated to the original handle verbatim.
 */
#include <dlfcn.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
extern int32_t NvSSVideoStreamConfigure(void *,const void *);
extern int32_t NvSSVideoClose(void *);
extern int32_t NvMediaVideoMixerRender(void *,uint32_t,void *,void *,void *,void *,void *,void *,void *);
extern void v75_capture_bind(void *,const char *,void *);
void *v75ds(void *handle,const char *name){
    void *original=dlsym(handle,name);
    const char *enabled=getenv("V75_WCP_VIDEO_TAP");
    if(!original||!enabled||strcmp(enabled,"1"))return original;
    if(!strcmp(name,"NvSSVideoStreamConfigure")){v75_capture_bind(handle,name,original);return (void *)NvSSVideoStreamConfigure;}
    if(!strcmp(name,"NvSSVideoClose")){v75_capture_bind(handle,name,original);return (void *)NvSSVideoClose;}
    if(!strcmp(name,"NvMediaVideoMixerRender")){v75_capture_bind(handle,name,original);return (void *)NvMediaVideoMixerRender;}
    return original;
}
