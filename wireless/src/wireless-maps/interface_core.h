/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
#ifndef V75_INTERFACE_CORE_H
#define V75_INTERFACE_CORE_H
#include <stddef.h>
#include <stdint.h>
#define V75_CSM_MAX 4096
#define V75_TX_MAX 2038
/* All functions are transport independent and fail closed on malformed TLVs.
 * They never generate authentication, audio, map pixels or synthetic RGI. */
int v75_csm_valid(const uint8_t *, size_t);
size_t v75_prepare_csm(const uint8_t *, size_t, uint8_t *, size_t);
int v75_button_command(const char *, unsigned *, unsigned *);
#endif
