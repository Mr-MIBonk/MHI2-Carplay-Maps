/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
#ifndef V75_CARD_CHANNEL_H
#define V75_CARD_CHANNEL_H
/* Called under one adapter mutex. Only the adapter's alt-display scene/view/
 * IDR commands use this lane. Original primary-screen requestUI/showUI is not
 * hijacked. No files, sockets, live pointers or CF references are owned here.
 */
#include <stdint.h>
#include <string.h>
#include "card_protocol.h"
#define CPMAP_CARD_RIGHT 2
#define CPMAP_CARD_VIEW_COUNT 5
#include "card_transition_policy.h"
#undef CPMAP_CARD_RIGHT
#undef CPMAP_CARD_VIEW_COUNT

struct v75_card_channel {
    struct card_transition_policy policy;
    struct card_command command;
    uintptr_t session;
    uint64_t epoch,revision,preference_generation;
    uint32_t serial,pending;
    int cluster,active,replay,position,navigating;
    int dispatch_done,callback_seen,callback_status,exhausted;
};
struct v75_card_dispatch {
    uint32_t token;
    uintptr_t session;
    uint64_t epoch;
    int operation,position,view;
};
int v75_card_channel_init(struct v75_card_channel *,int cluster);
int v75_card_channel_session(struct v75_card_channel *,uintptr_t session);
int v75_card_channel_desire(struct v75_card_channel *,int active,int position,
                            int navigating,uint64_t preference_generation);
int v75_card_channel_take(struct v75_card_channel *,struct v75_card_dispatch *);
int v75_card_channel_dispatched(struct v75_card_channel *,uint32_t token,int status);
int v75_card_channel_completed(struct v75_card_channel *,uint32_t token,int status);
#endif
