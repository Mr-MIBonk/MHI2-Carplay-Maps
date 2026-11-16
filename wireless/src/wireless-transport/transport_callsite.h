#ifndef V75_TRANSPORT_CALLSITE_H
#define V75_TRANSPORT_CALLSITE_H
#include <stdint.h>
#include <string.h>
/* Exact pinned custom programs only. OEM USB ownership remains untouched. */
static int v75_transport_callsite(const char *name,uintptr_t base,uintptr_t pc,int directory){
    if(!name)return 0;
    if(!strcmp(name,"/mnt/ota/libbtwcp.so")){
        uintptr_t offset=pc-base;
        return directory?(offset==0x20b68||offset==0x20be4):offset==0x20b54;
    }
    if(!strcmp(name,"/dev/shmem/wcp_receiver"))
        return directory?(pc==0xeb148||pc==0xeb19c):pc==0xeb12c;
    return 0;
}
#endif
