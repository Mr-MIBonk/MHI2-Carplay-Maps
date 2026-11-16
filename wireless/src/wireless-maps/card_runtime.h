/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
#ifndef V75_CARD_RUNTIME_H
#define V75_CARD_RUNTIME_H
#include <stddef.h>
int v75_cards_start(void);
int v75_cards_selftest(int cluster);
void v75_cards_new_session(void);
void v75_cards_route_observed(void);
void v75_cards_route_packet(const unsigned char *,size_t);
#endif
