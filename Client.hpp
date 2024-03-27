#pragma once
#include "Server.hpp"
#include <ctime>
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
};