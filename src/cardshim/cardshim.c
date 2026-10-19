/*
 * cardshim: LD_PRELOAD module loaded BEFORE the CarPlay Maps VC core.so.
 *
 * Intercepts AirPlayCopyServerInfo, calls the next one in the chain (core.so,
 * which already adds the cluster display, type 111) and completes the cluster
 * display announcement with the fields iOS uses to treat it as an instrument
 * cluster:
 *   properties = ["Instrument Cluster"]   (CarKit supportsInstrumentClusterContent)
 *   uid        = same value as uuid       (key read by CARDisplayInfo)
 *
 * Kill switch: if /mnt/app/carplaymaps/NO_CARD_SHIM exists, it only passes through.
 * Short log in /dev/shmem/cardshim.log (one line per session).
 *
 * No system headers: minimal prototypes, resolved at run time.
 */

typedef unsigned int size_t;
typedef long long int64_t;

#define RTLD_NEXT ((void *)-3)
#define O_WRONLY 0x0001
#define O_APPEND 0x0008
#define O_CREAT 0x0100
#define F_OK 0
#define kCFStringEncodingUTF8 0x08000100u

extern void *dlsym(void *handle, const char *name);
extern int open(const char *path, int flags, ...);
extern int write(int fd, const void *buf, size_t n);
extern int read(int fd, void *buf, size_t n);
extern int close(int fd);
extern int access(const char *path, int mode);

typedef void *(*fn_server_info)(void *, void *, void *, void *);
typedef void *(*fn_str_create)(void *, const char *, unsigned int);
typedef void *(*fn_arr_create)(void *, int, const void *);
typedef void (*fn_arr_append)(void *, const void *);
typedef int (*fn_arr_count)(const void *);
typedef const void *(*fn_arr_get)(const void *, int);
typedef const void *(*fn_dict_get)(const void *, const void *);
typedef void (*fn_dict_set)(void *, const void *, const void *);
typedef int64_t (*fn_dict_get_i64)(const void *, const void *, int *);
typedef void (*fn_release)(const void *);

static unsigned int slen(const char *s)
{
    unsigned int n = 0;
    while (s[n]) n++;
    return n;
}

static void log_line(const char *a, const char *b)
{
    int fd = open("/dev/shmem/cardshim.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) return;
    write(fd, a, slen(a));
    if (b) write(fd, b, slen(b));
    write(fd, "\n", 1);
    close(fd);
}

static void log_num(const char *a, int v)
{
    char buf[16];
    int i = 15, neg = v < 0;
    unsigned int u = neg ? (unsigned int)(-v) : (unsigned int)v;
    buf[i] = 0;
    do {
        buf[--i] = (char)('0' + (u % 10));
        u /= 10;
    } while (u && i > 1);
    if (neg) buf[--i] = '-';
    log_line(a, &buf[i]);
}

/*
 * Reads the first line of /mnt/app/carplaymaps/CLUSTER_URL (if present) to
 * override the cluster display's initialURL. Lets URLs be tested over SSH
 * without recompiling; applies on the next CarPlay connection.
 */
static int read_cluster_url(char *buf, int cap)
{
    int fd = open("/mnt/app/carplaymaps/CLUSTER_URL", 0);
    int n, i;
    if (fd < 0) return 0;
    n = read(fd, buf, cap - 1);
    close(fd);
    if (n <= 0) return 0;
    for (i = 0; i < n; i++) {
        if (buf[i] == '\n' || buf[i] == '\r' || buf[i] == ' ') break;
    }
    buf[i] = 0;
    return i;
}

/*
 * Reads /mnt/app/carplaymaps/CLUSTER_ZOOM (e.g. "1.5") into the cluster
 * display's zoomFactor. Accepts only digits and one decimal point.
 * In practice iOS keeps the cluster at @2x; the value only changes the turn
 * card style (below 1 black, 1.5 translucent "liquid glass").
 */
static int read_cluster_zoom(double *out, char *txt, int cap)
{
    int fd = open("/mnt/app/carplaymaps/CLUSTER_ZOOM", 0);
    int n, i;
    long num = 0, den = 1;
    int seen_dot = 0, digits = 0;
    if (fd < 0) return 0;
    n = read(fd, txt, cap - 1);
    close(fd);
    if (n <= 0) return 0;
    for (i = 0; i < n; i++) {
        char c = txt[i];
        if (c >= '0' && c <= '9') {
            if (digits >= 6) break;
            num = num * 10 + (c - '0');
            if (seen_dot) den *= 10;
            digits++;
        } else if (c == '.' && !seen_dot) {
            seen_dot = 1;
        } else {
            break;
        }
    }
    txt[i] = 0;
    if (digits == 0 || num == 0) return 0;
    *out = (double)num / (double)den;
    return 1;
}

typedef int (*fn_dict_set_double)(void *, const void *, double);

void *AirPlayCopyServerInfo(void *a0, void *a1, void *a2, void *a3)
{
    fn_server_info next = (fn_server_info)dlsym(RTLD_NEXT, "AirPlayCopyServerInfo");
    void *info;

    if (!next) {
        log_line("cardshim: next AirPlayCopyServerInfo nao encontrado", 0);
        return 0;
    }
    info = next(a0, a1, a2, a3);
    if (!info) return info;
    if (access("/mnt/app/carplaymaps/NO_CARD_SHIM", F_OK) == 0) {
        log_line("cardshim: desligado por NO_CARD_SHIM", 0);
        return info;
    }

    {
        fn_str_create str_create = (fn_str_create)dlsym(RTLD_NEXT, "CFStringCreateWithCString");
        fn_arr_create arr_create = (fn_arr_create)dlsym(RTLD_NEXT, "CFArrayCreateMutable");
        fn_arr_append arr_append = (fn_arr_append)dlsym(RTLD_NEXT, "CFArrayAppendValue");
        fn_arr_count arr_count = (fn_arr_count)dlsym(RTLD_NEXT, "CFArrayGetCount");
        fn_arr_get arr_get = (fn_arr_get)dlsym(RTLD_NEXT, "CFArrayGetValueAtIndex");
        fn_dict_get dict_get = (fn_dict_get)dlsym(RTLD_NEXT, "CFDictionaryGetValue");
        fn_dict_set dict_set = (fn_dict_set)dlsym(RTLD_NEXT, "CFDictionarySetValue");
        fn_dict_get_i64 dict_get_i64 = (fn_dict_get_i64)dlsym(RTLD_NEXT, "CFDictionaryGetInt64");
        fn_release release = (fn_release)dlsym(RTLD_NEXT, "CFRelease");
        const void *arr_cb = dlsym(RTLD_NEXT, "kCFLArrayCallBacksCFLTypes");
        void *k_displays, *k_type, *k_uuid, *k_uid, *k_props, *v_ic;
        const void *displays;
        int n, i, patched = 0;

        if (!str_create || !arr_create || !arr_append || !arr_count || !arr_get || !dict_get ||
            !dict_set || !dict_get_i64 || !release || !arr_cb) {
            log_line("cardshim: funcao CF ausente, nada alterado", 0);
            return info;
        }

        k_displays = str_create(0, "displays", kCFStringEncodingUTF8);
        k_type = str_create(0, "type", kCFStringEncodingUTF8);
        k_uuid = str_create(0, "uuid", kCFStringEncodingUTF8);
        k_uid = str_create(0, "uid", kCFStringEncodingUTF8);
        k_props = str_create(0, "properties", kCFStringEncodingUTF8);
        v_ic = str_create(0, "Instrument Cluster", kCFStringEncodingUTF8);
        if (!k_displays || !k_type || !k_uuid || !k_uid || !k_props || !v_ic) {
            log_line("cardshim: falha criando strings", 0);
            goto out;
        }

        displays = dict_get(info, k_displays);
        if (!displays) {
            log_line("cardshim: info sem displays", 0);
            goto out;
        }
        n = arr_count(displays);
        log_num("cardshim: displays=", n);
        for (i = 0; i < n; i++) {
            void *disp = (void *)arr_get(displays, i);
            int err = -1;
            int64_t type;
            const void *uuid;
            void *props;
            if (!disp) continue;
            type = dict_get_i64(disp, k_type, &err);
            if (err != 0 || type != 111) continue;
            props = arr_create(0, 1, arr_cb);
            if (props) {
                arr_append(props, v_ic);
                dict_set(disp, k_props, props);
                release(props);
            }
            uuid = dict_get(disp, k_uuid);
            if (uuid && !dict_get(disp, k_uid)) dict_set(disp, k_uid, uuid);
            {
                char url[160];
                if (read_cluster_url(url, (int)sizeof(url)) > 0) {
                    void *k_url = str_create(0, "initialURL", kCFStringEncodingUTF8);
                    void *v_url = str_create(0, url, kCFStringEncodingUTF8);
                    if (k_url && v_url) {
                        dict_set(disp, k_url, v_url);
                        log_line("cardshim: initialURL=", url);
                    }
                    if (k_url) release(k_url);
                    if (v_url) release(v_url);
                }
            }
            {
                double zoom;
                char ztxt[24];
                if (read_cluster_zoom(&zoom, ztxt, (int)sizeof(ztxt))) {
                    fn_dict_set_double set_double = (fn_dict_set_double)dlsym(RTLD_NEXT, "CFDictionarySetDouble");
                    void *k_zoom = str_create(0, "zoomFactor", kCFStringEncodingUTF8);
                    if (set_double && k_zoom) {
                        set_double(disp, k_zoom, zoom);
                        log_line("cardshim: zoomFactor=", ztxt);
                    } else {
                        log_line("cardshim: CFDictionarySetDouble ausente, zoom ignorado", 0);
                    }
                    if (k_zoom) release(k_zoom);
                }
            }
            patched++;
        }
        log_num("cardshim: displays type 111 completados=", patched);
out:
        if (k_displays) release(k_displays);
        if (k_type) release(k_type);
        if (k_uuid) release(k_uuid);
        if (k_uid) release(k_uid);
        if (k_props) release(k_props);
        if (v_ic) release(v_ic);
    }
    return info;
}
