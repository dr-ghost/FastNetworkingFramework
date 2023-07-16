#ifndef LISTENER_INCLUDED
#define LISTENER_INCLUDED
//c headers (socket api)
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
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <string>
//c++ headers
#include "UDPStream.h"

using namespace std;
class listener
{
    int socktd;
    string address;
    int port;
    bool islistening;

public:
    listener(int port, const char* address = "");
    ~listener();

    UDPStream* start();

};
#endif