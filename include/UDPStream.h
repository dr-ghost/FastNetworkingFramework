#ifndef UPSTREAM_INCLUDED
#define UPSTREAM_INCLUDED
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define cipherKey 'S'

using namespace std;

class UDPStream
{
    int socktd;
    string peerIP;
    int peerPort;
    struct sockaddr_in* addr;
    int addrlen;

public:
    friend class listener;
    friend class broadcaster;

    ~UDPStream();

    ssize_t send(const char* buffer, size_t len);
    ssize_t recv(char* buffer, size_t len);


    string getPeerIP();
    int getPeerPort();

    void* get_in_addr(struct sockaddr* sa);
    void* get_in_port(struct sockaddr* sa);

private:
    UDPStream(int socktd, struct sockaddr_in* addr, int addrlen);
    UDPStream();
    UDPStream(const UDPStream& stream);
    UDPStream(int socktd);

    char Cipher(char c);
};
#endif