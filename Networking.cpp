#include "Networking.hpp"

Networking::Networking(const ConfigParser& parser){
    for (size_t i = 0; i < parser.configData.size(); ++i){
        this->servers.push_back(Server(parser.configData[i]));
    }
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
	else
		closeConnection(pollindex, clientindex);
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
				if ()
					closeConnection(i, clientindex);
				//check if we appended the last request chunk if not
				//it means the client object hasnt received the whole request yet and we continue
				if ()
					continue;
				
			}
		}
	}
}
