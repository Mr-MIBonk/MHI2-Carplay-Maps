#include "primary30_mock.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

enum { DICT=1, ARRAY=2, NUMBER=3, STRING=4 };
struct Obj {
    unsigned long type;
    int refs, frozen;
    long count;
    int64_t number;
    char *string;
    struct Obj *keys[256], *values[256];
};
static int alive=0, budget=-1;
static Obj *make(unsigned long type)
{
    Obj *o;
    if (budget==0) return NULL;
    if (budget>0) --budget;
    o=(Obj *)calloc(1,sizeof(*o)); assert(o);
    o->type=type; o->refs=1; ++alive; return o;
}
void mfail_after(int n) { budget=n; }
int mlive(void) { return alive; }
int mrefs(const void *p) { return ((const Obj *)p)->refs; }
static void retain(const void *p) { assert(p); ++((Obj *)p)->refs; }
void mrelease(const void *p)
{
    Obj *o=(Obj *)p; long i;
    if (!o) return;
    assert(o->refs>0);
    if (--o->refs) return;
    for (i=0;i<o->count;++i) { if(o->keys[i]) mrelease(o->keys[i]); mrelease(o->values[i]); }
    free(o->string); free(o); --alive;
}
Obj *mdict(void) { return make(DICT); }
Obj *marray(void) { return make(ARRAY); }
Obj *mnum(int64_t n) { Obj *o=make(NUMBER); if(o) o->number=n; return o; }
Obj *mstr(const char *s) { Obj *o=make(STRING); if(o){o->string=strdup(s);assert(o->string);} return o; }
static int equal(const Obj *a,const Obj *b)
{ return a==b || (a && b && a->type==STRING && b->type==STRING && !strcmp(a->string,b->string)); }
unsigned long CFGetTypeID(const void *p) { assert(p); return ((const Obj *)p)->type; }
unsigned long CFDictionaryGetTypeID(void) { return DICT; }
unsigned long CFArrayGetTypeID(void) { return ARRAY; }
void *CFStringCreateWithCString(void *allocator,const char *s,uint32_t encoding)
{ (void)allocator; assert(encoding==0x08000100U); return mstr(s); }
const void *CFDictionaryGetValue(const void *p,const void *key)
{
    const Obj *o=(const Obj *)p; long i; assert(o->type==DICT);
    for(i=0;i<o->count;++i) if(equal(o->keys[i],key)) return o->values[i];
    return NULL;
}
long CFDictionaryGetCount(const void *p) { const Obj *o=p; assert(o->type==DICT);return o->count; }
void CFDictionarySetValue(void *p,const void *key,const void *value)
{
    Obj *o=p; long i; assert(o->type==DICT && !o->frozen && value && key);
    for(i=0;i<o->count;++i) if(equal(o->keys[i],key)) {
        retain(value);mrelease(o->values[i]);o->values[i]=(Obj *)value;return;
    }
    assert(o->count<256); retain(key);retain(value);
    o->keys[o->count]=(Obj *)key; o->values[o->count++]=(Obj *)value;
}
int32_t CFDictionarySetInt64(void *p,const void *k,int64_t n)
{ Obj *v=mnum(n);if(!v)return -1;CFDictionarySetValue(p,k,v);mrelease(v);return 0; }
int64_t CFDictionaryGetInt64(const void *p,const void *k,int32_t *e)
{ const Obj *v=CFDictionaryGetValue(p,k);*e=(v && v->type==NUMBER)?0:-1;return *e?0:v->number; }
void CFDictionaryApplyFunction(const void *p,void (*f)(const void *,const void *,void *),void *context)
{ const Obj *o=p;long i;assert(o->type==DICT);for(i=0;i<o->count;++i) f(o->keys[i],o->values[i],context); }
const int kCFLDictionaryKeyCallBacksCFLTypes=1, kCFLDictionaryValueCallBacksCFLTypes=2;
void *CFDictionaryCreateMutable(void *allocator,long cap,const void *keys,const void *values)
{ (void)allocator;(void)cap;assert(keys && values);return mdict(); }
long CFArrayGetCount(const void *p) { const Obj *o=p;assert(o->type==ARRAY);return o->count; }
const void *CFArrayGetValueAtIndex(const void *p,long i)
{ const Obj *o=p;assert(o->type==ARRAY && i>=0 && i<o->count);return o->values[i]; }
void CFArraySetValueAtIndex(void *p,long i,const void *value)
{ Obj *o=p;assert(o->type==ARRAY && !o->frozen && i>=0 && i<o->count);retain(value);mrelease(o->values[i]);o->values[i]=(Obj *)value; }
void *CFArrayCreateMutableCopy(void *allocator,long cap,const void *p)
{
    const Obj *o=p;Obj *n=marray();long i;(void)allocator;(void)cap;
    assert(o->type==ARRAY);if(!n)return NULL;
    for(i=0;i<o->count;++i){retain(o->values[i]);n->values[n->count++]=o->values[i];}return n;
}
void CFRelease(const void *p) { mrelease(p); }
void mput(Obj *d,const char *s,Obj *value)
{ Obj *k=mstr(s);assert(k && value);CFDictionarySetValue(d,k,value);mrelease(k);mrelease(value); }
void mappend(Obj *a,Obj *value)
{ assert(a->type==ARRAY && !a->frozen && a->count<256 && value);a->values[a->count++]=value; }
Obj *mget(const void *p,const char *key)
{ Obj k;memset(&k,0,sizeof(k));k.type=STRING;k.string=(char *)key;return (Obj *)CFDictionaryGetValue(p,&k); }
int64_t mvalue(const void *p,const char *key)
{ const Obj *v=mget(p,key);assert(v && v->type==NUMBER);return v->number; }
void mfreeze(Obj *o)
{ long i;o->frozen=1;for(i=0;i<o->count;++i){if(o->keys[i])mfreeze(o->keys[i]);mfreeze(o->values[i]);} }
V74CF mapi(void)
{
    V74CF c={CFGetTypeID,CFDictionaryGetTypeID,CFArrayGetTypeID,
        CFStringCreateWithCString,CFDictionaryGetValue,CFDictionaryGetCount,
        CFDictionaryApplyFunction,CFDictionaryCreateMutable,CFDictionarySetValue,
        CFDictionarySetInt64,CFDictionaryGetInt64,CFArrayGetCount,
        CFArrayGetValueAtIndex,CFArrayCreateMutableCopy,CFArraySetValueAtIndex,
        CFRelease,&kCFLDictionaryKeyCallBacksCFLTypes,&kCFLDictionaryValueCallBacksCFLTypes};
    return c;
}
Obj *mock_original=NULL;
int mock_calls=0;
int32_t mock_error=0;
void *mock_server=NULL;
const void *mock_device=NULL,*mock_interface=NULL;
void *AirPlayCopyServerInfo(void *s,const void *d,const void *i,int32_t *error)
{
    ++mock_calls;mock_server=s;mock_device=d;mock_interface=i;
    if(error)*error=mock_error;
    if(mock_original)retain(mock_original);
    errno=EAGAIN;return mock_original;
}
