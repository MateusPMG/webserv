#pragma once
#include "Server.hpp"

class Client{
	private:
		Server target_server;
		int client_socket_fd;
		time_t	previous_request_time;
	public:
		Client(Server target, int clientfd);
		int getsocketfd();
};