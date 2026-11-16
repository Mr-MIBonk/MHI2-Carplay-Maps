/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
#ifndef V75_ROUTE_OBSERVER_H
#define V75_ROUTE_OBSERVER_H
#include <stddef.h>
#include <stdint.h>
/* Observe ONLY an explicit RouteGuidanceUpdate state from this live phone.
 * No synthetic route, cached packet replay, PPS writer or subscription. */
static int v75_route_packet_state(const uint8_t *p,size_t n,int *state)
{
 size_t off=6;int found=0,value=0;
 if(!p||!state||n<6||n>4096||p[0]!=0x40||p[1]!=0x40||
    (((unsigned)p[2]<<8)|p[3])!=n||p[4]!=0x52||p[5]!=0x01)return 0;
 while(off<n){
  unsigned length,id;
  if(n-off<4)return 0;
  length=((unsigned)p[off]<<8)|p[off+1];id=((unsigned)p[off+2]<<8)|p[off+3];
  if(length<4||length>n-off)return 0;
  if(id==1){if(found||length!=5)return 0;found=1;value=p[off+4];}
  off+=length;
 }
 if(!found)return 0;
 *state=value;return 1;
}
#endif
