/*
 * cpvc-rgd-sync - MHI2 CarPlay route-guidance cache/association service.
 *
 * V52's original tap flattened asynchronous 0x5201/0x5202/0x5204 packets
 * into one PPS record.  That made the last future maneuver overwrite the
 * current maneuver and incorrectly compared a maneuver index with an iOS
 * composedGuidanceEventIndex.  This service follows /dev/shmem/iap2.log,
 * caches maneuvers and lane events independently, and publishes one atomic
 * snapshot selected by the active 0x5201 maneuver/lane indices.
 *
 * No private MHI API is required.  It intentionally emits the legacy flat
 * PPS keys consumed by Mst2CarPlayNaviSinkBridge.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define IAP2_LOG "/dev/shmem/iap2.log"
#define PPS_DIR "/ramdisk/cpmap-info/runtime"
#define PPS_PATH "/ramdisk/cpmap-info/runtime/v532-rgd-cache.pps"
#define SYNC_LOG "/ramdisk/cpmap-info/runtime/v532-rgd-sync.log"
#define HEARTBEAT_PATH "/ramdisk/cpmap-info/runtime/v532-rgd-heartbeat"
#define LOCK_PATH "/ramdisk/cpmap-info/runtime/v532-rgd-sync.lock"
#define START_MARKER "/ramdisk/cpmap-info/runtime/v532-rgd-sync-started"
#define RX_5201_MARKER "/ramdisk/cpmap-info/runtime/v532-rgd-5201"
#define RX_5202_MARKER "/ramdisk/cpmap-info/runtime/v532-rgd-5202"
#define RX_5204_MARKER "/ramdisk/cpmap-info/runtime/v532-rgd-5204"
#define CLEAR_REQUEST "/ramdisk/cpmap-info/runtime/v532-rgd-clear-request"
#define RESUME_REQUEST "/ramdisk/cpmap-info/runtime/v532-rgd-resume-request"
#define RESYNC_REQUEST "/ramdisk/cpmap-info/runtime/v513-rgd-resync-request"

#define MSG_ROUTE_UPDATE 0x5201
#define MSG_MANEUVER 0x5202
#define MSG_LANE 0x5204

#define CACHE_SIZE 64
#define MAX_ACTIVE_LIST 32
#define MAX_LANES 8
#define MAX_ANGLES 16
#define LINE_SIZE 32768
#define PACKET_SIZE 8192
#define SNAPSHOT_SIZE 8192
#define LOG_LIMIT_BYTES (64 * 1024)
#define HEARTBEAT_INTERVAL_MS 5000ULL
#define ROUTE_ZERO_NORMAL_GRACE_MS 800ULL
#define RGD_STATE_REROUTING 5

/* 0x5201 fields. */
#define U_STATE 0x0001
#define U_MAN_STATE 0x0002
#define U_ROAD 0x0003
#define U_DEST 0x0004
#define U_ETA 0x0005
#define U_RTT 0x0006
#define U_DIST_DEST 0x0007
#define U_DIST_STR 0x0008
#define U_DIST_UNITS 0x0009
#define U_DIST_MAN 0x000A
#define U_DIST_MAN_STR 0x000B
#define U_DIST_MAN_UNITS 0x000C
#define U_MAN_LIST 0x000D
#define U_MAN_COUNT 0x000E
#define U_VISIBLE_IN_APP 0x000F
#define U_LANE_INDEX 0x0010
#define U_LANE_TOTAL 0x0011
#define U_LANE_SHOWING 0x0012
#define U_SOURCE_SUPPORTS_RG 0x0014

/* 0x5202 fields. */
#define M_INDEX 0x0001
#define M_DESC 0x0002
#define M_TYPE 0x0003
#define M_AFTER_ROAD 0x0004
#define M_DIST_BETWEEN 0x0005
#define M_DIST_STR 0x0006
#define M_DIST_UNITS 0x0007
#define M_DRIVING_SIDE 0x0008
#define M_JUNCTION_TYPE 0x0009
#define M_EXIT_ANGLE 0x000B
#define M_LINKED_LANE 0x000C

/* 0x5204 fields and nested lane-information fields. */
#define L_EVENT_INDEX 0x0001
#define L_INFORMATION 0x0002
#define LI_INDEX 0x0000
#define LI_STATUS 0x0001
#define LI_ANGLES 0x0002
#define LI_HIGHLIGHT 0x0003

enum {
    UP_STATE = 1u << 0,
    UP_MAN_STATE = 1u << 1,
    UP_ROAD = 1u << 2,
    UP_DEST = 1u << 3,
    UP_ETA = 1u << 4,
    UP_RTT = 1u << 5,
    UP_DIST_DEST = 1u << 6,
    UP_DIST_MAN = 1u << 7,
    UP_DIST_MAN_STR = 1u << 8,
    UP_DIST_MAN_UNITS = 1u << 9,
    UP_MAN_LIST = 1u << 10,
    UP_MAN_COUNT = 1u << 11,
    UP_LANE_INDEX = 1u << 12,
    UP_LANE_TOTAL = 1u << 13,
    UP_LANE_SHOWING = 1u << 14,
    UP_VISIBLE_IN_APP = 1u << 15,
    UP_SOURCE_SUPPORTS_RG = 1u << 16
};

typedef struct {
    uint16_t position;
    uint8_t status;
    int16_t angles[MAX_ANGLES];
    uint8_t angle_count;
    int16_t highlight;
    uint8_t has_highlight;
} lane_item_t;

typedef struct {
    uint8_t valid;
    uint16_t event_index;
    lane_item_t lane[MAX_LANES];
    uint8_t lane_count;
    uint32_t generation;
} lane_event_t;

typedef struct {
    uint8_t valid;
    uint16_t index;
    char description[256];
    char after_road[256];
    uint8_t maneuver_type;
    uint8_t has_type;
    uint32_t distance_between;
    char distance_string[64];
    uint8_t distance_units;
    uint8_t driving_side;
    uint8_t junction_type;
    int16_t exit_angle;
    uint8_t has_exit_angle;
    uint16_t linked_lane_index;
    uint8_t has_linked_lane;
    uint32_t generation;
} maneuver_t;

typedef struct {
    uint32_t present;
    uint8_t route_state;
    uint8_t maneuver_state;
    char road[256];
    char destination[256];
    uint64_t eta;
    uint64_t time_remaining;
    uint32_t distance_remaining;
    uint32_t distance_to_maneuver;
    char distance_to_maneuver_string[64];
    uint8_t distance_to_maneuver_units;
    uint16_t maneuver_list[MAX_ACTIVE_LIST];
    uint8_t maneuver_list_count;
    uint16_t maneuver_count;
    uint16_t lane_index;
    uint16_t lane_total;
    uint8_t lane_showing;
    uint8_t visible_in_app;
    uint8_t source_supports_rg;
} update_t;

typedef struct {
    int route_state;
    int maneuver_state;
    char road[256];
    char destination[256];
    uint64_t eta;
    uint64_t time_remaining;
    int64_t distance_remaining;
    int64_t distance_to_maneuver;
    char distance_to_maneuver_string[64];
    int distance_to_maneuver_units;
    uint16_t active_maneuver_index;
    uint8_t has_active_maneuver;
    int maneuver_count;
    uint16_t active_lane_index;
    uint8_t has_active_lane;
    int lane_total;
    int lane_showing;
    uint8_t has_lane_showing;
} route_t;

static route_t g_route;
static maneuver_t g_maneuvers[CACHE_SIZE];
static lane_event_t g_lanes[CACHE_SIZE];
static uint32_t g_generation;
static unsigned long g_sequence;
static char g_last_body[SNAPSHOT_SIZE];
static const char *g_output_path = PPS_PATH;
static int g_replay;
static int g_suppress_updates;
static int g_lock_fd = -1;
static unsigned long g_count_5201, g_count_5202, g_count_5204;
static uint64_t g_last_heartbeat_ms;
static uint8_t g_last_raw_route_state;
static uint64_t g_route_zero_started_ms;
static uint64_t g_route_zero_deadline_ms;
static int g_route_zero_from_reroute;

static uint16_t be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16)
        | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint64_t be64(const uint8_t *p) {
    return ((uint64_t)be32(p) << 32) | be32(p + 4);
}

static void touch_file(const char *path) {
    FILE *f = fopen(path, "a");
    if (f) fclose(f);
}

static uint64_t monotonic_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
        return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
    return (uint64_t)time(NULL) * 1000ULL;
}

static void log_line(const char *message) {
    FILE *f;
    struct stat st;
    const char *mode = "a";
    if (g_replay) return;
    if (stat(SYNC_LOG, &st) == 0 && st.st_size >= LOG_LIMIT_BYTES)
        mode = "w";
    f = fopen(SYNC_LOG, mode);
    if (!f) return;
    fprintf(f, "%s\n", message);
    fclose(f);
}

static void copy_text(char *dst, size_t dst_size, const uint8_t *src, size_t n) {
    size_t i, o = 0;
    if (!dst || dst_size == 0) return;
    for (i = 0; i < n && o + 1 < dst_size; ++i) {
        uint8_t c = src[i];
        if (c == 0) break;
        if (c == '\r' || c == '\n') {
            if (o && dst[o - 1] != ' ' && o + 3 < dst_size) {
                dst[o++] = ' ';
                dst[o++] = '/';
                dst[o++] = ' ';
            }
        } else {
            dst[o++] = (char)c;
        }
    }
    dst[o] = '\0';
}

static void copy_cstr(char *dst, size_t dst_size, const char *src) {
    size_t n;
    if (!dst || dst_size == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    n = strlen(src);
    if (n >= dst_size) n = dst_size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static void route_reset(void) {
    memset(&g_route, 0, sizeof(g_route));
    g_route.route_state = -1;
    g_route.maneuver_state = -1;
    g_route.distance_remaining = -1;
    g_route.distance_to_maneuver = -1;
    g_route.distance_to_maneuver_units = -1;
    g_route.maneuver_count = -1;
    g_route.lane_total = -1;
    g_route.lane_showing = -1;
}

static void session_reset(void) {
    route_reset();
    memset(g_maneuvers, 0, sizeof(g_maneuvers));
    memset(g_lanes, 0, sizeof(g_lanes));
    ++g_generation;
}

static maneuver_t *maneuver_slot(uint16_t index, int create) {
    int i, victim = -1;
    uint32_t oldest = 0xffffffffu;
    for (i = 0; i < CACHE_SIZE; ++i) {
        if (g_maneuvers[i].valid && g_maneuvers[i].index == index)
            return &g_maneuvers[i];
        if (!g_maneuvers[i].valid && victim < 0) victim = i;
    }
    if (!create) return NULL;
    if (victim < 0) {
        for (i = 0; i < CACHE_SIZE; ++i) {
            if (g_maneuvers[i].generation < oldest) {
                oldest = g_maneuvers[i].generation;
                victim = i;
            }
        }
    }
    memset(&g_maneuvers[victim], 0, sizeof(g_maneuvers[victim]));
    g_maneuvers[victim].valid = 1;
    g_maneuvers[victim].index = index;
    g_maneuvers[victim].exit_angle = 1000;
    return &g_maneuvers[victim];
}

static lane_event_t *lane_slot(uint16_t index, int create) {
    int i, victim = -1;
    uint32_t oldest = 0xffffffffu;
    for (i = 0; i < CACHE_SIZE; ++i) {
        if (g_lanes[i].valid && g_lanes[i].event_index == index)
            return &g_lanes[i];
        if (!g_lanes[i].valid && victim < 0) victim = i;
    }
    if (!create) return NULL;
    if (victim < 0) {
        for (i = 0; i < CACHE_SIZE; ++i) {
            if (g_lanes[i].generation < oldest) {
                oldest = g_lanes[i].generation;
                victim = i;
            }
        }
    }
    memset(&g_lanes[victim], 0, sizeof(g_lanes[victim]));
    g_lanes[victim].valid = 1;
    g_lanes[victim].event_index = index;
    return &g_lanes[victim];
}

static int next_tlv(const uint8_t *packet, size_t packet_len, size_t *offset,
                    uint16_t *id, const uint8_t **value, size_t *value_len) {
    uint16_t len;
    if (*offset + 4 > packet_len) return 0;
    len = be16(packet + *offset);
    if (len < 4 || *offset + len > packet_len) return 0;
    *id = be16(packet + *offset + 2);
    *value = packet + *offset + 4;
    *value_len = (size_t)len - 4;
    *offset += len;
    return 1;
}

static int parse_ctrl_hex(const char *line, uint8_t *out, size_t max_len) {
    const char *p = strstr(line, "CTRL[");
    size_t n = 0;
    if (!p) return 0;
    p += 5;
    while (*p && *p != ']' && n < max_len) {
        char tmp[3];
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == ']' || !p[0] || !p[1]) break;
        if (!isxdigit((unsigned char)p[0]) || !isxdigit((unsigned char)p[1])) {
            ++p;
            continue;
        }
        tmp[0] = p[0];
        tmp[1] = p[1];
        tmp[2] = '\0';
        out[n++] = (uint8_t)strtoul(tmp, NULL, 16);
        p += 2;
    }
    return (int)n;
}

static void parse_update(const uint8_t *packet, size_t packet_len, update_t *u) {
    size_t off = 6, n;
    uint16_t id;
    const uint8_t *v;
    memset(u, 0, sizeof(*u));
    while (next_tlv(packet, packet_len, &off, &id, &v, &n)) {
        switch (id) {
        case U_STATE:
            if (n >= 1) { u->route_state = v[0]; u->present |= UP_STATE; }
            break;
        case U_MAN_STATE:
            if (n >= 1) { u->maneuver_state = v[0]; u->present |= UP_MAN_STATE; }
            break;
        case U_ROAD:
            copy_text(u->road, sizeof(u->road), v, n); u->present |= UP_ROAD;
            break;
        case U_DEST:
            copy_text(u->destination, sizeof(u->destination), v, n); u->present |= UP_DEST;
            break;
        case U_ETA:
            if (n >= 8) { u->eta = be64(v); u->present |= UP_ETA; }
            break;
        case U_RTT:
            if (n >= 8) { u->time_remaining = be64(v); u->present |= UP_RTT; }
            break;
        case U_DIST_DEST:
            if (n >= 4) { u->distance_remaining = be32(v); u->present |= UP_DIST_DEST; }
            break;
        case U_DIST_MAN:
            if (n >= 4) { u->distance_to_maneuver = be32(v); u->present |= UP_DIST_MAN; }
            break;
        case U_DIST_MAN_STR:
            copy_text(u->distance_to_maneuver_string,
                      sizeof(u->distance_to_maneuver_string), v, n);
            u->present |= UP_DIST_MAN_STR;
            break;
        case U_DIST_MAN_UNITS:
            if (n >= 1) { u->distance_to_maneuver_units = v[0]; u->present |= UP_DIST_MAN_UNITS; }
            break;
        case U_MAN_LIST:
            u->maneuver_list_count = 0;
            while (n >= 2 && u->maneuver_list_count < MAX_ACTIVE_LIST) {
                u->maneuver_list[u->maneuver_list_count++] = be16(v);
                v += 2; n -= 2;
            }
            u->present |= UP_MAN_LIST;
            break;
        case U_MAN_COUNT:
            if (n >= 2) { u->maneuver_count = be16(v); u->present |= UP_MAN_COUNT; }
            break;
        case U_VISIBLE_IN_APP:
            if (n >= 1) { u->visible_in_app = v[0]; u->present |= UP_VISIBLE_IN_APP; }
            break;
        case U_LANE_INDEX:
            if (n >= 2) { u->lane_index = be16(v); u->present |= UP_LANE_INDEX; }
            break;
        case U_LANE_TOTAL:
            if (n >= 2) { u->lane_total = be16(v); u->present |= UP_LANE_TOTAL; }
            break;
        case U_LANE_SHOWING:
            if (n >= 1) { u->lane_showing = v[0]; u->present |= UP_LANE_SHOWING; }
            break;
        case U_SOURCE_SUPPORTS_RG:
            if (n >= 1) {
                u->source_supports_rg = v[0];
                u->present |= UP_SOURCE_SUPPORTS_RG;
            }
            break;
        default:
            break;
        }
    }
}

static void parse_maneuver(const uint8_t *packet, size_t packet_len) {
    size_t off = 6, n;
    uint16_t id, index = 0xffff;
    const uint8_t *v;
    maneuver_t parsed;
    maneuver_t *dst;
    memset(&parsed, 0, sizeof(parsed));
    parsed.exit_angle = 1000;
    while (next_tlv(packet, packet_len, &off, &id, &v, &n)) {
        switch (id) {
        case M_INDEX:
            if (n >= 2) index = be16(v);
            break;
        case M_DESC:
            copy_text(parsed.description, sizeof(parsed.description), v, n);
            break;
        case M_TYPE:
            if (n >= 1) { parsed.maneuver_type = v[0]; parsed.has_type = 1; }
            break;
        case M_AFTER_ROAD:
            copy_text(parsed.after_road, sizeof(parsed.after_road), v, n);
            break;
        case M_DIST_BETWEEN:
            if (n >= 4) parsed.distance_between = be32(v);
            break;
        case M_DIST_STR:
            copy_text(parsed.distance_string, sizeof(parsed.distance_string), v, n);
            break;
        case M_DIST_UNITS:
            if (n >= 1) parsed.distance_units = v[0];
            break;
        case M_DRIVING_SIDE:
            if (n >= 1) parsed.driving_side = v[0];
            break;
        case M_JUNCTION_TYPE:
            if (n >= 1) parsed.junction_type = v[0];
            break;
        case M_EXIT_ANGLE:
            if (n >= 2) { parsed.exit_angle = (int16_t)be16(v); parsed.has_exit_angle = 1; }
            break;
        case M_LINKED_LANE:
            if (n >= 2) { parsed.linked_lane_index = be16(v); parsed.has_linked_lane = 1; }
            break;
        default:
            break;
        }
    }
    if (index == 0xffff) return;
    dst = maneuver_slot(index, 1);
    parsed.valid = 1;
    parsed.index = index;
    parsed.generation = ++g_generation;
    *dst = parsed;
}

static int parse_lane_information(const uint8_t *v, size_t n, lane_item_t *lane) {
    size_t off = 0, vn;
    uint16_t id;
    const uint8_t *value;
    int saw = 0;
    memset(lane, 0, sizeof(*lane));
    lane->highlight = 1000;
    while (next_tlv(v, n, &off, &id, &value, &vn)) {
        switch (id) {
        case LI_INDEX:
            if (vn >= 2) lane->position = be16(value);
            else if (vn >= 1) lane->position = value[0];
            saw = 1;
            break;
        case LI_STATUS:
            if (vn >= 1) lane->status = value[0];
            saw = 1;
            break;
        case LI_ANGLES:
            while (vn > 0 && lane->angle_count < MAX_ANGLES) {
                if (vn >= 2) {
                    lane->angles[lane->angle_count++] = (int16_t)be16(value);
                    value += 2; vn -= 2;
                } else {
                    lane->angles[lane->angle_count++] = value[0];
                    ++value; --vn;
                }
            }
            saw = 1;
            break;
        case LI_HIGHLIGHT:
            if (vn >= 2) lane->highlight = (int16_t)be16(value);
            else if (vn >= 1) lane->highlight = value[0];
            lane->has_highlight = 1;
            saw = 1;
            break;
        default:
            break;
        }
    }
    return saw;
}

static void parse_lane_event(const uint8_t *packet, size_t packet_len) {
    size_t off = 6, n;
    uint16_t id, index = 0xffff;
    const uint8_t *v;
    lane_event_t parsed;
    lane_event_t *dst;
    memset(&parsed, 0, sizeof(parsed));
    while (next_tlv(packet, packet_len, &off, &id, &v, &n)) {
        if (id == L_EVENT_INDEX) {
            if (n >= 2) index = be16(v);
            else if (n >= 1) index = v[0];
        } else if (id == L_INFORMATION && parsed.lane_count < MAX_LANES) {
            lane_item_t item;
            if (parse_lane_information(v, n, &item))
                parsed.lane[parsed.lane_count++] = item;
        }
    }
    if (index == 0xffff) return;
    dst = lane_slot(index, 1);
    parsed.valid = 1;
    parsed.event_index = index;
    parsed.generation = ++g_generation;
    *dst = parsed;
}

static int appendf(char *dst, size_t size, int off, const char *fmt, ...) {
    int n;
    va_list ap;
    if (off < 0 || (size_t)off >= size) return -1;
    va_start(ap, fmt);
    n = vsnprintf(dst + off, size - (size_t)off, fmt, ap);
    va_end(ap);
    if (n < 0 || (size_t)n >= size - (size_t)off) return -1;
    return off + n;
}

static int lane_direction(const lane_item_t *lane) {
    if (lane->has_highlight) return lane->highlight;
    if (lane->angle_count > 0) return lane->angles[0];
    return 1000;
}

static maneuver_t *active_maneuver(void) {
    if (!g_route.has_active_maneuver) return NULL;
    return maneuver_slot(g_route.active_maneuver_index, 0);
}

enum {
    LANE_SELECT_NONE = 0,
    LANE_SELECT_EXPLICIT = 1,
    LANE_SELECT_LINKED = 2,
    LANE_SELECT_MANEUVER_INDEX = 3
};

static lane_event_t *usable_lane_event(uint16_t index) {
    lane_event_t *lane = lane_slot(index, 0);
    if (!lane || lane->lane_count == 0) return NULL;
    return lane;
}

static lane_event_t *active_lane_event(maneuver_t *man, uint16_t *selected_index,
                                        int *selection_source) {
    uint16_t index;
    lane_event_t *lane;
    *selection_source = LANE_SELECT_NONE;

    /*
     * Some Chinese navigation apps publish a valid 0x5204 for the current
     * maneuver while omitting 0x5201 TLV 0x0010 and leaving TLV 0x0012 at
     * zero.  Do not reject the cached event solely because of that flag.
     * Keep the association strict so a batch-sent future event is never used.
     */
    if (g_route.has_active_lane) {
        index = g_route.active_lane_index;
        lane = usable_lane_event(index);
        if (lane) {
            *selected_index = index;
            *selection_source = LANE_SELECT_EXPLICIT;
            return lane;
        }
    }
    if (man && man->has_linked_lane) {
        index = man->linked_lane_index;
        lane = usable_lane_event(index);
        if (lane) {
            *selected_index = index;
            *selection_source = LANE_SELECT_LINKED;
            return lane;
        }
    }
    if (man && g_route.has_active_maneuver) {
        index = g_route.active_maneuver_index;
        lane = usable_lane_event(index);
        if (lane) {
            *selected_index = index;
            *selection_source = LANE_SELECT_MANEUVER_INDEX;
            return lane;
        }
    }
    return NULL;
}

static void ensure_parent_dir(void) {
    mkdir(PPS_DIR, 0777);
}

static int acquire_singleton_lock(void) {
    struct flock lock;
    char pid_text[32];
    int n;
    ensure_parent_dir();
    g_lock_fd = open(LOCK_PATH, O_RDWR | O_CREAT, 0666);
    if (g_lock_fd < 0) return -1;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(g_lock_fd, F_SETLK, &lock) != 0) {
        close(g_lock_fd);
        g_lock_fd = -1;
        return 0;
    }
    if (ftruncate(g_lock_fd, 0) != 0) {
        close(g_lock_fd);
        g_lock_fd = -1;
        return -1;
    }
    n = snprintf(pid_text, sizeof(pid_text), "%ld\n", (long)getpid());
    if (n > 0 && write(g_lock_fd, pid_text, (size_t)n) != n) {
        close(g_lock_fd);
        g_lock_fd = -1;
        return -1;
    }
    return 1;
}

static void write_heartbeat(void) {
    char tmp[512];
    FILE *f;
    uint64_t now = monotonic_ms();
    if (now - g_last_heartbeat_ms < HEARTBEAT_INTERVAL_MS) return;
    snprintf(tmp, sizeof(tmp), "%s.tmp.%ld", HEARTBEAT_PATH, (long)getpid());
    f = fopen(tmp, "w");
    if (!f) return;
    fprintf(f, "%llu\n", (unsigned long long)now);
    fclose(f);
    chmod(tmp, 0666);
    if (rename(tmp, HEARTBEAT_PATH) != 0) unlink(tmp);
    else chmod(HEARTBEAT_PATH, 0666);
    g_last_heartbeat_ms = now;
}

static int write_snapshot(void) {
    char body[SNAPSHOT_SIZE], complete[SNAPSHOT_SIZE], tmp[512];
    int off = 0, i;
    maneuver_t *man = active_maneuver();
    lane_event_t *lane;
    uint16_t lane_index = 0xffff;
    const char *turn_to = "";
    const char *man_desc = "";
    int man_type = -1, exit_angle = 1000, driving_side = 0, junction_type = 0;
    int man_index = -1, lane_count = 0;
    int lane_selection_source = LANE_SELECT_NONE;

    lane = active_lane_event(man, &lane_index, &lane_selection_source);
    if (g_route.has_active_maneuver) man_index = g_route.active_maneuver_index;
    if (man) {
        turn_to = man->after_road[0] ? man->after_road : man->description;
        man_desc = man->description;
        if (man->has_type) man_type = man->maneuver_type;
        exit_angle = man->has_exit_angle ? man->exit_angle : 1000;
        driving_side = man->driving_side;
        junction_type = man->junction_type;
    }
    if (lane) lane_count = lane->lane_count;

    if (g_replay && lane) {
        fprintf(stderr,
            "V521_REPLAY_LANE man=%d lane_event=%u lanes=%u showing=%d source=%d\n",
            man_index, (unsigned)lane_index, (unsigned)lane->lane_count,
            g_route.has_lane_showing ? g_route.lane_showing : -1,
            lane_selection_source);
    }

    off = appendf(body, sizeof(body), off,
        "state::%d\nman_state::%d\nroad::%s\ndest::%s\n"
        "eta::%llu\nrtt::%llu\ndist_dest::%lld\ndist_man::%lld\n"
        "dist_man_str::%s\ndist_man_units::%d\nman_index::%d\n"
        "man_type::%d\nturn_to::%s\nman_desc::%s\ndriving_side::%d\n"
        "junction_type::%d\nexit_angle::%d\nman_count::%d\n"
        "lane_idx::%d\nlane_count::%d\nlane_status::",
        g_route.route_state, g_route.maneuver_state, g_route.road,
        g_route.destination, (unsigned long long)g_route.eta,
        (unsigned long long)g_route.time_remaining,
        (long long)g_route.distance_remaining,
        (long long)g_route.distance_to_maneuver,
        g_route.distance_to_maneuver_string,
        g_route.distance_to_maneuver_units, man_index, man_type,
        turn_to, man_desc, driving_side, junction_type, exit_angle,
        g_route.maneuver_count,
        lane ? (int)lane_index : -1, lane_count);
    if (off < 0) return -1;
    for (i = 0; lane && i < lane->lane_count; ++i)
        off = appendf(body, sizeof(body), off, "%s%d", i ? "," : "", lane->lane[i].status);
    off = appendf(body, sizeof(body), off, "\nlane_dir::");
    for (i = 0; lane && i < lane->lane_count; ++i)
        off = appendf(body, sizeof(body), off, "%s%d", i ? "," : "", lane_direction(&lane->lane[i]));
    /* V53.2: publish complete lane angles and the active lane screen. */
    off = appendf(body, sizeof(body), off, "\nlane_angles::");
    for (i = 0; lane && i < lane->lane_count; ++i) {
        int a;
        if (i) off = appendf(body, sizeof(body), off, "|");
        for (a = 0; a < lane->lane[i].angle_count; ++a)
            off = appendf(body, sizeof(body), off, "%s%d",
                a ? "," : "", lane->lane[i].angles[a]);
    }
    off = appendf(body, sizeof(body), off, "\nlane_active_scr::%d",
        lane ? 1 : 0);
    off = appendf(body, sizeof(body), off, "\n");
    if (off < 0) return -1;
    unsigned long next_sequence;
    if (strcmp(body, g_last_body) == 0) return 0;
    next_sequence = g_sequence + 1;
    if (snprintf(complete, sizeof(complete), "%sseq::%lu\n", body, next_sequence) >= (int)sizeof(complete))
        return -1;

    ensure_parent_dir();
    snprintf(tmp, sizeof(tmp), "%s.tmp.%ld", g_output_path, (long)getpid());
    {
        FILE *f = fopen(tmp, "w");
        if (!f) return -1;
        if (fwrite(complete, 1, strlen(complete), f) != strlen(complete)) {
            fclose(f); unlink(tmp); return -1;
        }
        fflush(f);
        fclose(f);
    }
    chmod(tmp, 0666);
    if (rename(tmp, g_output_path) != 0) { unlink(tmp); return -1; }
    chmod(g_output_path, 0666);
    copy_cstr(g_last_body, sizeof(g_last_body), body);
    g_sequence = next_sequence;
    return 1;
}

static void publish_reset(const char *reason) {
    session_reset();
    g_route.route_state = 0;
    g_last_raw_route_state = 0;
    g_route_zero_started_ms = 0;
    g_route_zero_deadline_ms = 0;
    g_route_zero_from_reroute = 0;
    g_last_body[0] = '\0';
    if (write_snapshot() < 0) log_line("snapshot write failed during reset");
    if (reason) log_line(reason);
}

static void cancel_pending_route_zero(const char *reason) {
    if (g_route_zero_started_ms == 0) return;
    g_route_zero_started_ms = 0;
    g_route_zero_deadline_ms = 0;
    g_route_zero_from_reroute = 0;
    if (reason) log_line(reason);
}

static int update_has_active_evidence(const update_t *u) {
    if (!u) return 0;
    if ((u->present & UP_STATE) && u->route_state > 0) return 1;
    if ((u->present & UP_MAN_COUNT) && u->maneuver_count > 0) return 1;
    if ((u->present & UP_MAN_LIST) && u->maneuver_list_count > 0) return 1;
    if ((u->present & UP_DIST_MAN) && u->distance_to_maneuver > 0) return 1;
    if ((u->present & UP_DIST_DEST) && u->distance_remaining > 0) return 1;
    if ((u->present & UP_DEST) && u->destination[0] != '\0') return 1;
    if ((u->present & UP_ROAD) && u->road[0] != '\0') return 1;
    return 0;
}

static void flush_pending_route_zero(void) {
    uint64_t now;
    if (g_route_zero_started_ms == 0 || g_route.route_state <= 0) return;
    if (g_route_zero_from_reroute || g_route_zero_deadline_ms == 0) return;
    now = monotonic_ms();
    if (now < g_route_zero_deadline_ms) return;
    publish_reset("ROUTE: deferred state=0 committed after 800ms grace");
}

static void consume_control_requests(void) {
    if (unlink(CLEAR_REQUEST) == 0) {
        g_suppress_updates = 1;
        publish_reset("CONTROL: disconnect/close request consumed; stale packets suppressed");
    }
    if (unlink(RESUME_REQUEST) == 0) {
        g_suppress_updates = 0;
        cancel_pending_route_zero(NULL);
        log_line("CONTROL: reconnect/resync resume consumed; preserving last stable cache");
    }
}

/* Returns 0 when a transient reset-only frame must not be published. */
static int merge_update(const update_t *u) {
    int i;
    int skip_route_state = 0;
    int active_evidence = update_has_active_evidence(u);

    if ((u->present & UP_SOURCE_SUPPORTS_RG) && u->source_supports_rg == 0) {
        publish_reset("ROUTE: source_supports_rg=0 hard clear");
        return 1;
    }

    if (u->present & UP_STATE) {
        if (u->route_state == 0) {
            uint8_t previous_raw = g_last_raw_route_state;
            g_last_raw_route_state = 0;
            if (g_route.route_state <= 0) {
                g_route.route_state = 0;
            } else if (active_evidence) {
                cancel_pending_route_zero("ROUTE: state=0 canceled by active 0x5201 evidence");
                skip_route_state = 1;
            } else {
                uint64_t now = monotonic_ms();
                if (g_route_zero_started_ms == 0) {
                    g_route_zero_started_ms = now;
                    g_route_zero_from_reroute =
                        previous_raw == RGD_STATE_REROUTING ||
                        g_route.route_state == RGD_STATE_REROUTING;
                    g_route_zero_deadline_ms = g_route_zero_from_reroute
                        ? 0 : now + ROUTE_ZERO_NORMAL_GRACE_MS;
                    log_line(g_route_zero_from_reroute
                        ? "ROUTE: reroute state=0 deferred until active data or hard clear"
                        : "ROUTE: state=0 deferred for 800ms grace");
                }
                return 0;
            }
        } else {
            g_last_raw_route_state = u->route_state;
            cancel_pending_route_zero("ROUTE: pending state=0 canceled by route_state>0");
            g_route.route_state = u->route_state;
        }
    } else if (active_evidence) {
        cancel_pending_route_zero("ROUTE: pending state=0 canceled by active 0x5201");
    } else if (g_route_zero_started_ms != 0) {
        return 0;
    }
    if (u->present & UP_MAN_STATE) g_route.maneuver_state = u->maneuver_state;
    if (u->present & UP_ROAD) copy_cstr(g_route.road, sizeof(g_route.road), u->road);
    if (u->present & UP_DEST) copy_cstr(g_route.destination, sizeof(g_route.destination), u->destination);
    if (u->present & UP_ETA) g_route.eta = u->eta;
    if (u->present & UP_RTT) g_route.time_remaining = u->time_remaining;
    if (u->present & UP_DIST_DEST) g_route.distance_remaining = u->distance_remaining;
    if (u->present & UP_DIST_MAN) g_route.distance_to_maneuver = u->distance_to_maneuver;
    if (u->present & UP_DIST_MAN_STR)
        copy_cstr(g_route.distance_to_maneuver_string,
                  sizeof(g_route.distance_to_maneuver_string),
                  u->distance_to_maneuver_string);
    if (u->present & UP_DIST_MAN_UNITS)
        g_route.distance_to_maneuver_units = u->distance_to_maneuver_units;
    if (u->present & UP_MAN_COUNT) g_route.maneuver_count = u->maneuver_count;
    if (u->present & UP_MAN_LIST) {
        g_route.has_active_maneuver = 0;
        for (i = 0; i < u->maneuver_list_count; ++i) {
            if (u->maneuver_list[i] != 0xffff) {
                g_route.active_maneuver_index = u->maneuver_list[i];
                g_route.has_active_maneuver = 1;
                break;
            }
        }
    }
    if (u->present & UP_LANE_INDEX) {
        g_route.active_lane_index = u->lane_index;
        g_route.has_active_lane = u->lane_index != 0xffff;
    }
    if (u->present & UP_LANE_TOTAL) g_route.lane_total = u->lane_total;
    if (u->present & UP_LANE_SHOWING) {
        g_route.lane_showing = u->lane_showing;
        g_route.has_lane_showing = 1;
        if (!u->lane_showing && !(u->present & UP_LANE_INDEX))
            g_route.has_active_lane = 0;
    }
    (void)skip_route_state;
    return 1;
}

static void process_packet(const uint8_t *packet, size_t packet_len) {
    uint16_t msgid;
    if (packet_len < 6) return;
    consume_control_requests();
    msgid = be16(packet + 4);
    if (g_suppress_updates &&
        (msgid == MSG_ROUTE_UPDATE || msgid == MSG_MANEUVER || msgid == MSG_LANE))
        return;
    if (msgid == MSG_ROUTE_UPDATE) {
        update_t u;
        ++g_count_5201;
        parse_update(packet, packet_len, &u);
        if (!merge_update(&u)) return;
        if (!g_replay) touch_file(RX_5201_MARKER);
        write_snapshot();
    } else if (msgid == MSG_MANEUVER) {
        ++g_count_5202;
        cancel_pending_route_zero("ROUTE: pending state=0 canceled by 0x5202");
        parse_maneuver(packet, packet_len);
        if (!g_replay) touch_file(RX_5202_MARKER);
        write_snapshot();
    } else if (msgid == MSG_LANE) {
        ++g_count_5204;
        cancel_pending_route_zero("ROUTE: pending state=0 canceled by 0x5204");
        parse_lane_event(packet, packet_len);
        if (!g_replay) touch_file(RX_5204_MARKER);
        write_snapshot();
    }
}

static int replay_file(const char *path) {
    FILE *f = fopen(path, "r");
    char line[LINE_SIZE];
    uint8_t packet[PACKET_SIZE];
    if (!f) { perror(path); return 2; }
    session_reset();
    write_snapshot();
    while (fgets(line, sizeof(line), f)) {
        int n = parse_ctrl_hex(line, packet, sizeof(packet));
        if (n >= 6) process_packet(packet, (size_t)n);
    }
    fclose(f);
    fprintf(stderr, "V521_REPLAY_COUNTS 5201=%lu 5202=%lu 5204=%lu\n",
            g_count_5201, g_count_5202, g_count_5204);
    return 0;
}

enum log_follow_action {
    LOG_FOLLOW_WAIT = 0,
    LOG_FOLLOW_SEEK_SAVED,
    LOG_FOLLOW_REOPEN_ZERO
};

/*
 * Decide how to follow iap2.log without ever replaying bytes already parsed.
 * A same-inode file that grows after stdio reached EOF is a normal append,
 * not a replacement.  Only a missing/replaced/truncated file may restart at
 * byte zero.
 */
static enum log_follow_action choose_log_follow_action(
        int path_stat_ok, int fd_stat_ok, int identity_changed,
        off_t path_size, long saved_position, int lines_read) {
    if (!path_stat_ok || !fd_stat_ok) return LOG_FOLLOW_REOPEN_ZERO;
    if (identity_changed || path_size < saved_position)
        return LOG_FOLLOW_REOPEN_ZERO;
    if (lines_read == 0 && path_size > saved_position)
        return LOG_FOLLOW_SEEK_SAVED;
    return LOG_FOLLOW_WAIT;
}

static int log_follow_selftest(void) {
    if (choose_log_follow_action(0, 1, 0, 100, 100, 0) !=
            LOG_FOLLOW_REOPEN_ZERO) return 1;
    if (choose_log_follow_action(1, 0, 0, 100, 100, 0) !=
            LOG_FOLLOW_REOPEN_ZERO) return 2;
    if (choose_log_follow_action(1, 1, 1, 200, 100, 0) !=
            LOG_FOLLOW_REOPEN_ZERO) return 3;
    if (choose_log_follow_action(1, 1, 0, 90, 100, 0) !=
            LOG_FOLLOW_REOPEN_ZERO) return 4;
    if (choose_log_follow_action(1, 1, 0, 200, 100, 0) !=
            LOG_FOLLOW_SEEK_SAVED) return 5;
    if (choose_log_follow_action(1, 1, 0, 200, 100, 1) !=
            LOG_FOLLOW_WAIT) return 6;
    if (choose_log_follow_action(1, 1, 0, 100, 100, 0) !=
            LOG_FOLLOW_WAIT) return 7;
    fprintf(stderr, "V5322_LOG_FOLLOW_SELFTEST PASS\n");
    return 0;
}

static void daemon_loop(void) {
    FILE *f = NULL;
    char line[LINE_SIZE];
    uint8_t packet[PACKET_SIZE];
    long last_position = 0;
    int had_log = 0;
    struct stat opened_st;
    int have_opened_st = 0;
    session_reset();
    write_snapshot();
    touch_file(START_MARKER);
    log_line("cpvc-rgd-sync V53.2.2 REPLAY GUARD TEST started (append-safe log follow)");

    for (;;) {
        struct stat st, fd_st;
        int lines_read = 0;
        int reopen_log = 0;
        int path_stat_ok;
        int fd_stat_ok;
        int identity_changed = 0;
        enum log_follow_action follow_action;
        consume_control_requests();
        flush_pending_route_zero();
        write_heartbeat();
        if (!f) {
            f = fopen(IAP2_LOG, "r");
            if (!f) { usleep(200000); continue; }
            if (!had_log) {
                /*
                 * Never replay a previous CarPlay session on daemon start.
                 * The wrapper/native sender requests a fresh 0x5200 snapshot,
                 * so only newly appended packets belong to this session.
                 */
                if (fseek(f, 0, SEEK_END) != 0) {
                    fclose(f);
                    f = NULL;
                    usleep(200000);
                    continue;
                }
                last_position = ftell(f);
                if (last_position < 0) last_position = 0;
            } else {
                /* A replacement log is a new inode and must be read at byte 0. */
                last_position = 0;
            }
            have_opened_st = fstat(fileno(f), &opened_st) == 0;
            had_log = 1;
            log_line("iap2.log opened; last stable route cache preserved");
        }
        while (fgets(line, sizeof(line), f)) {
            int n;
            ++lines_read;
            last_position = ftell(f);
            n = parse_ctrl_hex(line, packet, sizeof(packet));
            if (n >= 6) process_packet(packet, (size_t)n);
        }
        clearerr(f);
        path_stat_ok = stat(IAP2_LOG, &st) == 0;
        fd_stat_ok = fstat(fileno(f), &fd_st) == 0;
        if (path_stat_ok && fd_stat_ok && have_opened_st) {
            identity_changed = st.st_dev != opened_st.st_dev ||
                               st.st_ino != opened_st.st_ino;
        }
        follow_action = choose_log_follow_action(
                path_stat_ok, fd_stat_ok, identity_changed,
                path_stat_ok ? st.st_size : 0, last_position, lines_read);
        if (follow_action == LOG_FOLLOW_SEEK_SAVED) {
            /*
             * QNX stdio can remain at EOF briefly after the same inode grows.
             * Resume from the exact parsed offset.  Reopening at byte zero here
             * replays every old 0x5201/0x5202/0x5204 packet to the cluster.
             */
            if (fseek(f, last_position, SEEK_SET) != 0) {
                clearerr(f);
                usleep(100000);
            }
            continue;
        }
        if (follow_action == LOG_FOLLOW_REOPEN_ZERO) {
            reopen_log = 1;
        }
        if (reopen_log) {
            fclose(f);
            f = NULL;
            have_opened_st = 0;
            /* Keep the last valid snapshot visible while a new log inode is
             * opened.  The phone is asked for a fresh snapshot in parallel;
             * RESUME_REQUEST no longer clears this stable cache. */
            touch_file(RESYNC_REQUEST);
            if (path_stat_ok && fd_stat_ok && identity_changed) {
                log_line("WATCHDOG: iap2.log inode replaced; cache preserved and fresh 0x5200 requested");
            } else if (path_stat_ok && fd_stat_ok && st.st_size < last_position) {
                log_line("WATCHDOG: iap2.log truncated; cache preserved and fresh 0x5200 requested");
            } else {
                log_line("WATCHDOG: iap2.log path/fd unavailable; cache preserved and fresh 0x5200 requested");
            }
        } else {
            usleep(100000);
        }
    }
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--selftest-log-follow") == 0) {
        return log_follow_selftest();
    }
    if (argc == 4 && strcmp(argv[1], "--replay") == 0) {
        g_replay = 1;
        g_output_path = argv[3];
        return replay_file(argv[2]);
    }
    if (argc != 1) {
        fprintf(stderr, "usage: %s [--replay iap2.log output.pps]\n", argv[0]);
        return 2;
    }
    {
        int lock_result = acquire_singleton_lock();
        if (lock_result == 0) return 0;
        if (lock_result < 0) {
            fprintf(stderr, "cpvc-rgd-sync: cannot acquire singleton lock\n");
            return 3;
        }
    }
    daemon_loop();
    return 0;
}
