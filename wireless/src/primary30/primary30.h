#ifndef V74_PRIMARY60_H
#define V74_PRIMARY60_H
#include <stdint.h>

/* Explicit CFL ABI, resolved from the exact stock libairplay.so. No SDK CF
 * structs are imported, and no display object supplied by Apple is mutated. */
typedef struct V74CF {
    unsigned long (*type)(const void *);
    unsigned long (*dict_type)(void);
    unsigned long (*array_type)(void);
    void *(*string)(void *, const char *, uint32_t);
    const void *(*get)(const void *, const void *);
    long (*dict_count)(const void *);
    void (*dict_apply)(const void *, void (*)(const void *, const void *, void *), void *);
    void *(*dict_new)(void *, long, const void *, const void *);
    void (*set)(void *, const void *, const void *);
    int32_t (*set_int)(void *, const void *, int64_t);
    int64_t (*get_int)(const void *, const void *, int32_t *);
    long (*array_count)(const void *);
    const void *(*item)(const void *, long);
    void *(*array_copy)(void *, long, const void *);
    void (*array_set)(void *, long, const void *);
    void (*release)(const void *);
    const void *dict_keys, *dict_values;
} V74CF;

enum V74PrimaryResult {
    V74_PRIMARY_UNCHANGED = 0,
    V74_PRIMARY_CHANGED = 1,
    V74_PRIMARY_ALREADY30 = 2,
    V74_PRIMARY_INVALID = -1,
    V74_PRIMARY_NOMEM = -2
};

/* Returns a new +1 owned object ONLY on CHANGED. Otherwise returns NULL,
 * leaving the original +1 result and all its children untouched. */
void *v74_primary30_copy(const V74CF *, const void *, int enabled,
                         int width, int height, int *result);
#endif
