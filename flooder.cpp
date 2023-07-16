#include "broadcaster.h"
#include "listener.h"

void clearBuf(char* b, size_t len)
{
    int i;
    for (i = 0; i < len; i++)
        b[i] = '\0';
}
//172.31.19.91 ankur
//172.31.19.187 adarsh
//172.31.24.185 lahore
//172.31.54.145 amkit
//172.16.100.6 ldap
int main(int argc, char* argv[])
{
    broadcaster client;
    
    UDPStream * upStream;

    if ((upStream = client.connect("107.180.41.236", 80)) == NULL)
    {
        perror("Error in creating upStream");
        exit(1);
    }
    
    while(true)
    {
        char buffer[1024];
        upStream->send(buffer, sizeof buffer);
        //clearBuf(buffer, sizeof buffer);
    }
}