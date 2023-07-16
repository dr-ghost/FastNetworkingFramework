#ifndef BROADCASTER_INCLUDED
#define BROADCASTER_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "UDPStream.h"

using namespace std;

class broadcaster
{
public:
    UDPStream* connect(const char* server, int port);

private:
    int resolveHost(const char* server, int port, struct addrinfo*& servinfo);
};
#endif