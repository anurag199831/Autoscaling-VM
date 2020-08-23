#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <thread>
#include <mutex>
using namespace std;
//Client side

int port=8100;
int N=4,vm_limit=3;
mutex mtx;
thread th[11];

char *ippool[10]={"127.0.0.1","192.168.122.181","192.168.122.182","192.168.122.183","192.168.122.184","192.168.122.185"};
int sleeptimer;
void foo(char * serverIp)
{
    //create a message buffer 
    char msg[1500]; 
    //setup a socket and connection tools 
    struct hostent* host = gethostbyname(serverIp); 
    sockaddr_in sendSockAddr;   
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = 
    inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);
    // mtx.lock();
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    //try to connect...
    int status = connect(clientSd,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket! "<<serverIp<<endl;
        return ;
    }
    cout << "\n Connected to the server! " <<serverIp<< endl;
    // mtx.unlock();
    int bytesRead, bytesWritten = 0;
    struct timeval start1, end1;
    gettimeofday(&start1, NULL);
    while(1)
    {
        if((serverIp[14]-48)>N)
        return;
        usleep(sleeptimer);
        mtx.lock();
        // cout <<serverIp<< " >";
        string data;
        // getline(cin, data);
        data="hh";
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, data.c_str());
        if(data == "exit")
        {
            send(clientSd, (char*)&msg, strlen(msg), 0);
            break;
        }
        bytesWritten += send(clientSd, (char*)&msg, strlen(msg), 0);
        mtx.unlock();
        // cout << "Awaiting server response..." << endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer
        bytesRead += recv(clientSd, (char*)&msg, sizeof(msg), 0);
        mtx.lock();
        if(!strcmp(msg, "exit"))
        {
            cout << serverIp << " Server has quit the session" << endl;
            break;
        }
        cout << serverIp <<" Server: " << msg << endl;
        mtx.unlock();
    }
    gettimeofday(&end1, NULL);
    close(clientSd);
    cout << "********Session********" << endl;
    cout << "Bytes written: " << bytesWritten << 
    " Bytes read: " << bytesRead << endl;
    cout << "Elapsed time: " << (end1.tv_sec- start1.tv_sec) 
      << " secs" << endl;
    cout << "Connection closed" << endl;
}

void check()
{
    //create a message buffer 
    char msg[1500]; 
    char *serverIp="127.0.0.1";
    //setup a socket and connection tools 
    struct hostent* host = gethostbyname(serverIp); 
    sockaddr_in sendSockAddr;   
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = 
    inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);
    mtx.lock();
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    //try to connect...
    int status = connect(clientSd,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket! monitor"<<endl;
        return ;
    }
    cout << "\n Connected to the server! " <<serverIp<< endl;
    mtx.unlock();
    int bytesRead, bytesWritten = 0;
    struct timeval start1, end1;
    gettimeofday(&start1, NULL);
    while(1)
    {
        // usleep(2000000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // mtx.lock();
        // cout <<serverIp<< " >";
        string data;
        // getline(cin, data);
        data="check";
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, data.c_str());
        bytesWritten += send(clientSd, (char*)&msg, strlen(msg), 0);
        // cout << "Awaiting server response..." << endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer
        bytesRead += recv(clientSd, (char*)&msg, sizeof(msg), 0);
        if(msg[0]=='+' && msg[1]=='1')
        {
            // mtx.lock();
            if(N<vm_limit)
            {
                N++;
                th[N]=thread(foo,ippool[N]);
            }
            // mtx.unlock();
            data="done";
            memset(&msg, 0, sizeof(msg));//clear the buffer
            strcpy(msg, data.c_str());
            bytesWritten += send(clientSd, (char*)&msg, strlen(msg), 0);
        }
        else if(msg[0]=='-' && msg[1]=='1')
        {
            // mtx.lock();
            if(N>1)
            N--;
            // mtx.unlock();

            data="done";
            memset(&msg, 0, sizeof(msg));//clear the buffer
            strcpy(msg, data.c_str());
            bytesWritten += send(clientSd, (char*)&msg, strlen(msg), 0);
        }
        else if(msg[0]=='0' && msg[1]=='0')
        {
            data="done";
            memset(&msg, 0, sizeof(msg));//clear the buffer
            strcpy(msg, data.c_str());
            bytesWritten += send(clientSd, (char*)&msg, strlen(msg), 0);
        }
        cout<<"done sent"<<endl;
        // mtx.unlock();
    }
    gettimeofday(&end1, NULL);
    close(clientSd);
    cout << "********Session********" << endl;
    cout << "Bytes written: " << bytesWritten << 
    " Bytes read: " << bytesRead << endl;
    cout << "Elapsed time: " << (end1.tv_sec- start1.tv_sec) 
      << " secs" << endl;
    cout << "Connection closed" << endl;
}


int main(int argc, char *argv[])
{
    //we need 2 things: ip address and port number, in that order
    // if(argc != 3)
    // {
    //     cerr << "Usage: ip_address port" << endl; exit(0); 
    // } //grab the IP address and port number 
    //char *serverIp = argv[1]; int port = atoi(argv[2]); 
    // char *serverIp = "192.168.122.139";
    // char *serverIp = "127.0.0.1";
    char *serverIp;
    // thread th1(foo, serverIp); 
    // th1.join();
    cout<<"Enter number of VMs online: ";
    cin>>N;
    cout<<"Enter cpu load level \nLow =1 \nMedium=2 \nHigh=3 \nEnter input: ";
    int mode;
    cin>>mode;
    if(mode==1)
    sleeptimer=10000;
    else if(mode==2)
    sleeptimer=1000;
    else sleeptimer=0;
    for(int i=1;i<=N;i++)
    {
        serverIp = ippool[i];
        th[i]=thread(foo, serverIp); 
    }
    check();
    // th[10]=thread(check);
    // for(int i=1;i<=N;i++)
    // {
    //     th[i].join();
    // }

    return 0;    
}