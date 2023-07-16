#include "ethernet.h"
#include "icmp.h"
#include "ip.h"
#include "utils.h"

void icmp_incoming(struct sk_buff *skb) 
{
    struct iphdr *iphdr = ip_hdr(skb);
    struct icmp_ *icmp = (struct icmp_*) iphdr->data;

    //TODO: Check csum

    switch (icmp->type) {
    case ICMP_ECHO:
        icmp_reply(skb);
        return;
    case ICMP_DST_UNREACHABLE:
        print_err("ICMPv4 received 'dst unreachable' code %d, "
                  "check your routes and firewall rules\n", icmp->code);
        free_skb(skb);
        return;
    default:
        print_err("ICMPv4 did not match supported types\n");
        free_skb(skb);
        return;
    }

}

void icmp_reply(struct sk_buff *skb)
{
    struct iphdr *iphdr = ip_hdr(skb);
    struct icmp_ *icmp;
    struct sock sk;
    memset(&sk, 0, sizeof(struct sock));
    
    uint16_t icmp_len = iphdr->len - (iphdr->ihl * 4);

    skb_reserve(skb, ETH_HDR_LEN + IP_HDR_LEN + icmp_len);
    skb_push(skb, icmp_len);
    
    icmp = (struct icmp_ *)skb->data;
        
    icmp->type = ICMP_REPLY;
    icmp->csum = 0;
    icmp->csum = checksum(icmp, icmp_len, 0);

    skb->protocol = ICMP;
    sk.daddr = iphdr->saddr;

    ip_output(&sk, skb);
    free_skb(skb);
}
