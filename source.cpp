#include "sock.h"
#include "tuntap_if.h"
#include "arp.h"
#include "route.h"
#include "netdev.h"
#include "raw_socket.h"
#include "list.h"

int running = 1;
int debug = 0;

extern struct netdev* netdev;

struct sk_buff *arp_skb;

char* GetIPString(uint32_t x)
{
      char buffer[INET_ADDRSTRLEN + 1];
      auto result = inet_ntop(AF_INET, &x, buffer, sizeof(buffer));
      return buffer;
}

struct sock * allocate_sock(int protocol);
int disconnect(struct sock * sk, int flag);
int read(struct sock *sk, void *buf, int len);
int recv_notify(struct sock *sk);
int close_sock(struct sock * sk);
int abort_sock(struct sock * sk);
int sock_init(struct sock *sk);

struct net_ops ip_ops{
    .alloc_sock = &allocate_sock,
    .init = &sock_init,
    .disconnect = &disconnect,
    .read = &read,
    .recv_notify = &recv_notify,
    .close = &close_sock,
    .abort = &abort_sock
};

static void init_stack()
{
    socket_init("wlp1s0", ETH_P_ALL);
    //tun_init();
    netdev_init();
    route_init();
    arp_init();
}

void free_stack()
{
    abort_sockets();
    free_arp();
    free_routes();
    free_netdev();
    //free_tun();
    free_socket();
}

int ipStringToNumber (const char* pDottedQuad, unsigned int *    pIpAddr)
{
   unsigned int            byte3;
   unsigned int            byte2;
   unsigned int            byte1;
   unsigned int            byte0;
   char              dummyString[2];

   /* The dummy string with specifier %1s searches for a non-whitespace char
    * after the last number. If it is found, the result of sscanf will be 5
    * instead of 4, indicating an erroneous format of the ip-address.
    */
   if (sscanf (pDottedQuad, "%u.%u.%u.%u%1s", &byte3, &byte2, &byte1, &byte0, dummyString) == 4)
   {
    if (    (byte3 < 256)
           && (byte2 < 256)
           && (byte1 < 256)
           && (byte0 < 256)
        )
        {
         *pIpAddr  =   (byte3 << 24)
                     + (byte2 << 16)
                     + (byte1 << 8)
                     +  byte0;

         return 1;
        }
    }

   return 0;
}

int main()
{
    init_stack();
    print_debug("%u", netdev->addr);
    //netdev_rx_loop();
    
    
    //while (true)
        //arp_request(sa, da, netdev);

    
    netdev_rx_loop();

    free_stack();
    return 0;
}

struct sock * allocate_sock(int protocol)
{
    struct sock * sck = (struct sock*)malloc(sizeof(struct sock));

    return sck;
}

int disconnect(struct sock * sk, int flag)
{
    return 0;
}

int read(struct sock *sk, void *buf, int len)
{
    return 0;
}

int recv_notify(struct sock *sk)
{
    return 0;
}

int close_sock(struct sock * sk)
{
    return 0;
}

int abort_sock(struct sock * sk)
{
    return 0;
}

int sock_init(struct sock *sk)
{
    return 0;
}