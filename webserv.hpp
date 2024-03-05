#pragma once

#include <string>


//NETWORKING

//NOTES -> our adress family will be the AF_INET protocol since it is 
//what is used in TCP/IP socket connections and we will be using stream sockets

//offers socket methods for creation and usage of sockets
//aka the doors through which our server receives and sends data
#include <sys/socket.h>
//for ip networking, we will be using the sockaddr_in struct to bind the sockets
//aka to identify/name them by assigning a transport adress(a port number in IP networking)
#include <netinet/in.h>

class NetKing{
    private:
        
    public:

};