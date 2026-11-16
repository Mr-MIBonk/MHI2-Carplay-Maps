#ifndef CPVC_AUTHORIZED_RUNTIME_H
#define CPVC_AUTHORIZED_RUNTIME_H
/* Public GPL runtime: no device binding, password changes or paid guard.
 * The historical function name is retained for source compatibility only.
 * It checks a live owning DIO process or exact accepted wireless session, not an entitlement. */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#ifndef CPMAP_ROOT_PREFIX
#define CPMAP_ROOT_PREFIX ""
#endif
#ifndef CPVC_PREF_DIR
#define CPVC_PREF_DIR CPMAP_ROOT_PREFIX "/mnt/icab/persistent/cpmap-ui"
#endif
#include "wireless_mirror_owner.h"
#define CPVC_DIO_OWNER_PID CPMAP_ROOT_PREFIX "/ramdisk/carplay-dio-manager.pid"
static int cpvc_authorized_runtime(void) {
    FILE *f=fopen(CPVC_DIO_OWNER_PID,"r"); long pid=0;
    if(!f)return v75_wireless_mirror_owner();
    if(fscanf(f,"%ld",&pid)!=1)pid=0;
    fclose(f);
    return (pid>1 && (kill((pid_t)pid,0)==0 || errno==EPERM)) || v75_wireless_mirror_owner();
}
#endif
