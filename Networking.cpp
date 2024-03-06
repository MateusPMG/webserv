#include "Networking.hpp"

Networking::Networking(){}

Networking::~Networking(){
    for (std::map<std::pair<std::string, int>, int>::iterator it = serverSockets.begin(); it != serverSockets.end(); it++){
        close(it->second);
    }
}

int Networking::CreateBindSocket(const char* ip, int port){
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1){
        std::cerr << "Error: Failed to create socket for port " << port << std::endl;
        return (-1);
    }
    int bounce = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &bounce, sizeof(bounce)) == -1){
        std::cerr << "Error: Setting socket options failed\n";
        close (serverSocket);
        return (-1);
    }
    struct sockaddr_in serverAddress;
    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1){
        std::cerr << "Error: Failed to bind socket to port " << port << std::endl;
        close (serverSocket);
        return (-1);
    }
    return (serverSocket);
}

bool Networking::addPort(const char* ip, int port){
    int serverSocket = CreateBindSocket(ip, port);
    if (serverSocket != -1){
        serverSockets[std::make_pair(std::string(ip), port)] = serverSocket;
        return (true);
    }
    return (false);
}

bool Networking::startListening(){
    for (std::map<std::pair<std::string, int>, int>::iterator it = serverSockets.begin(); it != serverSockets.end(); it++){
        if (listen(it->second, SOMAXCONN) == -1){
            std::cerr << "Error: Failed to start listening on port " << it->first.second << std::endl;
            return false;
        }
        std::cout << "Server listening on port " << it->first.second << std::endl;   
    }
    return (true);
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