#include <iostream>
#include <string>

#include "broadcaster.h"
#include "listener.h"

using namespace std;

void clearBuf(char* b, size_t len)
{
    int i;
    for (i = 0; i < len; i++)
        b[i] = '\0';
}

int recvFile(UDPStream *upStream, UDPStream *dwnStream, FILE* fp)
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

int sendFile(UDPStream *upStream, UDPStream *dwnStream, FILE* fp)
{
    
    int i, len;
    if (fp == NULL) {
        char buffer[] = "File not Found";
        len = strlen("File not Found");
        buffer[len] = EOF;
        /*
        for (i = 0; i <= len; i++)
            buffer[i] = Cipher(buffer[i]);
        */ 
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

int main(int argc, char *argv[])
{
    listener lstn(4423);
    broadcaster spkr;

    UDPStream *dwnStream, *upStream;

    if ((dwnStream = lstn.start()) == NULL)
    {
        cout<<"Can't listen"<<endl;
        return 1;
    }

    char sbuffer[100], rbuffer[100];

    clearBuf(sbuffer, sizeof sbuffer);
    clearBuf(rbuffer, sizeof rbuffer);

    ssize_t numbts;

    if ((upStream = spkr.connect("172.31.22.34", 4423)) == NULL)
    {
        cout<<"Server not available"<<endl;
        return 1;
    }

    upStream->send("172.31.19.187", sizeof "172.31.19.187");

    string rPort;


    if ((numbts = dwnStream->recv(rbuffer, sizeof rbuffer)) != -1)
    {
        for (int i = 0; i < numbts; ++i)
        {
            rPort += rbuffer[i];
        }
    }

    if ((upStream = spkr.connect("172.31.22.34", stoi(rPort))) == NULL)
    {
        cout<<"Server not available"<<endl;
        return 1;
    }
    while (true)
    {
        string cmd;
        cin>>cmd;
        upStream->send(cmd.c_str(), cmd.size());
        clearBuf(rbuffer, sizeof rbuffer);

        if ((numbts = dwnStream->recv(rbuffer, sizeof rbuffer)) != -1)
        {
            for (int i = 0; i < numbts; ++i)
                cout<<rbuffer[i];
            
            if (strncmp("ok sending", rbuffer, sizeof "ok sending"))
            {
                upStream->send(".", sizeof ".");
                
                FILE *f = fopen("movie.mp4", "wb");

                recvFile(upStream, dwnStream, f);
            }
            cout<<endl;
        }


    }
    cout<<endl;

    return 0;
}
