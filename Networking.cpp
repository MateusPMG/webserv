#include "Networking.hpp"

std::vector<pollfd> Networking::poll_fds;

Networking::Networking(const ConfigParser& parser){
    for (size_t i = 0; i < parser.configData.size(); ++i){
        this->servers.push_back(Server(parser.configData[i]));
    }
}

std::string inttostring(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

Networking::~Networking(){}

void Networking::booting(){
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
        if (bindcreatelisten(*it))
			throw std::runtime_error("");
		std::cout << "Server booted on port: " << (*it).getport() << std::endl;
		std::cout << "Server socket: " << (*it).getsocketfd() << std::endl;
    }
	this->numservers = servers.size();
}

int Networking::bindcreatelisten(Server& server) {
	//create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating server socket");
        return (1);
    }
 	//set socket option to reuse address
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("Error setting socket options");
        return (1);
    }
	//bind socket to port and net interface ip
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server.getport());
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding server socket");
        return (1);
    }
	//start listening on the socket for incoming connections
    if (listen(sockfd, SOMAXCONN) < 0) {
        perror("Error listening on socket");
        return (1);
    }
    //store the socket file descriptor in the Server object
    server.setsocket(sockfd);
    return (0);
}

void Networking::acceptNewConnection(Server& server){
	int socket = accept(server.getsocketfd(), NULL, NULL);
	if (socket == -1)
		throw (std::runtime_error("Error: accepting connection failed"));
	fcntl(socket, F_SETFL, O_NONBLOCK);
	clients.push_back(Client(server, socket));
	struct pollfd pollst;
	pollst.fd = socket;
	pollst.events = POLLIN | POLLOUT;
	pollst.revents = 0;
	poll_fds.push_back(pollst);
}

void Networking::closeConnection(int pollindex, int clientindex){
	close(poll_fds[pollindex].fd);
	poll_fds.erase(poll_fds.begin() + pollindex);
	clients.erase(clients.begin() + clientindex);
	std::cout << "Connection closed" << std::endl;
}

void Networking::receiveRequest(int clientindex, int fd, int pollindex){
	char buff[4096] = {0};
	int read_byte_n;
	//when recv returns 0 it means the connection has been closed on the client end so we close as well
	if ((read_byte_n = recv(fd, buff, 4096, 0)) > 0){
		clients[clientindex].addRequest(buff, read_byte_n);
	}
	else if (read_byte_n == 0)
		closeConnection(pollindex, clientindex);
	else if (read_byte_n == -1){
		throw std::runtime_error("500 Internal Server Error");
		closeConnection(pollindex, clientindex);
	}

}

Server& Networking::checktarget(const std::string& buff, Server& defaultserv){
    (void)defaultserv;
	size_t hostPos = buff.find("Host:");
	std::string servname;
    if (hostPos != std::string::npos) {
        std::istringstream iss(buff.substr(hostPos));
        std::string hostLine;
        std::getline(iss, hostLine);
        size_t hostStartPos = hostLine.find(":") + 2;
        servname = hostLine.substr(hostStartPos);
        size_t endPos = servname.find("\r");
        if (endPos != std::string::npos) {
            servname = servname.substr(0, endPos);
        }
	}
	if (servname.find(":") != std::string::npos){
		if (servname.substr(0, servname.find(":")) == "localhost"){
			servname = "127.0.0.1" + servname.substr(servname.find(":"));
		}
		for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it){
 			if (servname.substr(0, servname.find(":")) == it->gethost() && servname.substr(servname.find(":") + 1) == inttostring(it->getport())){
				std::cout << "made it here\n";
				return (*it);
			}
		}
	}
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it){
		if (servname == it->getservername())
			return (*it);
	}
	throw (std::runtime_error("Error: Host: not found"));
}

void Networking::runservers(){
	//initialize poll_fds vector with the number of existing servers
	for (size_t i = 0; i < servers.size(); i++) {
		poll_fds.push_back(pollfd());
	}
	//fill pollfds vector with the required information
	for (size_t i = 0; i < servers.size(); i++) {
		poll_fds[i].fd = servers[i].getsocketfd();
		//pollin will indicate we are interested in monitoring the fd for READ events
		poll_fds[i].events = POLLIN;
		//revents set to 0, the poll() function will change this indicating which event happened on the fd
		poll_fds[i].revents = 0;
	}
	//MAIN LOOP, this loop is basically the webserv running
	//handles events and monitors connections
	while(true){
		if (poll(poll_fds.data(), poll_fds.size(), 200) == -1){
        	throw std::runtime_error("Error: poll failed");
		}
		for (size_t i = 0; i < poll_fds.size(); i++){
			int clientindex = i - numservers;
			//check if pollin BIT was set in the revent variable, 
			//if so it means theres is data available to be read on a socket
			if (poll_fds[i].revents & POLLIN){
				//we must first check if the current i corresponds to a server fd
				//which means a new connection is ready to be accepted, otherwise its a client socket
				//and theres data to be read from the client
				if (i < servers.size() && poll_fds[i].fd == servers[i].getsocketfd()){
					acceptNewConnection(servers[i]);
					//accept new connection and then skip to the next iteration
					continue;
				}
				//will read and store the request for future parsing and if not closes connection
				receiveRequest(clientindex, poll_fds[i].fd, i);
				continue;
			}
			//if any error with fds or the connection is closed on client end we close the connection
			if (poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL | POLLRDHUP)){
				closeConnection(i, clientindex);
			}
			//due to not using else if conditions we must stop server sockets from being further processed
			if (poll_fds[i].fd <= servers[numservers - 1].getsocketfd())
				continue;
			//check if the socket is ready to write into and if client object is ready to send the response
			if (poll_fds[i].revents & POLLOUT){
				 //need to check if client timed out, if true we close connection
				if (clients[clientindex].timeout()){
					closeConnection(i, clientindex);
					continue;
				}
				//check if we appended the last request chunk if not
				//it means the client object hasnt received the whole request yet and we continue
				if (!clients[clientindex].requestready() && clients[clientindex].multiparts.empty())
					continue;
				//if we reach this point we parse and handle the request to build a response
				//but first we must check if the request sent is for the server who accepted the connection or another server
				try
				{
					clients[clientindex].settarget(checktarget(clients[clientindex].getrequest(), clients[clientindex].gettarget()));
					if (clients[clientindex].handleRequest() == 1)
						closeConnection(i, clientindex);
				}
				catch(const std::string& e)
				{
					std::cout << "omg im an idiot" << std::endl;
					std::stringstream response;
					size_t spacePos = e.find(' ');
					std::string errorCode = e.substr(0, spacePos);
					std::string errorpage = clients[clientindex].target_server.geterrorpage();
					if (!errorpage.empty()){
						size_t codepos = errorpage.find('.');
						std::string pagecode = errorpage.substr(0, codepos);
						if (pagecode == errorCode){
							std::string errorpagepath = clients[clientindex].target_server.getdirectory() + "/" + clients[clientindex].target_server.geterrorpage();
							std::cout << errorpagepath << "= er page path" << std::endl;
							std::ifstream html_file(errorpagepath.c_str());
							if (html_file.is_open()) {
								std::stringstream buffer;
								buffer << html_file.rdbuf();
								std::string html_content = buffer.str();
								html_file.close();
								std::string http_response = "HTTP/1.1 " + errorCode + "\r\n";
								http_response += "Content-Type: text/html\r\n";
								http_response += "Content-Length: " + inttostring(html_content.size()) + "\r\n";
								http_response += "\r\n";
								http_response += html_content;
								if (send(clients[clientindex].client_socket_fd, http_response.c_str(), http_response.size(), 0) <= 0)
									throw std::runtime_error("500 Internal Server Error");
								return;
							}
							std::cout << "didnt enter" << std::endl;
						}
					}
					std::string responses =
						"HTTP/1.1 404 Not Found\r\n"
						"Content-Type: text/html\r\n"
						"\r\n"
						"<!DOCTYPE html>\r\n"
						"<html>\r\n"
						"<head><title>404 Not Found</title></head>\r\n"
						"<body>\r\n"
						"<h1>404 Not Found</h1>\r\n"
						"<p>The requested resource could not be found.</p>\r\n"
						"</body>\r\n"
						"</html>\r\n";
					if (send(clients[clientindex].client_socket_fd, responses.c_str(), responses.length(), 0) <= 0)
						throw std::runtime_error("500 Internal Server Error");
					std::cerr << e << '\n';
					closeConnection(i, clientindex);
				}
			}
		}
	}
}
