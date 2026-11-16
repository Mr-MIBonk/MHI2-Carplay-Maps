/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
#ifndef V75_CARD_PROTOCOL_H
#define V75_CARD_PROTOCOL_H
#include <stdint.h>
struct v75_card_cf {
 void *(*dictionary)(void *,int,const void *,const void *);
 void *(*string)(void *,const char *,unsigned);
 int (*object)(void *,const void *,const void *);
 int (*integer)(void *,const void *,int64_t);
 void *(*array)(void *,int,const void *);
 void *(*number)(int64_t);
 int (*append)(void *,const void *);
 void (*release)(const void *);
 const void *dictionary_keys,*dictionary_values,*array_values;
};
int v75_card_count(int cluster);
int v75_card_view(int cluster,int position,int navigating);
const char *v75_card_url(int cluster,int position);
void *v75_card_request(const struct v75_card_cf *,int cluster,int operation,int position,int view);
#endif
