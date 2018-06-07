#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include <stdint.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <err.h>
#include <errno.h>
#include <time.h>

#include <iostream>

namespace Daan
{
    namespace Utilities
    {
        std::size_t split(std::string s, const char delim, std::vector<std::string> *result) {
            std::size_t count;
            
            std::stringstream ss(s);
            std::string item;

            count = result->size();

            while (std::getline(ss, item, delim))
                result->push_back(item);

            return result->size() - count;
        }

        bool isNumeric(std::string s)
        {
            return !s.empty() && std::find_if(s.begin(), 
                s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
        }
    }

    namespace Net
    {
        class IPAddress
        {
            private:
                std::vector<uint8_t> _ip;

            public:    
                IPAddress()
                {
                    int i;
                    _ip.clear();

                    for(i = 0; i < 4; i++)
                        _ip.push_back(0);
                }


                IPAddress(std::string ip)
                {
                    int i;
                    std::size_t n;
                    std::vector<std::string> p;

                    n = Utilities::split(ip, '.', &p);

                    if(n != 4 && n != 6)
                        throw ("Invalid IP Address. The IP should consist out of either 4, or 6 bytes");

                    for(i = 0; i < n; i++)
                    {
                        if(!Utilities::isNumeric(p.at(i)))
                            throw("All members of the IP should be numeric");
                        
                        _ip.push_back(uint8_t (atoi(p.at(i).c_str())));
                    }
                }

                IPAddress(std::vector<uint8_t> *ip) 
                {
                    if(!this->Validate(ip))
                        throw ("Invalid IP Address. The IP should consist out of either 4, or 6 bytes");

                    _ip = *ip;
                }

                static bool Validate(std::vector<uint8_t> *ip)
                {
                    return (ip->size() == 4 || ip->size() == 6);
                }

                std::string ToString()
                {
                    int i;
                    std::stringstream ss;

                    ss.clear();

                    for(i = 0; i < _ip.size(); i++)
                    {
                        ss << unsigned(_ip.at(i)) << ((i < _ip.size() - 1) ? "." : "");                
                    }

                    return ss.str();
                }

                std::vector<uint8_t> Get() { return _ip; }
        };

        class TCPClient
        {
            private:
                bool _isconnected;
                IPAddress _ip;
                unsigned short _port;
    
            public:
                int Socket;

                TCPClient(IPAddress ip, unsigned short port)
                {
                    _ip = ip;
                    _port = port;

                    std::cout<<"IP = " << ip.ToString() << std::endl;
     
                    //Create socket
                    Socket = socket(AF_INET , SOCK_STREAM , 0);
                    if (Socket == -1)
                        throw("Could not create socket");
                }
                
                bool isConnected()
                {
                    fd_set rfd;
                    FD_ZERO(&rfd);
                    FD_SET(Socket, &rfd);
                    timeval tv = { 0 };
                    select(Socket+1, &rfd, 0, 0, &tv);
                    if (!FD_ISSET(Socket, &rfd))
                        return false;
                    int n = 0;
                    ioctl(Socket, FIONREAD, &n);
                }

                bool Connect()
                {
                    struct sockaddr_in server;

                    server.sin_addr.s_addr = inet_addr(_ip.ToString().c_str());
                    server.sin_family = AF_INET;
                    server.sin_port = htons( _port );
 
                    //Connect to remote server
                    _isconnected = connect(Socket , (struct sockaddr *)&server , sizeof(server)) >= 0;

                    return _isconnected;
                }

                void Disconnect()
                {
                    if(!isConnected())
                        return;
                    
                    close(Socket);
                }

                int Recv(uint8_t * buffer, size_t len)
                {
                    //if(!isConnected())
                    //    return -1;
                    //int count;
                   // ioctl(Socket, FIONREAD, &count);

                   // if(!count)
                   //     return 0;

                    return read(Socket, buffer, len);
                }

                int Send(uint8_t * buffer, size_t len)
                {
                    //if(!isConnected())
                    //    return -1;

                    return write(Socket, buffer, len);
                }
        };

        class TCPServer
        {
            private:
                std::vector<void (*) (TCPClient *)> onConnectedHandlers;
                unsigned short _port;
                int _socket_desc;

            public:
                TCPServer()
                {
                    _port = 23;
                }

                TCPServer(unsigned short port)
                {
                    _port = port;
                }

                void AddOnConnectedHandler(void (*handler)(TCPClient *cli))
                {
                    onConnectedHandlers.push_back(handler);
                }

                void RemoveOnConnectedHandler(void (*handler)(TCPClient *cli))
                {
                    for(std::vector<void (*)(TCPClient*)>::iterator it = onConnectedHandlers.begin();
                        it != onConnectedHandlers.end() - 1; ++it)
                    {
                        if(*it == handler)
                            onConnectedHandlers.erase(it);
                    }

                }

                bool Open()
                {
                    int client_sock, read_size;
                    struct sockaddr_in server , client;
                    
                    //Create socket
                    _socket_desc = socket(AF_INET , SOCK_STREAM , 0);
                    if (_socket_desc == -1)
                    {
                        return false;
                    }
                    
                    //Prepare the sockaddr_in structure
                    server.sin_family = AF_INET;
                    server.sin_addr.s_addr = INADDR_ANY;
                    server.sin_port = htons( _port );
                    
                    //Bind
                    if( bind(_socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
                    {
                        perror("HUH?: ");
                        //print the error message
                        return false;
                    }
                    
                    //Listen
                    listen(_socket_desc , 3);

                    return true;
                }

                bool Close()
                {
                    close(_socket_desc);
                }
        };

        class FTPClient
        {

        };

        class FTPServer
        {

        };

        class TFTPClient
        {

        };

        class TFTPServer
        {

        };        
    };

};


using namespace Daan;

#include <stdio.h>
#include <string.h>

int main(int argc, char * argv[])
{
    uint8_t buffer[128];
    std::vector<uint8_t> ip;

    Net::IPAddress *google = new Net::IPAddress("108.177.119.94");
    
    std::cout << "The IP address = " << google->ToString() << std::endl;

    Net::TCPClient tcps = Net::TCPClient(*google, 80);

    //tcps.Connect();
//
    //std::cout <<"Connected ! :)"  <<std::endl;
//
    //printf("Send result %d\r\n", tcps.Send((uint8_t*)"GET / HTTP/1.1\r\n", strlen("GET / HTTP/1.1\r\n")));
    //tcps.Send((uint8_t*)"\r\n", 2);
    //
    //while(tcps.Recv(buffer, sizeof(buffer)) > 0)
    //{
    //    for(int i = 0; i < sizeof(buffer); i++)
    //        printf("%c", buffer[i]);
    //}

    Net::TCPServer tcpserv = Net::TCPServer(23);

    if(tcpserv.Open() == false)
        return 0;

    for(;;)
    {
        sleep(1);
    }
    return 0;
}