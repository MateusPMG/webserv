#include "Client.hpp"

Client::Client(Server target, int clientfd):target_server(target),client_socket_fd(clientfd){}

int Client::getsocketfd(){
	return (client_socket_fd);
}

void Client::addRequest(const char* buff, int bufflen){
	this->previous_request_time = std::time(NULL);
	this->sent = false;
	this->request.append(buff, bufflen);
}