#include "Server.hpp"

std::string Server::findValue(const std::map<std::string, std::string>& myMap, const std::string& word) {
    std::map<std::string, std::string>::const_iterator it;
    for (it = myMap.begin(); it != myMap.end(); ++it) {
        if (it->first == word) {
            return it->second;
        }
    }
    return ""; //Return empty string if fail however previous parsing should prevent this 
}

size_t convertToBytes(const std::string& sizeInMB) {
    std::istringstream iss(sizeInMB);
    double size;
    if (!(iss >> size)) {
        //Parsing failed, should never happen tho due to previous parsing
        return 0;
    }
    //Convert size to bytes
    size_t bytes = static_cast<size_t>(size * 1024 * 1024);
    return bytes;
}

std::map<std::string, Routes> Server::parseRoutes(const std::map<std::string, std::map<std::string, std::set<std::string> > >& routes) {
    std::map<std::string, Routes> parsedRoutes;
    std::map<std::string, std::map<std::string, std::set<std::string> > >::const_iterator it;
    for (it = routes.begin(); it != routes.end(); ++it) {
        Routes route;
        std::map<std::string, std::set<std::string> >::const_iterator innerIt;
        innerIt = it->second.find("directory");
        route.directory = (innerIt != it->second.end() && innerIt->second.size() > 0) ? *(innerIt->second.begin()) : "";
        innerIt = it->second.find("auto_index");
        route.auto_index = (innerIt != it->second.end() && innerIt->second.size() > 0) ? (*(innerIt->second.begin()) == "on") : false;
        innerIt = it->second.find("methods");
        if (innerIt != it->second.end() && innerIt->second.size() > 0) {
            route.methods.assign(innerIt->second.begin(), innerIt->second.end());
        }
        innerIt = it->second.find("cgi");
        route.hasCGI = (innerIt != it->second.end() && innerIt->second.size() > 0);
        if (route.hasCGI) {
            innerIt = it->second.find("cgi_path");
            route.cgi_path = (innerIt != it->second.end() && innerIt->second.size() > 0) ? *(innerIt->second.begin()) : "";
            innerIt = it->second.find("cgi");
            route.cgi_ext = (innerIt != it->second.end() && innerIt->second.size() > 0) ? *(innerIt->second.begin()) : "";
        }
        innerIt = it->second.find("upload_dir");
        route.uploadpath = (innerIt != it->second.end() && innerIt->second.size() > 0) ? *(innerIt->second.begin()) : "";
        //Store 'route' object in 'parsedRoutes' map
        parsedRoutes[it->first] = route;
    }
    return parsedRoutes;
}


Server::Server(const ServerConfig& config){
	port = findValue(config.serverConfig, "port");
	host = findValue(config.serverConfig, "host");
	server_name = findValue(config.serverConfig, "server_name");
	index = findValue(config.serverConfig, "index");
	directory = findValue(config.serverConfig, "directory");
	error_page_path = findValue(config.serverConfig, "error_page");
	client_body_size = convertToBytes(findValue(config.serverConfig, "client_body_size"));
	routes = parseRoutes(config.routes);
	
}