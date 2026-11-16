/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
/* Reader/CF request construction only. No file, socket, driver or HMI writes. */
#include "card_protocol.h"
#include <stddef.h>
int v75_card_count(int c){return c==791?2:c==790?5:0;}
int v75_card_view(int c,int p,int nav){
 if(p<0||p>3)return -1;
 if(c==791)return p==3?0:1;
 if(c!=790)return -1;
 if(p==3)return nav?4:0;
 if(p==0)return 3;
 return nav?(p==1?1:2):0;
}
const char *v75_card_url(int c,int p){
 if(p<0||p>3||!v75_card_count(c))return NULL;
 if(c==791)return p==3||p==0?"maps:/car/instrumentcluster/map":"maps:/car/instrumentcluster?showETA=no&maneuverLayout=topAligned&showSpeedLimit=no";
 if(p==3)return "maps:/car/instrumentcluster/map?showETA=no&maneuverLayout=no&showSpeedLimit=no";
 if(p==0)return "maps:/car/instrumentcluster?showETA=no&maneuverLayout=topAligned&showSpeedLimit=no";
 if(p==1)return "maps:/car/instrumentcluster?showETA=no&maneuverLayout=leftAligned&showSpeedLimit=no";
 return "maps:/car/instrumentcluster?showETA=no&maneuverLayout=rightAligned&showSpeedLimit=no";
}
static void *dictionary(const struct v75_card_cf *a){return a->dictionary(NULL,0,a->dictionary_keys,a->dictionary_values);}
static int object(const struct v75_card_cf *a,void *d,const char *name,const void *v){
 void *key=a->string(NULL,name,0x08000100);int rc=-1;
 if(key&&v)rc=a->object(d,key,v);if(key)a->release(key);return rc;
}
static int text(const struct v75_card_cf *a,void *d,const char *name,const char *value){
 void *v=a->string(NULL,value,0x08000100);int rc=object(a,d,name,v);if(v)a->release(v);return rc;
}
static int integer(const struct v75_card_cf *a,void *d,const char *name,int value){
 void *key=a->string(NULL,name,0x08000100);int rc=key?a->integer(d,key,value):-1;if(key)a->release(key);return rc;
}
void *v75_card_request(const struct v75_card_cf *a,int cluster,int op,int pos,int view){
 void *d=NULL,*p=NULL,*adj=NULL,*n=NULL;int rc=-1,i,count=v75_card_count(cluster);const char *url=v75_card_url(cluster,pos);
 if(!a||!a->dictionary||!a->string||!a->object||!a->integer||!a->array||!a->number||!a->append||!a->release||!url||view<0||view>=count||op<1||op>3)return NULL;
 d=dictionary(a);p=dictionary(a);if(!d||!p)goto done;
 rc=text(a,d,"type",op==1?"showUI":op==2?"updateViewArea":"forceKeyFrame");
 if(!rc)rc=text(a,p,"uuid","b2c3d4e5-0000-4000-8000-00000000c1a5");
 if(!rc&&op==1)rc=text(a,p,"url",url);
 if(!rc&&op==2){
  adj=a->array(NULL,0,a->array_values);if(!adj){rc=-1;goto done;}
  rc=integer(a,p,"animationDurationMillis",0);
  if(!rc)rc=integer(a,p,"viewAreaIndex",view);
  for(i=0;!rc&&i<count;i++)if(i!=view){
   n=a->number(i);if(!n){rc=-1;break;}rc=a->append(adj,n);a->release(n);n=NULL;
  }
  if(!rc)rc=object(a,p,"adjacentViewAreas",adj);
 }
 if(!rc)rc=object(a,d,"params",p);
done:
 if(n)a->release(n);if(adj)a->release(adj);if(p)a->release(p);
 if(rc&&d){a->release(d);d=NULL;}return d;
}
