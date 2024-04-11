#include "Client.hpp"

Client::Client(Server target, int clientfd):target_server(target),client_socket_fd(clientfd){}

int Client::getsocketfd(){
	return (client_socket_fd);
}

Server& Client::gettarget(){
	return (target_server);
}

std::string Client::getrequest(){
	return (request);
}

std::string intToString(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

void Client::addRequest(const char* buff, int bufflen){
	this->previous_request_time = std::time(NULL);
	this->sent = false;
	this->request.append(buff, bufflen);
}

bool Client::timeout(){
	//90 seconds for a time out, randomly chosen
	return (std::time(NULL) - previous_request_time > 90);
}

bool Client::requestready(){
	//if the request hasnt been sent AND the delimiter has been found 
	//the request string is ready to be handled
	return !(sent || request.find("\r\n\r\n") == std::string::npos);
}

void Client::settarget(Server& target){
	target_server = target;
}

bool Client::resourceexists(const std::string& rpath){
	return (access(rpath.c_str(), F_OK) == 0);
}

bool Client::isdirectory(const std::string& dpath){
	struct stat s;

	if (stat(dpath.c_str(), &s) == 0) {
		if (s.st_mode & S_IFDIR) {
			return true;
		}
	}
	return false;
}

void Client::sendErrorResponse(const std::string& error){
	std::stringstream response;
    size_t spacePos = error.find(' ');
    std::string errorCode = error.substr(0, spacePos);
    std::string errorReason = error.substr(spacePos + 1);
    response << "HTTP/1.1 " << errorCode << " " << errorReason << "\r\n";
    response << "Content-Type: text/html\r\n\r\n";
    response << "<!DOCTYPE html>\r\n";
    response << "<html>\r\n";
    response << "<head><title>" << errorCode << " " << errorReason << "</title></head>\r\n";
    response << "<body>\r\n";
    response << "<h1>" << errorCode << " " << errorReason << "</h1>\r\n";
    response << "<p>" << errorReason << "</p>\r\n";
    response << "</body>\r\n";
    response << "</html>\r\n"; 
	std::string responseStr = response.str();
	send(client_socket_fd, responseStr.c_str(), responseStr.length(), 0);
}

void Client::parseRequest(){
	std::cout << request << std::endl;
	std::stringstream requeststream(request.c_str());
	std::string line;
	std::getline(requeststream, line);
	std::cout << line << std::endl;
	std::stringstream linestream(line);
	std::string method;
	std::string URI;
	std::string httpversion;
	linestream >> method >> URI >> httpversion;
	if (method.empty() || URI.empty() || httpversion.empty()){
		throw std::runtime_error("400 Bad Request1");
	}
	if (!(method == "GET" || method == "POST" || method == "DELETE")){
		throw std::runtime_error("501 Not Implemented");
	}
	requestmethod = method;
	if (httpversion == "HTTP/1.0"){
		throw std::runtime_error("505 HTTP Version Not Supported");
	}
	if (httpversion != "HTTP/1.1"){
		throw std::runtime_error("400 Bad Request2");
	}
	if (URI.empty() || URI[0] != '/' || URI.find("../") != std::string::npos){
		throw std::runtime_error("400 Bad Request3");
	}
	requestURI = URI;
	//the "\r" signals the end of the headers and the start of the body of the request/chunk
	//here we store the headers
	while (std::getline(requeststream, line) && line != "\r"){
		std::stringstream linestream1(line);
		std::string headerName;
		std::string headerValue;
		std::getline(linestream1, headerName, ':');
		std::getline(linestream1, headerValue);
		std::size_t first = headerValue.find_first_not_of(' ');
		if (first != std::string::npos)
			headerValue.erase(0, first);
		std::size_t last = headerValue.find_last_not_of(' ');
		if (last != std::string::npos)
			headerValue.erase(last + 1);
		if (headerValue.empty()){
			throw std::runtime_error("400 Bad Request");
		}
		requestheaders[headerName] = headerValue;
	}
	//generally GET and DELETE methods dont have a body in their request so we skip the rest
	if (method == "GET" || method == "DELETE")
		return;
	//lets check body size
	std::map<std::string, std::string>::iterator it = requestheaders.find("Content-length");
	if (it != requestheaders.end()) {
		std::istringstream iss(it->second);
		if (!(iss >> request_body_size)) {
			throw std::runtime_error("400 Bad Request4");
		}
	} else {
		throw std::runtime_error("400 Bad Request5");
	}
	if (request_body_size > target_server.getclientbodysize()){
		throw std::runtime_error("413 Payload Too Large");
	}
	//skip over blank line after headers
	std::getline(requeststream, line);
	//since there might be images or executables sent we dont want to corrupt them
	//thus we dont want to interpret any character in the request body by reading it as text
	//we just want to store it so we will be treating it as binary so no char will be interpreted
	//Resize requestBody to request_body_size
	requestbody.resize(request_body_size);
	//Read data from requeststream directly into requestbody without any interpretation
	requeststream.read(&requestbody[0], request_body_size);
}

void Client::handledirlist(std::string& rqdir, std::string& rquri) {
    std::string response;
    DIR* dir = opendir(rqdir.c_str());
    if (dir != NULL) {
        struct dirent* entry;
        response += "<html><head><title>Directory Listing</title></head><body><h1>Directory Listing</h1><ul>";
        while ((entry = readdir(dir)) != NULL) {
            std::string filename = entry->d_name;
            if (filename == "." || filename == "..")
                continue;
            struct stat fileStat;
            std::string filepath = rqdir + "/" + filename;
            if (stat(filepath.c_str(), &fileStat) == 0) {
                //Append filename to the response
                response += "<li><a href=\"" + rquri + "/" + filename + "\">" + filename + "</a></li>";
            }
        }
        response += "</ul></body></html>";
        closedir(dir);
    } else {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
    }
    std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + intToString(response.length()) + "\r\n\r\n" + response;
    send(client_socket_fd, httpResponse.c_str(), httpResponse.length(), 0);
	return;
}

void Client::handleget(std::string& rqdir, std::string& rquri, const Routes& location, const std::string& route) {
	std::string path = rqdir + rquri;
	std::string trypath = rqdir + location.tryfile;
	if(route == "/cgi-bin")
	{
		if (!rquri.empty() || rquri == "/") {
			throw std::runtime_error("401 Unauthorized");
		}
		//cgi handler
	}
	
	std::cout << path << "1" << std::endl;
	std::cout << rqdir <<  "2" <<std::endl;
	std::cout << rquri << "3" <<std::endl;
	std::cout << location.tryfile << "4" << std::endl;
	if (!resourceexists(path))
		throw std::runtime_error("404 Not Found6");
	//remove trailing slash "/" if it exists
	if (path != "./" && path[path.length() - 1] == '/') {
		path = path.substr(0, path.length() - 1);
	}
	//check if its a directory
	if (isdirectory(path)){
		std::cout << trypath << std::endl;
		if (!location.tryfile.empty() && path == rqdir
		&& resourceexists(trypath) && !isdirectory(trypath)){
			//handletryfile(trypath);
			std::cout << "here1" << std::endl;
			return;
		}
		if (!location.auto_index){
			std::cout << "here2" << std::endl;
			throw std::runtime_error("403 Forbidden");
		}
		else {
			std::cout << "here3" << std::endl;
			handledirlist(rqdir, rquri);
			return;
		}
	}
	else{
		std::cout << "here4" << std::endl;
		//handletryfile();
	}
}

void Client::sendget(std::string rquri){
   (void)rquri;
}

void Client::parseRoute(int exit, std::string requestdirectory){
	if (exit >= 10)
		throw std::runtime_error("508 Loop Detected");
	std::cout << requestmethod << std::endl;
	size_t pos;
	std::map<std::string, Routes>::const_iterator route;
	for (route = target_server.getroutes().begin(); route != target_server.getroutes().end(); route++){
		std::cout << route->first << std::endl;
		if (route->first == "/" && requestURI != "/") continue;
		//if the route in the iterator is not part of a longer path in the URI and the URI
		//doesnt end with an exact match of route then its not this route
		if (requestURI.find(route->first + '/') == std::string::npos
		&& (requestURI.length() < route->first.length()
		|| requestURI.compare(requestURI.length() - route->first.length(), route->first.length(), route->first) != 0))
    		continue;
		//route position in the uri string
		pos = requestURI.find(route->first);
		//check if the route supports the requested method
		if (!route->second.methods.empty() && 
    	std::find(route->second.methods.begin(), route->second.methods.end(), requestmethod) == route->second.methods.end())
			throw std::runtime_error("405 Method Not Allowed");
		//if theres a redirection simply process the request in the route its redirected to
		if (route->second.redirection.size()){
			requestURI.erase(pos, route->first.size()).insert(pos, route->second.redirection);
			parseRoute(exit + 1, requestdirectory);
			return;
		}
		std::cout << requestURI << "uri1" << std::endl;
		//prepare URI for further handling since we already established the correct route
		if (route->second.directory.size()){
			requestURI.erase(pos, route->first.size());
			requestdirectory = route->second.directory;
		}
		std::cout << requestURI << "uri2" << std::endl;
		if (requestbody.length() > request_body_size)
			throw std::runtime_error("413 Payload Too Large");
		if (requestmethod.size()){
			if (requestmethod == "GET"){
				handleget(requestdirectory, requestURI, route->second, route->first);
				return;
			}
			else if (requestmethod == "POST"){
				//handlepost();
				return;
			}
			else if (requestmethod == "DELETE"){
				//handledelete();
				return;
			}
		}
	}
	throw std::runtime_error("404 Not Found7");
}

void Client::handleRequest(){
	sent = true;
	previous_request_time = std::time(NULL);
	try
	{
		parseRequest();
		std::string dir = target_server.getdirectory();
		parseRoute(0, dir);
	}
	catch(const std::exception& e)
	{
		sendErrorResponse(e.what());

	}
	request.clear();
	requestbody.clear();
}
