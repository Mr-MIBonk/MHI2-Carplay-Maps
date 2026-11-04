#define _GNU_SOURCE

#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define DRIVER_NAME "ipod-drvr-iap2"
#define LOG_PATH "/ramdisk/cpmap-info/runtime/v532-rgd-native.log"
#define START_MARKER "/ramdisk/cpmap-info/runtime/v49-rgd-start"
#define SEND_ENABLE_MARKER "/ramdisk/cpmap-info/runtime/v49-rgd-send-enable"
#define RESYNC_MARKER "/ramdisk/cpmap-info/runtime/v513-rgd-resync-request"
#define RESUME_MARKER "/ramdisk/cpmap-info/runtime/v532-rgd-resume-request"
#define LOG_LIMIT_BYTES (64 * 1024)

typedef int (*ident_callback_t)(void *, void *);
typedef void *(*dlopen_fn_t)(const char *, int);
/*
 * MU0841 ABI:
 *   packet_sendlist_pop(iap2_context *)
 *   packet_sendlist_push(iap2_context *, packet *)
 *
 * MST2's helper called pop without an argument.  Doing that on MU0841 makes
 * packet_sendlist_pop dereference a garbage r0 value and terminates mm-ipod.
 */
typedef void *(*packet_pop_fn_t)(void *);
typedef int (*packet_push_fn_t)(void *, void *);
typedef int (*link_send_fn_t)(void *, void *, unsigned int);
typedef void *(*packet_addparam_fn_t)(
    void *, void *, unsigned int, const void *, unsigned int);

struct ident_entry {
    uint32_t id;
    ident_callback_t callback;
};

struct message_row {
    const char *name;
    void *decode;
    void *handler;
};

struct message_family {
    void *decode;
    struct message_row *rows;
    uint32_t count;
};

static struct message_row g_family_52_rows[] = {
    { "5200", 0, 0 },
    { "5201", 0, 0 },
    { "5202", 0, 0 },
    { "5203", 0, 0 },
    { "5204", 0, 0 }
};

static ident_callback_t g_original_id17;
static packet_pop_fn_t g_packet_pop;
static packet_push_fn_t g_packet_push;
static link_send_fn_t g_link_send;
static packet_addparam_fn_t g_addparam;
static void *g_context;
static volatile unsigned int g_identify_generation;
static volatile int g_patched;
static volatile int g_sender_started;
static volatile int g_dlopen_hook;
static dlopen_fn_t g_real_dlopen;

static void log_line(const char *line)
{
    struct stat st;
    const char *mode = "a";
    FILE *fp;
    if (stat(LOG_PATH, &st) == 0 && st.st_size >= LOG_LIMIT_BYTES) {
        mode = "w";
    }
    fp = fopen(LOG_PATH, mode);
    if (fp == 0) {
        return;
    }
    fputs(line, fp);
    fputc('\n', fp);
    fclose(fp);
}

static int marker_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static void write_marker(const char *path)
{
    FILE *fp = fopen(path, "w");
    if (fp == 0) {
        return;
    }
    fputc('1', fp);
    fclose(fp);
}

static unsigned char *put_tlv(
    unsigned char *dst,
    unsigned int type,
    const void *value,
    unsigned int value_len)
{
    unsigned int total = value_len + 4U;
    dst[0] = (unsigned char)((total >> 8) & 0xffU);
    dst[1] = (unsigned char)(total & 0xffU);
    dst[2] = (unsigned char)((type >> 8) & 0xffU);
    dst[3] = (unsigned char)(type & 0xffU);
    if (value_len != 0U) {
        memcpy(dst + 4, value, value_len);
    }
    return dst + total;
}

static unsigned char *put_tlv_u16(
    unsigned char *dst,
    unsigned int type,
    unsigned int value)
{
    unsigned char encoded[2];
    encoded[0] = (unsigned char)((value >> 8) & 0xffU);
    encoded[1] = (unsigned char)(value & 0xffU);
    return put_tlv(dst, type, encoded, sizeof(encoded));
}

static int safe_ident_id17(void *context, void *packet)
{
    static const char component_name[] = "Virtual Cockpit";
    unsigned char component[192];
    unsigned char *end = component;
    int original_rc = 0;

    /*
     * MU0841 entry 17 is ident_info_tspbt.  It is required by the stock
     * CarPlay authentication/transport path, so it must never be discarded.
     */
    if (g_original_id17 != 0) {
        original_rc = g_original_id17(context, packet);
    }

    g_context = context;
    ++g_identify_generation;

    if (g_addparam == 0) {
        log_line("IDENT: stock id17 retained; component 30 skipped (no addparam)");
        return original_rc;
    }

    end = put_tlv_u16(end, 0, 1);
    end = put_tlv(end, 1, component_name, sizeof(component_name));
    /* Match the native MIB2Q/MHI3 RouteGuidanceDisplayComponent exactly. */
    end = put_tlv_u16(end, 2, 0x0100);
    end = put_tlv_u16(end, 3, 0x0100);
    end = put_tlv_u16(end, 4, 0x0100);
    end = put_tlv_u16(end, 5, 0x0100);
    end = put_tlv_u16(end, 6, 0x0006);
    end = put_tlv_u16(end, 7, 0x0100);
    end = put_tlv_u16(end, 8, 0x0006);

    g_addparam(
        context,
        packet,
        0x1e,
        component,
        (unsigned int)(end - component));
    log_line("IDENT: stock id17 retained + component 30 appended");
    return original_rc;
}

static int send_route_guidance_start(void)
{
    unsigned char component_id[2] = { 0, 1 };
    unsigned char empty = 0;
    void *packet;
    int rc;

    if (g_context == 0 || g_packet_pop == 0 ||
        g_packet_push == 0 || g_link_send == 0) {
        return -1;
    }

    packet = g_packet_pop(g_context);
    if (packet == 0) {
        log_line("SEND: packet_sendlist_pop returned null");
        return -2;
    }

    /*
     * Native 0x5200 subscription shape used by MIB2Q/MHI3:
     *   id 0 = component id (u16)
     *   id 1 = request sourceName (presence TLV)
     *   id 2 = request sourceSupportsRouteGuidance (presence TLV)
     *   id 3 = request supportsExitInfo (presence TLV)
     * Empty/partial V52 subscriptions were accepted for basic RGI but iOS
     * did not consistently republish 0x5204 after reconnects.
     */
    g_addparam(g_context, packet, 0, component_id, sizeof(component_id));
    g_addparam(g_context, packet, 1, &empty, 0);
    g_addparam(g_context, packet, 2, &empty, 0);
    g_addparam(g_context, packet, 3, &empty, 0);

    rc = g_link_send(g_context, packet, 0x5200);
    g_packet_push(g_context, packet);
    log_line("SEND: 0x5200 native full subscription (component+source+RG+exit)");
    return rc;
}

static void *sender_thread(void *unused)
{
    unsigned int handled_generation = 0;
    unsigned int resynced_generation = 0;
    (void)unused;
    log_line("THREAD: sender started");

    for (;;) {
        unsigned int generation = g_identify_generation;

        /*
         * Java writes this marker only on a real disconnected -> connected
         * edge.  It asks the phone to republish its current Route Guidance
         * state without restarting cpvc-rgd-tap (which would replay the old
         * iap2.log from offset zero).
         */
        if (marker_exists(RESYNC_MARKER) &&
            marker_exists(START_MARKER) &&
            marker_exists(SEND_ENABLE_MARKER) &&
            generation != 0 && g_context != 0) {
            /*
             * A watchdog or log-reopen race must never create an endless
             * 0x5200 loop.  One identify generation represents one live
             * CarPlay link; allow at most one explicit resync volley within
             * that generation.  A genuine reconnect increments generation
             * and is therefore allowed immediately.
             */
            if (generation == resynced_generation) {
                unlink(RESYNC_MARKER);
                log_line("RESYNC: duplicate request suppressed in current generation");
                sleep(1);
                continue;
            }
            resynced_generation = generation;
            /* Release the cache suppression before fresh 0x5201 replies arrive. */
            write_marker(RESUME_MARKER);
            unlink(RESYNC_MARKER);
            log_line("RESYNC: reconnect marker consumed");
            send_route_guidance_start();
            sleep(1);
            if (g_identify_generation == generation) {
                send_route_guidance_start();
                sleep(1);
            }
            if (g_identify_generation == generation) {
                send_route_guidance_start();
            }
            handled_generation = generation;
            log_line("RESYNC: reconnect 0x5200 volley complete");
            continue;
        }

        if (!marker_exists(START_MARKER) ||
            !marker_exists(SEND_ENABLE_MARKER) ||
            generation == 0 || generation == handled_generation) {
            sleep(1);
            continue;
        }

        handled_generation = generation;
        sleep(2);
        if (g_identify_generation != generation) {
            continue;
        }

        send_route_guidance_start();
        sleep(3);
        if (g_identify_generation != generation) {
            continue;
        }
        send_route_guidance_start();
        sleep(3);
        if (g_identify_generation != generation) {
            continue;
        }
        send_route_guidance_start();
    }
    return 0;
}

static void resolve_engine_symbols(void *handle)
{
    if (handle == 0) {
        handle = RTLD_DEFAULT;
    }
    g_packet_pop = (packet_pop_fn_t)dlsym(handle, "packet_sendlist_pop");
    g_packet_push = (packet_push_fn_t)dlsym(handle, "packet_sendlist_push");
    g_link_send = (link_send_fn_t)dlsym(handle, "link_send_message");
    g_addparam = (packet_addparam_fn_t)dlsym(handle, "packet_addparam_blob");
}

static void patch_driver(void *handle)
{
    struct ident_entry *ident;
    struct message_family *families;
    pthread_t thread;
    unsigned int i;

    if (g_patched) {
        return;
    }

    resolve_engine_symbols(handle);
    ident = (struct ident_entry *)dlsym(handle, "ident_info_funcs");
    families = (struct message_family *)dlsym(handle, "iap2_ctrl_msg_table");
    if (ident == 0 || g_addparam == 0 || g_packet_pop == 0 ||
        g_packet_push == 0 || g_link_send == 0) {
        log_line("PATCH: required MHI2 symbols not ready");
        return;
    }

    for (i = 0; i < 24U; ++i) {
        if (ident[i].id == 17U) {
            if (!ident[i].callback || ident[i].callback == safe_ident_id17) {
                log_line("PATCH: id17 callback absent or already hooked; no write performed");
                return;
            }
            g_original_id17 = ident[i].callback;
            ident[i].callback = safe_ident_id17;
            break;
        }
    }
    if (i == 24U || g_original_id17 == 0) {
        log_line("PATCH: id17 entry not found; no write performed");
        return;
    }

    /*
     * MU0841 exports the 256-entry family table.  Resolve it symbolically and
     * fill only family 0x52; never use the MST2 fixed module offset.
     */
    if (families != 0) {
        if (families[0x52].rows == 0 && families[0x52].count == 0U) {
            families[0x52].decode = 0;
            families[0x52].rows = g_family_52_rows;
            families[0x52].count = 5;
            log_line("PATCH: family 0x52 installed through exported table");
        } else {
            log_line("PATCH: family 0x52 already populated; retained");
        }
    } else {
        log_line("PATCH: iap2_ctrl_msg_table unavailable");
    }

    g_patched = 1;
    log_line("PATCH: MU0841 safe id17 chain active");

    if (!g_sender_started) {
        g_sender_started = 1;
        if (pthread_create(&thread, 0, sender_thread, 0) == 0) {
            pthread_detach(thread);
        } else {
            g_sender_started = 0;
            log_line("THREAD: create failed");
        }
    }
}

void *dlopen(const char *filename, int flags)
{
    void *handle;

    if (g_real_dlopen == 0) {
        g_real_dlopen = (dlopen_fn_t)dlsym(RTLD_NEXT, "dlopen");
    }
    if (g_real_dlopen == 0) {
        return 0;
    }

    if (g_dlopen_hook) {
        return g_real_dlopen(filename, flags);
    }

    g_dlopen_hook = 1;
    handle = g_real_dlopen(filename, flags);
    if (handle != 0 && filename != 0 &&
        strstr(filename, DRIVER_NAME) != 0) {
        patch_driver(handle);
    }
    g_dlopen_hook = 0;
    return handle;
}

__attribute__((constructor))
static void cpvc_rgd_mhi2_init(void)
{
    unlink(LOG_PATH);
    log_line("INIT: V53.2 MHI2-safe RGI hook loaded");
    patch_driver(RTLD_DEFAULT);
}
