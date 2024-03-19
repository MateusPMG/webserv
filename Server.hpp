#pragma once
#include <string>
#include <map>
#include <set>

class Routes{
    std::string directory;
    std::vector<std::string> methods;
    bool auto_index;
    bool hasCGI;
    std::string cgi_path;
    std::string cgi_ext;
    std::string uploadpath;
};

class Server{
	private:
		std::string port;
		std::string host;
		std::string server_name;
		std::string index;
		size_t client_body_size;
		int server_socket_fd;
		std::map<std::string, Routes> routes;
};