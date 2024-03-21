#pragma once
#include <string>
#include <map>
#include <set>
#include "ConfigParser.hpp"

class Routes{
	public: 
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
		std::string directory;
		std::string error_page_path;
		size_t client_body_size;
		int server_socket_fd;
		std::map<std::string, Routes> routes;
	public:
		std::string findValue(const std::map<std::string, std::string>& myMap, const std::string& word);
		size_t convertToBytes(const std::string& sizeInMB);
		std::map<std::string, Routes> parseRoutes(const std::map<std::string, std::map<std::string, std::set<std::string> > >& routes);
		Server(const ServerConfig& config);
};