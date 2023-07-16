#include "raw_socket.h"
#include <net/if.h>
#include <netpacket/packet.h>
#include "syshead.h"
#include "utils.h"

static int socket_fd;
static struct sockaddr_ll addr;

static int socket_alloc(int protocol)
{
    int fd = socket(AF_PACKET, SOCK_RAW, htons(protocol));

    if (fd == -1)
    {
        print_err("Socket Creation failed!!");
        return -1;
    }

    return fd;
}
int bind_using_iface_ip(int fd, char *ipaddr, uint16_t port)
{
    struct sockaddr_in localaddr = {0};
    localaddr.sin_family    = AF_PACKET;
    localaddr.sin_port  = htons(port);
    localaddr.sin_addr.s_addr = inet_addr(ipaddr);
    return bind(fd, (struct sockaddr*) &localaddr, sizeof(struct sockaddr_in));
}

int bind_using_iface_name(int fd, char *iface_name)
{
    return setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, iface_name, strlen(iface_name));
}
void socket_init(char *if_name, int protocol)
{
    socket_fd = socket_alloc(protocol);

    if (socket_fd == -1)
    {
        print_err("Socket not initialised");
        return ;
    }

    bind_using_iface_name(socket_fd, if_name);
}

int socket_read(char *buffer, int len)
{
    return read(socket_fd, buffer, len); 
}

int socket_write(char *buffer, int len)
{
    return write(socket_fd, buffer, len);
}

void free_socket()
{
    close(socket_fd);
}