#include "syshead.h"
#include "skbuff.h"
#include "arp.h"
#include "ip.h"
#include "icmp.h"
#include "utils.h"

static void ip_init_pkt(struct iphdr *ih)
{
    ih->saddr = ntohl(ih->saddr);
    ih->daddr = ntohl(ih->daddr);
    ih->len = ntohs(ih->len);
    ih->id = ntohs(ih->id);
}

int ip_rcv(struct sk_buff *skb)
{
    struct iphdr *ih = ip_hdr(skb);
    uint16_t csum = -1;

    if (ih->version != IPV4) {
        print_err("Datagram version was not IPv4\n");
        free_skb(skb);
        return 0;
    }

    if (ih->ihl < 5) {
        print_err("IPv4 header length must be at least 5\n");
        free_skb(skb);
        return 0;
    }

    if (ih->ttl == 0) {
        //TODO: Send ICMP error
        print_err("Time to live of datagram reached 0\n");
        free_skb(skb);
        return 0;
    }

    csum = checksum(ih, ih->ihl * 4, 0);

    if (csum != 0) {
        // Invalid checksum, drop packet handling
        free_skb(skb);
        return 0;
    }

    // TODO: Check fragmentation, possibly reassemble

    ip_init_pkt(ih);

    ip_dbg("in", ih);

    switch (ih->proto) {
    case ICMP:
        icmp_incoming(skb);
        return 0;
    default:
        print_err("Unknown IP header proto\n");
        free_skb(skb);
        return 0;
    }

}
