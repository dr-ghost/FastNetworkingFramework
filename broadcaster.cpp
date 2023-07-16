#include "broadcaster.h"


UDPStream* broadcaster::connect(const char* server, int port)
{
    
    struct addrinfo *servinfo, *p;
    int socktd;

    if (resolveHost(server, port, servinfo) != 0)
        return NULL;
    
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((socktd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("talker: socket");
            continue;
        }
        uint32_t on = 1;
        if(setsockopt(socktd,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on)) < 0)
        {
            perror("not with a speaker");
            continue;
        }
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return NULL;
    }
    
    return new UDPStream(socktd, (struct sockaddr_in*)p->ai_addr, p->ai_addrlen);
}

int broadcaster::resolveHost(const char* server, int port, struct addrinfo*& servinfo)
{
    struct addrinfo hints;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    string pstr = to_string(port);
    const char* pchr = pstr.c_str();

    if ((rv = getaddrinfo(server, pchr, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    return rv;

}