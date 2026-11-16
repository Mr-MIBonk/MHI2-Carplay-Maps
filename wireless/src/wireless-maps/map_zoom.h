/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
/* Existing wired semantic commands, delivered to the accepted wireless session.
 * The worker owns only 127.0.0.1:7032. It does not consume mirror-canvas:7033,
 * touch, track controls, audio, display geometry or frame-rate settings. */
#ifndef V75_MAP_ZOOM_H
#define V75_MAP_ZOOM_H
#include <stddef.h>
int v75_zoom_command(const char *, size_t);
int v75_zoom_start(void);
int v75_zoom_selftest(void);
#endif
