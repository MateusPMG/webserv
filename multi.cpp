#include "Client.hpp"

void Client::parseRoutemulti(int exit, std::string requestdirectory, std::string mrequestbody){
	if (exit >= 10)
		throw std::runtime_error("508 Loop Detected");
	size_t pos;
	std::map<std::string, Routes>::const_iterator route;
	for (route = target_server.getroutes().begin(); route != target_server.getroutes().end(); route++){
		std::cout << route->first << std::endl;
		if (route->first == "/" && requestURI != "/") continue;
		if (requestURI.find(route->first + '/') == std::string::npos
		&& (requestURI.length() < route->first.length()
		|| requestURI.compare(requestURI.length() - route->first.length(), route->first.length(), route->first) != 0))
    		continue;
		pos = requestURI.find(route->first);
		if (!route->second.methods.empty() && 
    	std::find(route->second.methods.begin(), route->second.methods.end(), requestmethod) == route->second.methods.end())
			throw std::runtime_error("405 Method Not Allowed");
		if (route->second.redirection.size()){
			requestURI.erase(pos, route->first.size()).insert(pos, route->second.redirection);
			parseRoutemulti(exit + 1, requestdirectory, requestbody);
			return;
		}
		std::cout << requestURI << "uri1" << std::endl;
		if (route->second.directory.size()){
			requestURI.erase(pos, route->first.size());
			requestdirectory = route->second.directory;
		}
		std::cout << requestURI << "uri2" << std::endl;
		std::cout << requestdirectory + requestURI << " path" << std::endl;
		if (mrequestbody.length() > request_body_size)
			throw std::runtime_error("413 Payload Too Large");
		if (requestmethod.size()){
			if (requestmethod == "GET"){
				handlemultiget(requestdirectory, requestURI, route->second, route->first);
				return;
			}
			else if (requestmethod == "POST"){
				handlemultipost(requestdirectory, requestURI, route->second, route->first);
				return;
			}
			else if (requestmethod == "DELETE"){
				std::cout << "here delete\n";
				handlemultidelete(requestdirectory, requestURI, route->second, route->first);
				return;
			}
		}
	}
	throw std::runtime_error("404 Not Found7");
}

