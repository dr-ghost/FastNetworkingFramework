#include "listener.h"

listener::listener(int port, const char* address) : socktd(0), port(port), address(address), islistening(false)
{
    ;
}

listener::~listener()
{
    if (socktd > 0)
        close(socktd);
}

UDPStream* listener::start()
{
    if (islistening == true)
        return NULL;
    
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    string pstr = to_string(port);
    const char* pchr = pstr.c_str();

    if ((rv = getaddrinfo(NULL, pchr, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }


    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((socktd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(socktd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socktd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return NULL;
    }
    islistening = true;

    freeaddrinfo(servinfo);
    
    return new UDPStream(socktd);
}
