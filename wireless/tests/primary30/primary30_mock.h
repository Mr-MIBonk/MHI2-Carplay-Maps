#ifndef V74_MOCK_CF_H
#define V74_MOCK_CF_H
#include "primary30.h"
typedef struct Obj Obj;
Obj *mdict(void);
Obj *marray(void);
Obj *mnum(int64_t);
Obj *mstr(const char *);
void mput(Obj *, const char *, Obj *); /* consumes the value's +1 */
void mappend(Obj *, Obj *); /* consumes the value's +1 */
Obj *mget(const void *, const char *);
int64_t mvalue(const void *, const char *);
void mfreeze(Obj *);
void mrelease(const void *);
void mfail_after(int);
int mlive(void);
int mrefs(const void *);
V74CF mapi(void);
extern Obj *mock_original;
extern int mock_calls;
extern int32_t mock_error;
extern void *mock_server;
extern const void *mock_device, *mock_interface;
#endif
