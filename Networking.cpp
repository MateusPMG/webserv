#include "Networking.hpp"

Networking::Networking(const ConfigParser& parser){
		
}

Networking::~Networking(){}

int Networking::CreateBindSocket(int port){
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1){
		std::cerr << "Error: Failed to create socket for port " << port << std::endl;
		return (-1);
	}
	int reuseadd = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseadd, sizeof(reuseadd)) == -1){
		std::cerr << "Error: Setting socket options failed\n";
		close (serverSocket);
		return (-1);
	}
	struct sockaddr_in serverAddress;
	std::memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = inet_addr(INADDR_ANY);
	if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1){
		std::cerr << "Error: Failed to bind socket to port " << port << std::endl;
		close (serverSocket);
		return (-1);
	}
	return (serverSocket);
}

//main server loop
void Networking::runServer(){
	while (true){
	// Handle incoming connections and other I/O events
			// This is where you would use select() or poll() to handle multiple sockets concurrently
			// Example:
			// - Check for new incoming connections on server sockets
			// - Check for data available to read on connected sockets
			// - Check for sockets ready for writing
	}
}
