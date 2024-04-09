#pragma once
#include "Server.hpp"
#include <ctime>
#include <sys/socket.h>
#include <algorithm>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h> 

class Client{
	private:
		Server target_server;
		int client_socket_fd;
		time_t	previous_request_time;
		std::string request;
		bool sent;
		size_t request_body_size;
		std::string requestmethod;
		std::string requestURI;
		std::string requestbody;
	public:
		Client(Server target, int clientfd);
		int getsocketfd();
		void addRequest(const char* buff, int bufflen);
		bool timeout();
		bool requestready();
		void handleRequest();
		void settarget(Server& target);
		std::string getrequest();
		Server& gettarget();
		void sendErrorResponse(const std::string& error);
		void parseRequest();
		std::map<std::string, std::string> requestheaders;
		void parseRoute(int exit, std::string requestdirectory);
		void handleresponse(std::string rqdir, std::string rquri, const Routes& location);
		void handleget(std::string& rqdir, std::string& rquri, const Routes& location, const std::string& route);
		void sendget(std::string rquri);
};
