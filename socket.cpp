#include "syshead.h"
#include "utils.h"
#include "socket.h"
#include "inet.h"
#include "wait.h"

static int sock_amount = 0;
static LIST_HEAD(sockets);

extern struct net_family inet;

static struct net_family *families[128] = {
    &inet,
};

static struct socket *alloc_socket(pid_t pid)
{
    // TODO: Figure out a way to not shadow kernel file descriptors.
    // Now, we'll just expect the fds for a process to never exceed this.
    static int fd = 4097;
    struct socket *sock = (struct socket *)malloc(sizeof (struct socket));
    list_init(&sock->list);

    sock->pid = pid;
    sock->refcnt = 1;

    sock->fd = fd++;

    sock->state = SS_UNCONNECTED;
    sock->ops = NULL;
    sock->flags = O_RDWR;

    
    return sock;
}

int socket_release(struct socket *sock)
{
    int rc = 0;
    sock->refcnt--;

    if (sock->refcnt == 0) {
        free(sock);
    } else {
        ;
    }

    return rc;
}

int socket_free(struct socket *sock)
{
    list_del(&sock->list);
    sock_amount--;

    if (sock->ops) {
        sock->ops->free(sock);
    }

    socket_release(sock);
    
    return 0;
}

static void *socket_garbage_collect(void *arg)
{
    struct socket *sock = socket_find((struct socket *)arg);

    if (sock == NULL) return NULL;

    socket_free(sock);

    return NULL;
}

int socket_delete(struct socket *sock)
{
    int rc = 0;

    if (sock->state == SS_DISCONNECTING) return rc;

    sock->state = SS_DISCONNECTING;
    socket_garbage_collect(sock);

    return rc;
}

void abort_sockets() {
    struct list_head *item, *tmp;
    struct socket *sock;

    list_for_each_safe(item, tmp, &sockets) {
        sock = list_entry(item, struct socket, list);
        sock->ops->abort(sock);
    }
}

static struct socket *get_socket(pid_t pid, uint32_t fd)
{
    struct list_head *item;
    struct socket *sock = NULL;

    list_for_each(item, &sockets) {
        sock = list_entry(item, struct socket, list);
        if (sock->pid == pid && sock->fd == fd)
        {
            return sock;
        }
    }
    
    sock = NULL;
    return sock;
    
}

struct socket *socket_lookup(uint16_t remoteport, uint16_t localport)
{
    struct list_head *item;
    struct socket *sock = NULL;
    struct sock *sk = NULL;
    
    list_for_each(item, &sockets) {
        sock = list_entry(item, struct socket, list);

        if (sock == NULL || sock->sk == NULL) continue;
        sk = sock->sk;

        if (sk->sport == localport && sk->dport == remoteport) {
            return sock;
        }
    }

    sock = NULL;

    return sock;
    
}
struct socket *socket_find(struct socket *find)
{
    struct list_head *item;
    struct socket *sock = NULL;

    list_for_each(item, &sockets) {
        sock = list_entry(item, struct socket, list);
        if (sock == find)
        {
            return sock;
        }
    }
    
    sock = NULL;

    return sock;
    
}

#ifdef DEBUG_SOCKET
void socket_debug()
{
    struct list_head *item;
    struct socket *sock = NULL;


    list_for_each(item, &sockets) {
        sock = list_entry(item, struct socket, list);
        socket_rd_acquire(sock);
        socket_dbg(sock, "");
        socket_release(sock);
    }

}
#else
void socket_debug()
{
    return;
}
#endif

int abort_socket(struct socket* sock)
{
    socket_free(sock);
    return -1;
}
int _socket(pid_t pid, int domain, int type, int protocol)
{
    struct socket *sock;
    struct net_family *family;

    if ((sock = alloc_socket(pid)) == NULL) {
        print_err("Could not alloc socket\n");
        return -1;
    }

    sock->type = type;

    family = families[domain];

    if (!family) {
        print_err("Domain not supported: %d\n", domain);
        return abort_socket(sock);
    }
    
    if (family->create(sock, protocol) != 0) {
        print_err("Creating domain failed\n");
        return abort_socket(sock);
    }

    
    list_add_tail(&sock->list, &sockets);
    sock_amount++;

    int rc = sock->fd;
    socket_release(sock);

    return rc;

}

int _connect(pid_t pid, int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    struct socket *sock;

    if ((sock = get_socket(pid, sockfd)) == NULL) {
        print_err("Connect: could not find socket (fd %u) for connection (pid %d)\n", sockfd, pid);
        return -EBADF;
    }

    int rc = sock->ops->connect(sock, addr, addrlen, 0);
    switch (rc) {
    case -EINVAL:
    case -EAFNOSUPPORT:
    case -ECONNREFUSED:
    case -ETIMEDOUT:
        socket_release(sock);
        socket_free(sock);
        break;
    default:
        socket_release(sock);
    }
    
    return rc;
}

int _write(pid_t pid, int sockfd, const void *buf, const unsigned int count)
{
    struct socket *sock;

    if ((sock = get_socket(pid, sockfd)) == NULL) {
        print_err("Write: could not find socket (fd %u) for connection (pid %d)\n", sockfd, pid);
        return -EBADF;
    }

    int rc = sock->ops->write(sock, buf, count);
    socket_release(sock);

    return rc;
}

int _read(pid_t pid, int sockfd, void *buf, const unsigned int count)
{
    struct socket *sock;

    if ((sock = get_socket(pid, sockfd)) == NULL) {
        print_err("Read: could not find socket (fd %u) for connection (pid %d)\n", sockfd, pid);
        return -EBADF;
    }

    int rc = sock->ops->read(sock, buf, count);
    socket_release(sock);

    return rc;
}

int _close(pid_t pid, int sockfd)
{
    struct socket *sock;

    if ((sock = get_socket(pid, sockfd)) == NULL) {
        print_err("Close: could not find socket (fd %u) for connection (pid %d)\n", sockfd, pid);
        return -EBADF;
    }


    int rc = sock->ops->close(sock);
    socket_release(sock);

    return rc;
}

int _poll(pid_t pid, struct pollfd fds[], nfds_t nfds, int timeout)
{
    for (;;) {
        int polled = 0;

        for (int i = 0; i < nfds; i++) {
            struct socket *sock;
            struct pollfd *poll = &fds[i];
            if ((sock = get_socket(pid, poll->fd)) == NULL) {
                print_err("Poll: could not find socket (fd %u) for connection (pid %d)\n", poll->fd, pid);
                return -EBADF;
            }

            poll->revents = sock->sk->poll_events & (poll->events | POLLHUP | POLLERR | POLLNVAL);
            if (poll->revents > 0) {
                polled++;
            }
            socket_release(sock);
        }

        if (polled > 0 || timeout == 0) {
            return polled;
        } else {
            if (timeout > 0) {
                if (timeout > 10) {
                    timeout -= 10;
                } else {
                    timeout = 0;
                }
            }
            usleep(1000 * 10);
        }
    }

    return -EAGAIN;
}

int out_(struct socket* sock, int rc)
{
    socket_release(sock);
    return rc;
}
int _fcntl(pid_t pid, int fildes, int cmd, ...)
{
    struct socket *sock;

    if ((sock = get_socket(pid, fildes)) == NULL) {
        print_err("Fcntl: could not find socket (fd %u) for connection (pid %d)\n", fildes, pid);
        return -EBADF;
    }

    va_list ap;
    int rc = 0;

    switch (cmd) {
    case F_GETFL:
        rc = sock->flags;
        return out_(sock, rc);
    case F_SETFL:
        va_start(ap, cmd);
        sock->flags = va_arg(ap, int);
        va_end(ap);
        rc = 0;
        return out_(sock, rc);
    default:
        rc = -1;
        return out_(sock, rc);
    }

    rc = -1;

    socket_release(sock);
    return rc;
}

int _getsockopt(pid_t pid, int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    struct socket *sock;

    if ((sock = get_socket(pid, fd)) == NULL) {
        print_err("Getsockopt: could not find socket (fd %u) for connection (pid %d)\n", fd, pid);
        return -EBADF;
    }

    int rc = 0;

    switch (level) {
    case SOL_SOCKET:
        switch (optname) {
        case SO_ERROR:
            *optlen = 4;
            *(int *)optval = sock->sk->err;
            rc = 0;
            break;
        default:
            print_err("Getsockopt unsupported optname %d\n", optname);
            rc =  -ENOPROTOOPT;
            break;
        }
        
        break;
    default:
        print_err("Getsockopt: Unsupported level %d\n", level);
        rc = -EINVAL;
        break;
    }

    socket_release(sock);

    return rc;
}

int _getpeername(pid_t pid, int socket, struct sockaddr *address,
                 socklen_t *address_len)
{
    struct socket *sock;

    if ((sock = get_socket(pid, socket)) == NULL) {
        print_err("Getpeername: could not find socket (fd %u) for connection (pid %d)\n", socket, pid);
        return -EBADF;
    }

    int rc = sock->ops->getpeername(sock, address, address_len);
    socket_release(sock);

    return rc;
}

int _getsockname(pid_t pid, int socket, struct sockaddr *address,
                 socklen_t *address_len)
{
    struct socket *sock;

    if ((sock = get_socket(pid, socket)) == NULL) {
        print_err("Getsockname: could not find socket (fd %u) for connection (pid %d)\n", socket, pid);
        return -EBADF;
    }

    int rc = sock->ops->getsockname(sock, address, address_len);
    socket_release(sock);

    return rc;
}
