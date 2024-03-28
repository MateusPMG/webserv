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
	//90 seconds for a time out randomly chosen
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
	std::ifstream requeststream(request);
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
	if (httpversion == "HTTP/1.0"){
		throw std::runtime_error("505 HTTP Version Not Supported");
	}
	if (httpversion != "HTTP/1.1"){
		throw std::runtime_error("400 Bad Request");
	}
	if (URI.empty() || URI[0] != '/' || URI.find("../") != std::string::npos){
		throw std::runtime_error("400 Bad Request");
	}

}

void Client::handleRequest(){
	sent = true;
	previous_request_time = std::time(NULL);
	try
	{
		parseRequest();
	}
	catch(const std::exception& e)
	{
		sendErrorResponse(e.what());
	}
}