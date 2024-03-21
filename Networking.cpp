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
