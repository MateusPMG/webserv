#pragma once
#include "Server.hpp"
#include <ctime>
#include <sys/socket.h>
#include <sstream>
class Client{
	private:
		Server target_server;
		int client_socket_fd;
		time_t	previous_request_time;
		std::string request;
		bool sent;
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
};