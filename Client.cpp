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

void Client::sendErrorResponse(const std::string& error){
	std::stringstream response;
    //Extracting error code and reason from the input string
    size_t spacePos = error.find(' ');
    std::string errorCode = error.substr(0, spacePos);
    std::string errorReason = error.substr(spacePos + 1);
    //Constructing the HTML response
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
	//sending html error response to the client 
	std::string responseStr = response.str();
	send(client_socket_fd, responseStr.c_str(), responseStr.length(), 0);
}

void Client::parseRequest(){
	std::ifstream requeststream(request.c_str());
	std::string line;
	std::getline(requeststream, line);
	std::stringstream linestream(line);
	std::string method;
	std::string URI;
	std::string httpversion;
	linestream >> method >> URI >> httpversion;
	if (method.empty() || URI.empty() || httpversion.empty()){
		throw std::runtime_error("400 Bad Request");
	}
	if (!(method == "GET" || method == "POST" || method == "DELETE")){
		throw std::runtime_error("501 Not Implemented");
	}
	requestmethod = method;
	if (httpversion == "HTTP/1.0"){
		throw std::runtime_error("505 HTTP Version Not Supported");
	}
	if (httpversion != "HTTP/1.1"){
		throw std::runtime_error("400 Bad Request");
	}
	if (URI.empty() || URI[0] != '/' || URI.find("../") != std::string::npos){
		throw std::runtime_error("400 Bad Request");
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
	//generally GET and DELETE methods dont have a body in their request
	if (method == "GET" || method == "DELETE")
		return;
	//lets check body size
	std::map<std::string, std::string>::iterator it = requestheaders.find("Content-length");
	if (it != requestheaders.end()) {
		std::istringstream iss(it->second);
		if (!(iss >> request_body_size)) {
			throw std::runtime_error("400 Bad Request");
		}
	} else {
		throw std::runtime_error("400 Bad Request");
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

void Client::parseRoute(int exit){
	if (exit >= 10)
		throw std::runtime_error("508 Loop Detected");
	
}

void Client::handleRequest(){
	sent = true;
	previous_request_time = std::time(NULL);
	try
	{
		parseRequest();
		parseRoute(0);
	}
	catch(const std::exception& e)
	{
		sendErrorResponse(e.what());
	}
}
