#include <iostream>
#include <thread>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <vector>


#include "broadcaster.h"
#include "listener.h"

using namespace std;
using namespace boost::placeholders;

void clearBuf(char* b, size_t len)
{
    int i;
    for (i = 0; i < len; i++)
        b[i] = '\0';
}

int sendFile(UDPStream *upStream, UDPStream *dwnStream, FILE* fp)
{
    
    int i, len;
    if (fp == NULL) {
        cout<<"!"<<endl;
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
    const int BUFF_SZ = 6555;

    char fbuffer[BUFF_SZ], rbuffer[10];

    clearBuf(fbuffer, sizeof fbuffer);

    int data = 1;

    size_t numbts;
    int noPkts = 0;
    //cout<<"~"<<endl;
    while (data != 0)
    {
        data = fread(fbuffer, 1, BUFF_SZ, fp);

        if (data == 0)
        {
            break;
        }

        for (int i = 0; i < 10000; ++i)
            ;

        upStream->send(fbuffer, sizeof fbuffer);
        noPkts++;
        cout<<"~"<<endl;
        if ((numbts = dwnStream->recv(rbuffer, sizeof rbuffer)) != -1)
        {
            cout<<noPkts<<endl;
            continue;
        }
        
    }
    cout<<"#"<<endl;
    upStream->send(".", sizeof ".");

    printf("Streamed the video using %d packets\n", noPkts);
    return 0;
}
int recvFile(UDPStream *upStream, UDPStream *dwnStream, FILE* fp)
{
    const int BUFF_SZ = 6555;

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
/*
class BlockingInt
{
public:
    explicit BlockingInt(const int st) : val(st)
    {}
    explicit BlockingInt() : val(0)
    {}
    void increment()
    {
        boost::mutex::scoped_lock lock(mutex);
        val++;

    }
    int getVal()
    {
        boost::mutex::scoped_lock lock(mutex);
        return val;
    }
    ~BlockingInt()
    {}

private:
    boost::mutex mutex;
    int val;

};
*/

char movielst[] = "movie_01.mp4 movie_02.mp4";


void ServerClientInt(string ip, int pooled)
{
    listener serv(pooled);
    broadcaster spkr;

    UDPStream *dwnStream, *upStream;

    if ((dwnStream = serv.start()) == NULL)
    {
        cout<<"Server Thread at "<<pooled<<" coulden't start"<<endl;
        return;
    }

    char buffer[1000];

    clearBuf(buffer, sizeof buffer);

    ssize_t numbts;

    if ((upStream = spkr.connect(ip.c_str(), 4423)) == NULL)
    {
        cout<<"Client "<<ip<<" is not responding"<<endl;
        return;
    }
    
    upStream->send(to_string(pooled).c_str(), sizeof to_string(pooled).c_str());

    while (true)
    {
        clearBuf(buffer, sizeof buffer);
        if ((numbts = dwnStream->recv(buffer, sizeof buffer)) != -1)
        {
            /*
            cout<<numbts<<endl;
            for (int i = 0; i < numbts; ++i)
                cout<<buffer[i];
            cout<<endl;
            */
            
            for (int i = 0; i < numbts; ++i)
            {
                cout<<buffer[i];
            }

            cout<<endl;

            if (strncmp("movie_01", buffer, sizeof "movie_01") == 0)
            {
                upStream->send("ok sending", sizeof "ok sending");
                
                if ((numbts = dwnStream->recv(buffer, sizeof buffer)) != -1)
                {
                    //cout<<"~"<<endl;
                    FILE *f = fopen("movie_01.mp4", "rb");
                    sendFile(upStream, dwnStream, f);
                    fclose(f);
                }
                
                
            }
            else if (strncmp("movie_02", buffer, sizeof "movie_02") == 0)
            {
                upStream->send("ok sending", sizeof "ok sending");
                
                if ((numbts = dwnStream->recv(buffer, sizeof buffer)) != -1)
                {
                    FILE *f = fopen("movie_02.mp4", "rb");
                    sendFile(upStream, dwnStream, f);
                }
                
                
                
            }
            else if (strncmp("movie_list", buffer, sizeof "movie_list") == 0)
            {
                upStream->send(movielst, sizeof movielst);
            }
            else if (strncmp("downloa", buffer, sizeof("downloa")) == 0)
            {
                string s;
                for (int i = sizeof("download"); i < numbts; ++i)
                {
                    s+=buffer[i];
                }
                cout<<"!"<<endl;
                cout<<s<<endl;
                string cmd = "python3 youtube.py ";
                cmd += s;
                cout<<cmd<<endl;
                system(cmd.c_str());
            }
            else if (strncmp("close", buffer, sizeof "close") == 0)
            {
                upStream->send("closing connection", sizeof "closing connection");
                break;
            }
            else
            {
                upStream->send("Cannot Parse ur command", sizeof "Cannot Parse ur command");
            }
        }
    }
}

int main(int argc, char* argv[])
{
    vector<thread> threadQ;

    int portPool = 4423;

    listener server(portPool);
    broadcaster speaker;

    UDPStream *dwnStream;
    UDPStream *upStream;
    
    if ((dwnStream = server.start()) == NULL)
    {
        perror("Server not starting!");
        exit(1);
    }


    char buffer[100];

    clearBuf(buffer, sizeof buffer);

    ssize_t numbts;
    while (true)
    {
        if ((numbts = dwnStream->recv(buffer, sizeof buffer)) != -1)
        {
            string s;

            for (int i = 0; i < numbts; ++i)
            {
                s += buffer[i];
            }

            cout<<"A wild "<<s<<" has appeared!!"<<endl;

            if ((upStream = speaker.connect(s.c_str(), 4423)) == NULL)
            {
                cout<<"Client "<<s<<" is not responding"<<endl;
                continue;
            }

            portPool++;

            thread tmp(ServerClientInt, s, portPool);

            threadQ.push_back(move(tmp));
        }
        clearBuf(buffer, sizeof buffer);
    }

    for (int i = 0; i < threadQ.size(); ++i)
    {
        threadQ[i].join();
    }
}