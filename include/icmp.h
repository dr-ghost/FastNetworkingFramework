#ifndef ICMP_H
#define ICMP_H

#include "syshead.h"
#include "skbuff.h"

#define ICMP_REPLY           0x00
#define ICMP_DST_UNREACHABLE 0x03
#define ICMP_SRC_QUENCH      0x04
#define ICMP_REDIRECT        0x05
#define ICMP_ECHO            0x08
#define ICMP_ROUTER_ADV      0x09
#define ICMP_ROUTER_SOL      0x0a
#define ICMP_TIMEOUT         0x0b
#define ICMP_MALFORMED       0x0c

struct icmp_ {
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint8_t data[];
} __attribute__((packed));

struct icmp_echo {
    uint16_t id;
    uint16_t seq;
    uint8_t data[];
} __attribute__((packed));

struct icmp_dst_unreachable {
    uint8_t unused;
    uint8_t len;
    uint16_t var;
    uint8_t data[];
} __attribute__((packed));


void icmp_incoming(struct sk_buff *skb);
void icmp_reply(struct sk_buff *skb);

#endif
