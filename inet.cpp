#include "syshead.h"
#include "inet.h"
#include "socket.h"
#include "sock.h"

extern struct net_ops ip_ops;


static int INET_OPS = 1;

struct net_family inet = {
    .create = inet_create,
};

static struct sock_ops inet_stream_ops = {
    .connect = &inet_connect,
    .write = &inet_write,
    .read = &inet_read,
    .close = &inet_close,
    .free = &inet_free,
    .abort = &inet_abort,
    .getpeername = &inet_getpeername,
    .getsockname = &inet_getsockname,
};

static struct sock_type inet_ops[] = {
    {
        .sock_ops = &inet_stream_ops,
        .net_ops = &ip_ops,
        .type = SOCK_RAW,
        .protocol = IPPROTO_IP,
    }
};

int inet_create(struct socket *sock, int protocol)
{
    struct sock *sk;
    struct sock_type *skt = NULL;

    for (int i = 0; i < INET_OPS; i++) {
        if (inet_ops[i].type & sock->type) {
            skt = &inet_ops[i];
            break;
        }
    }

    if (!skt) {
        print_err("Could not find socktype for socket\n");
        return 1;
    }

    sock->ops = skt->sock_ops;

    sk = sk_alloc(skt->net_ops, protocol);
    sk->protocol = protocol;
    
    sock_init_data(sock, sk);
    
    return 0;
}

int inet_socket(struct socket *sock, int protocol)
{
    return 0;
}

int inet_connect(struct socket *sock, const struct sockaddr *addr,int addr_len, int flags)
{
    return 0;
}

int inet_write(struct socket *sock, const void *buf, int len)
{
    struct sock *sk = sock->sk;

    return sk->ops->write(sk, buf, len);
}

int inet_read(struct socket *sock, void *buf, int len)
{
    struct sock *sk = sock->sk;

    return sk->ops->read(sk, buf, len);
}

struct sock *inet_lookup(struct sk_buff *skb, uint16_t sport, uint16_t dport)
{
    struct socket *sock = socket_lookup(sport, dport);
    if (sock == NULL) return NULL;
    
    return sock->sk;
}

int inet_close(struct socket *sock)
{
    if (!sock) {
        return 0;
    }

    struct sock *sk = sock->sk;

    return sock->sk->ops->close(sk);
}

int inet_free(struct socket *sock)
{
    struct sock *sk = sock->sk;
    sock_free(sk);
    free(sock->sk);
    
    return 0;
}

int inet_abort(struct socket *sock)
{
    struct sock *sk = sock->sk;
    
    if (sk) {
        sk->ops->abort(sk);
    }

    return 0;
}

int inet_getpeername(struct socket *sock, struct sockaddr *address,
                     socklen_t *address_len)
{
    struct sock *sk = sock->sk;

    if (sk == NULL) {
        return -1;
    }

    struct sockaddr_in *res = (struct sockaddr_in *) address;
    res->sin_family = AF_INET;
    res->sin_port = htons(sk->dport);
    res->sin_addr.s_addr = htonl(sk->daddr);
    *address_len = sizeof(struct sockaddr_in);

    inet_dbg(sock, "geetpeername sin_family %d sin_port %d sin_addr %d addrlen %d",
             res->sin_family, ntohs(res->sin_port), ntohl(res->sin_addr.s_addr), *address_len);
    
    return 0;
}
int inet_getsockname(struct socket *sock, struct sockaddr *address,
                     socklen_t *address_len)
{
    struct sock *sk = sock->sk;

    if (sk == NULL) {
        return -1;
    }
    
    struct sockaddr_in *res = (struct sockaddr_in *) address;
    res->sin_family = AF_INET;
    res->sin_port = htons(sk->sport);
    res->sin_addr.s_addr = htonl(sk->saddr);
    *address_len = sizeof(struct sockaddr_in);

    inet_dbg(sock, "getsockname sin_family %d sin_port %d sin_addr %d addrlen %d",
             res->sin_family, ntohs(res->sin_port), ntohl(res->sin_addr.s_addr), *address_len);
    
    return 0;
}
