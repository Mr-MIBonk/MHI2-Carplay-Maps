#include "primary30.h"
#include <stddef.h>

typedef struct CopyContext { const V74CF *cf; void *copy; } CopyContext;
static void copy_entry(const void *key, const void *value, void *context)
{
    CopyContext *c = (CopyContext *)context;
    c->cf->set(c->copy, key, value);
}
static void *copy_dictionary(const V74CF *c, const void *original)
{
    CopyContext ctx;
    long count = c->dict_count(original);
    if (count < 0 || count > 256) return NULL;
    ctx.cf = c;
    ctx.copy = c->dict_new(NULL, 0, c->dict_keys, c->dict_values);
    if (!ctx.copy) return NULL;
    c->dict_apply(original, copy_entry, &ctx);
    if (c->dict_count(ctx.copy) != count) {
        c->release(ctx.copy); return NULL;
    }
    return ctx.copy;
}
static int ready(const V74CF *c)
{
    return c && c->type && c->dict_type && c->array_type && c->string &&
        c->get && c->dict_count && c->dict_apply && c->dict_new && c->set &&
        c->set_int && c->get_int && c->array_count && c->item &&
        c->array_copy && c->array_set && c->release && c->dict_keys && c->dict_values;
}
static int integer(const V74CF *c, const void *dict, const void *key, int64_t *n)
{
    int32_t error = -1;
    if (!c->get(dict, key)) return 0;
    *n = c->get_int(dict, key, &error);
    return error == 0;
}

void *v74_primary30_copy(const V74CF *c, const void *info, int enabled,
                         int width, int height, int *result)
{
    const void *array, *selected = NULL;
    void *keys[5] = {NULL,NULL,NULL,NULL,NULL};
    const char *names[5] = {"displays","widthPixels","heightPixels","maxFPS","type"};
    void *new_info = NULL, *new_array = NULL, *new_display = NULL;
    long i, count, selected_index = -1;
    int rc = V74_PRIMARY_UNCHANGED, auto_main = width == 0 && height == 0;
    int64_t fps = 0;
    if (result) *result = rc;
    if (!enabled) return NULL;
    /* Main-screen geometry is an installer-verified input, NOT a cluster
     * resolution. Unknown geometry/configuration never selects by index. */
    if (!ready(c) || !info || (!auto_main && (width < 320 || width > 4096 || height < 200 ||
        height > 2160)) || c->type(info) != c->dict_type()) {
        if (result) *result = V74_PRIMARY_INVALID;
        return NULL;
    }
    for (i=0; i<5; ++i) {
        keys[i] = c->string(NULL, names[i], 0x08000100U);
        if (!keys[i]) { rc = V74_PRIMARY_NOMEM; goto done; }
    }
    array = c->get(info, keys[0]);
    if (!array || c->type(array) != c->array_type()) goto done;
    count = c->array_count(array);
    if (count < 1 || count > 16) goto done;
    for (i=0; i<count; ++i) {
        const void *d = c->item(array, i), *type;
        int64_t w=0, h=0, t=0;
        if (!d || c->type(d) != c->dict_type()) goto done;
        type = c->get(d, keys[4]);
        if (type && (!integer(c,d,keys[4],&t) || t != 110)) continue;
        if (auto_main && (!type || t != 110)) continue;
        if (integer(c,d,keys[1],&w) && integer(c,d,keys[2],&h) &&
            ((auto_main && w >= 320 && w <= 4096 && h >= 200 && h <= 2160) ||
             (w == width && h == height))) {
            /* A duplicate match is ambiguous. Never choose the first one. */
            if (selected) goto done;
            selected = d; selected_index = i;
        }
    }
    if (!selected || !integer(c,selected,keys[3],&fps)) goto done;
    if (fps == 30) { rc = V74_PRIMARY_ALREADY30; goto done; }
    if (fps < 30 || fps > 120) goto done; /* Never raise an unknown lower cap. */
    new_display = copy_dictionary(c, selected);
    new_array = c->array_copy(NULL, 0, array);
    new_info = copy_dictionary(c, info);
    if (!new_display || !new_array || !new_info) { rc = V74_PRIMARY_NOMEM; goto done; }
    if (c->set_int(new_display,keys[3],30) != 0 ||
        !integer(c,new_display,keys[3],&fps) || fps != 30) goto done;
    c->array_set(new_array, selected_index, new_display);
    if (c->item(new_array,selected_index) != new_display) goto done;
    c->set(new_info,keys[0],new_array);
    if (c->get(new_info,keys[0]) != new_array) goto done;
    rc = V74_PRIMARY_CHANGED;
done:
    for (i=0; i<5; ++i) if (keys[i]) c->release(keys[i]);
    if (new_array) c->release(new_array);
    if (new_display) c->release(new_display);
    if (result) *result = rc;
    if (rc == V74_PRIMARY_CHANGED) return new_info;
    if (new_info) c->release(new_info);
    return NULL;
}
