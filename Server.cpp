#include "Server.hpp"

std::string Server::findValue(const std::map<std::string, std::string>& myMap, const std::string& word) {
    std::map<std::string, std::string>::const_iterator it;
    for (it = myMap.begin(); it != myMap.end(); ++it) {
        if (it->first == word) {
            return it->second;
        }
    }
    return ""; //Return an empty string if the word is not found tho it will never happen this far ahead in the code
}

size_t convertToBytes(const std::string& sizeInMB) {
    std::istringstream iss(sizeInMB);
    double size;
    if (!(iss >> size)) {
        //Parsing failed
        return 0;
    }
    //Convert size to bytes
    size_t bytes = static_cast<size_t>(size * 1024 * 1024);
    return bytes;
}

Server::Server(const ServerConfig& config){
	port = findValue(config.serverConfig, "port");
	host = findValue(config.serverConfig, "host");
	server_name = findValue(config.serverConfig, "server_name");
	index = findValue(config.serverConfig, "index");
	directory = findValue(config.serverConfig, "directory");
	error_page_path = findValue(config.serverConfig, "error_page");
	client_body_size = convertToBytes(findValue(config.serverConfig, "client_body_size"));
	
}