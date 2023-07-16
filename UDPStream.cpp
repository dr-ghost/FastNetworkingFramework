#include "UDPStream.h"

UDPStream::UDPStream(int socktd, struct sockaddr_in* addr, int addrlen) : socktd(socktd), addr(addr), addrlen(addrlen)
{
    char ip4[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(addr->sin_addr), ip4, INET_ADDRSTRLEN);

    peerIP = ip4;

    peerPort = ntohs(addr->sin_port);   
}
UDPStream::UDPStream(int socktd) : socktd(socktd)
{
    ;
}
UDPStream::~UDPStream()
{
    close(socktd);
}

string UDPStream::getPeerIP()
{
    return peerIP;
}

int UDPStream::getPeerPort()
{
    return peerPort;
}
void * UDPStream::get_in_addr(struct sockaddr *sa)
{

    return &(((struct sockaddr_in*)sa)->sin_addr);

}

void * UDPStream::get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_port);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_port);
}
ssize_t UDPStream::send(const char* buffer, size_t len)
{
    ssize_t numbytes;
    if ((numbytes = sendto(socktd, buffer, len, 0, (struct sockaddr*)addr, addrlen)) == -1) {
        perror("-1");
        return -1;
    }
    return numbytes;
}

ssize_t UDPStream::recv(char* buffer, size_t len)
{
    ssize_t numbytes;
    //struct sockaddr_storage their_addr;
    //socklen_t addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(socktd, buffer, len, 0, (struct sockaddr*)&addr, (socklen_t *)&addrlen)) == -1) {
        perror("recvfrom");
        return -1;
    }

    if (peerIP == "")
    {
        char ip4 [INET_ADDRSTRLEN];

        inet_ntop(AF_INET, get_in_addr((struct sockaddr* )&addr), ip4, sizeof ip4);

        peerIP = ip4;

        //peerPort = htons(addr->sin_port);
    }
    
    return numbytes;
}

/*
int UDPStream::sendFile(UDPStream *upStream, UDPStream *dwnStream, FILE* fp)
{
    
    int i, len;
    if (fp == NULL) {
        char buffer[] = "File not Found";
        len = strlen("File not Found");
        buffer[len] = EOF;
        /*
        for (i = 0; i <= len; i++)
            buffer[i] = Cipher(buffer[i]);
        
        upStream->send(buffer, sizeof buffer);
        return 1;
    }
    const int BUFF_SZ = 65554;

    char fbuffer[BUFF_SZ], rbuffer[10];

    clearBuf(fbuffer, sizeof fbuffer);

    int data = 1;

    size_t numbts;
    int noPkts = 0;
    while (data != 0)
    {
        data = fread(fbuffer, 1, BUFF_SZ, fp);

        if (data == 0)
        {
            break;
        }



        upStream->send(fbuffer, sizeof fbuffer);
        noPkts++;
        if ((numbts = dwnStream->recv(rbuffer, sizeof rbuffer)) != -1)
        {
            continue;
        }
    }

    upStream->send(".", sizeof ".");

    printf("Streamed the video using %d packets\n", noPkts);
    return 0;
}
*/
char UDPStream::Cipher(char ch)
{
    return ch ^ cipherKey;
}
/*
int UDPStream::recvFile(UDPStream *upStream, UDPStream *dwnStream, FILE* fp)
{
    const int BUFF_SZ = 65554;

    size_t numbts;

    char rbuffer[BUFF_SZ];
    while (true)
    {
        if ((numbts = dwnStream->recv(rbuffer, sizeof rbuffer)) != -1)
        {
            if (strncmp(".", rbuffer, sizeof ".") == 0)
                break;
            fwrite(rbuffer, 1, BUFF_SZ, fp);
        }

        upStream->send(".", sizeof ".");
    }
    
    return 0;
}
*/