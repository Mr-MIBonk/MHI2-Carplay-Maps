/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
#ifndef V75_TRANSPORT_PREFERENCE_H
#define V75_TRANSPORT_PREFERENCE_H
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#ifndef V75_TRANSPORT_ROOT
#define V75_TRANSPORT_ROOT ""
#endif
#ifndef V75_TRANSPORT_INTENT
#define V75_TRANSPORT_INTENT V75_TRANSPORT_ROOT "/ramdisk/v75-wireless-transport.intent"
#endif
#ifndef V75_TRANSPORT_USB_PRIMARY
#define V75_TRANSPORT_USB_PRIMARY V75_TRANSPORT_ROOT "/dev/ipod0"
#define V75_TRANSPORT_USB_SECONDARY V75_TRANSPORT_ROOT "/dev/io-cdd/ipod0"
#endif
static int v75_transport_override(void){
    struct stat st,named,usb;struct timeval time;unsigned char raw[64];
    const char prefix[]="V75_TRANSPORT1\n";uint64_t expires=0,now;ssize_t n;size_t i;int fd,ok=0,saved=errno;
    if(lstat(V75_TRANSPORT_INTENT,&named)||!S_ISREG(named.st_mode)||named.st_uid!=getuid()||
        named.st_nlink!=1||named.st_size<16||named.st_size>48)goto done;
    fd=open(V75_TRANSPORT_INTENT,O_RDONLY);if(fd<0)goto done;
    if(fstat(fd,&st)||st.st_dev!=named.st_dev||st.st_ino!=named.st_ino){close(fd);goto done;}
    n=read(fd,raw,sizeof(raw));close(fd);
    if(n<16||memcmp(raw,prefix,sizeof(prefix)-1))goto done;
    for(i=sizeof(prefix)-1;i<(size_t)n-1;i++){
        unsigned digit=raw[i]-'0';if(digit>9||expires>UINT64_C(999999999999999))goto done;
        expires=expires*10+digit;
    }
    if(raw[n-1]!='\n'||gettimeofday(&time,NULL))goto done;
    now=(uint64_t)time.tv_sec*1000+(unsigned)time.tv_usec/1000;
    if(expires<=now||expires-now>120000)goto done;
    /* Explicit preference is confined to this already-attached USB episode.
     * Remove it on unplug, so a new cable insertion wins automatically.
     */
    if(lstat(V75_TRANSPORT_USB_PRIMARY,&usb)&&lstat(V75_TRANSPORT_USB_SECONDARY,&usb)){
        unlink(V75_TRANSPORT_INTENT);goto done;
    }
    ok=1;
done:errno=saved;return ok;
}
#endif
