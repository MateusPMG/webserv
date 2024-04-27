#pragma once
#include "Server.hpp"
#include <ctime>
#include <sys/socket.h>
#include <algorithm>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

class Client{
	private:
		time_t	previous_request_time;
		std::string request;
		bool sent;
		size_t request_body_size;
		std::string requestmethod;
		std::string requestURI;
		std::string requestbody;
	public:
		Server target_server;
		std::vector <std::string> multibody;
		std::vector <std::string> multiparts;
		int client_socket_fd;
		Client(Server target, int clientfd);
		int getsocketfd();
		void addRequest(const char* buff, int bufflen);
		bool timeout();
		bool requestready();
		int handleRequest();
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
		bool resourceexists(const std::string& rpath);
		bool isdirectory(const std::string& dpath);
		void handledirlist(std::string& rqdir, const std::string& rquri);
		void handletryfile(std::string path);
		void handlepost(std::string& rqdir, std::string& rquri, const Routes& location, const std::string& route);
		void handledelete(std::string& rqdir, std::string& rquri, const Routes& location, const std::string& route);
		std::vector<std::string> multipartrequest(std::string rline, std::string boundary);
		void parsemulti();
		void parseRoutemulti(int exit, std::string requestdirectory, std::string mrequestbody, size_t i);
		void handlemultipost(std::string& rqdir, std::string& rquri ,std::string& rbody, const Routes& location, const std::string& route, size_t i);
		std::string extractfilename(const std::string& body);
		void cgiget();
		void cgipost(std::string rbody);
};
