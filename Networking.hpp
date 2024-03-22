#pragma once

#include <string>
#include "Server.hpp"
#include "ConfigParser.hpp"

//NETWORKING

//NOTES -> our adress family will be the AF_INET protocol since it is 
//what is used in TCP/IP socket connections and we will be using stream sockets

//offers socket methods for creation and usage of sockets
//aka the doors through which our server receives and sends data
#include <sys/socket.h>
//for ip networking, we will be using the sockaddr_in struct to bind the sockets
//aka to identify/name them by assigning a transport adress(a port number in IP networking)
#include <netinet/in.h>
//defines the close() function used to close the sockets in the destructor
#include <unistd.h>
//vector to store socket fds for each server
#include <vector>
//printing and streams
#include <iostream>
//included for memset() function to reset struct values to 0 thus clearing trash values
#include <cstring>
//included for the inet_addr() function to set the IP adress of the server
//it converts the ip adress from string to network byte order 
#include <arpa/inet.h>
//pollfds and poll function to monitor server and client sockets and allow multiple connections without blocking
#include <poll.h>
class Networking{
	private:
		//this vector will store the servers and their configs;
		std::vector<Server> servers;
		static std::vector<pollfd> poll_fds;
	public:
		//constructor and destructor
		Networking(const ConfigParser& parser);
		~Networking();
		//starts servers by creating and binding sockets for each server and starts listening on each
		void booting();
		//create socket, bind it, store it in server object and listen on it
		int bindcreatelisten(Server& server);
		//this method will start the server and run it
		//on a while(true) statement
		void runservers();
};
