#include "Client.hpp"
#include "Networking.hpp"

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

std::string extractboundary(const std::string& requestData) {
    std::string boundary;
    size_t boundaryStart = requestData.find("boundary=");
    if (boundaryStart != std::string::npos) {
        boundaryStart += 9;
        size_t boundaryEnd = requestData.find("\r\n", boundaryStart);
        if (boundaryEnd != std::string::npos) {
            boundary = requestData.substr(boundaryStart, boundaryEnd - boundaryStart);
        }
    }
    return boundary;
}

std::vector<std::string> Client::multipartrequest(std::string rline, std::string boundary){
	std::vector<std::string> parts;
	std::string firstheader;
	size_t boundarypos = rline.find("--" + boundary);
	std::cout << boundarypos << "= first boundary post" << std::endl;
	firstheader = rline.substr(0, boundarypos - 1);
	std::cout << firstheader << "= first header" <<std::endl;
	std::istringstream headerStream(firstheader);
    std::string method, URI, httpversion;
    headerStream >> method >> URI >> httpversion;
    std::cout << "Method: " << method << std::endl;
    std::cout << "URI: " << URI << std::endl;
    std::cout << "HTTP Version: " << httpversion << std::endl;
	requestmethod = method;
	requestURI = URI;
	if (!firstheader.empty()){	
		size_t nextboundarypos;
		while(boundarypos != std::string::npos){
			nextboundarypos = rline.find("--" + boundary, boundarypos + boundary.length() - 3);
			if (nextboundarypos == std::string::npos)
				break;
			std::cout << nextboundarypos << "= current boundary pos" << std::endl;
			parts.push_back(rline.substr(boundarypos + boundary.length() + 3, nextboundarypos - boundarypos - boundary.length() - 4));
			//
			boundarypos = nextboundarypos;
		}
	}
	for (size_t i = 0; i < parts.size(); ++i) {
		std::istringstream partStream(parts[i]);
		std::string line;
		while (std::getline(partStream, line) && !line.empty()) {
		}
		std::string requestBody;
		while (std::getline(partStream, line)) {
			requestBody += line + "\n";
		}
		std::cout << "Request body: " << requestBody ;
		multibody.push_back(requestBody);
	}
	std::cout << " finished parsing parts" << std::endl;
	return parts;
}

void Client::addRequest(const char* buff, int bufflen){
	(void) bufflen;
	this->previous_request_time = std::time(NULL);
	this->sent = false;
	std::string rline (buff);
	std::string contentTypeHeader = "Content-Type: multipart/form-data";
	if (rline.find(contentTypeHeader) != std::string::npos){
		std::string boundary = extractboundary(rline);
		multiparts = multipartrequest(rline, boundary);
		std::cout << " exited multi function" << std::endl;
	}
	this->request.append(rline);
	return;
}

void Client::parsemulti(){

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

bool deletedirectory(const char* path){
	DIR* dir = opendir(path);
    if (dir == NULL) {
        return false;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            std::string filePath = std::string(path) + "/" + entry->d_name;

            if (entry->d_type == DT_DIR) {
                if (!deletedirectory(filePath.c_str()))
                    return false;
            } else {
                if (remove(filePath.c_str()) != 0) {
                    return false;
                }
            }
        }
    }
    closedir(dir);
    if (rmdir(path) != 0) {
        return false;
    }
    return true;
}

void Client::sendErrorResponse(const std::string& error){
	std::stringstream response;
    size_t spacePos = error.find(' ');
    std::string errorCode = error.substr(0, spacePos);
	std::string errorpage = target_server.geterrorpage();
	if (!errorpage.empty()){
		size_t codepos = errorpage.find('.');
		std::string pagecode = errorpage.substr(0, codepos);
		if (pagecode == errorCode){
			std::string errorpagepath = target_server.getdirectory() + "/" + target_server.geterrorpage();
			std::cout << errorpagepath << "= er page path" << std::endl;
			std::ifstream html_file(errorpagepath.c_str());
			if (html_file.is_open()) {
				std::stringstream buffer;
				buffer << html_file.rdbuf();
				std::string html_content = buffer.str();
				html_file.close();
				std::string http_response = "HTTP/1.1 " + errorCode + "\r\n";
				http_response += "Content-Type: text/html\r\n";
				http_response += "Content-Length: " + intToString(html_content.size()) + "\r\n";
				http_response += "\r\n";
				http_response += html_content;
				if (send(client_socket_fd, http_response.c_str(), http_response.size(), 0) <= 0)
					return;
				return;
			}
			std::cout << "didnt enter" << std::endl;
		}
	}
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
	if (send(client_socket_fd, responseStr.c_str(), responseStr.length(), 0) <= 0)
		return; //closes the connection immediately
}

void Client::parseRequest(){
	std::stringstream requeststream(request.c_str());
	std::string line;
	std::getline(requeststream, line);
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
	while (std::getline(requeststream, line) && (line != "\r" && line != "\r\n" && !line.empty())){
		std::stringstream linestream1(line);
		std::string headerName;
		std::string headerValue;
		std::getline(linestream1, headerName, ':');
		std::getline(linestream1, headerValue);
		std::cout << "header=" << headerName << "/ value=" << headerValue << std::endl;
		std::size_t first = headerValue.find_first_not_of(' ');
		if (first != std::string::npos)
			headerValue.erase(0, first);
		std::size_t last = headerValue.find_last_not_of(' ');
		if (last != std::string::npos)
			headerValue.erase(last + 1);
		if (headerValue.empty()){
			std::cout << "contains=" << line << std::endl;
			throw std::runtime_error("400 Bad Request92");
		}
		requestheaders[headerName] = headerValue;
	}
	if (line.empty())
		std::getline(requeststream, line);
	std::cout << "1\n";
	//generally GET and DELETE methods dont have a body in their request so we skip the rest
	if (method == "GET" || method == "DELETE")
		return;
	//lets check body size
	std::map<std::string, std::string>::iterator it = requestheaders.find("Content-Length");
	if (it != requestheaders.end()) {
		std::istringstream iss(it->second);
		if (!(iss >> request_body_size)) {
			throw std::runtime_error("400 Bad Request4");
		}
	} else {
		throw std::runtime_error("400 Bad Request5");
	}
	std::cout << "1\n";
	if (request_body_size > target_server.getclientbodysize()){
		throw std::runtime_error("413 Payload Too Large");
	}
	std::cout << line << " line " << std::endl;
	std::cout << request_body_size << std::endl;
	requestbody.resize(request_body_size);
	char* requestBodyPtr = &requestbody[0]; 
	int bytesRead = 0;
	std::cout << "1\n";
	while (static_cast<size_t>(bytesRead) < request_body_size) {
		requeststream.read(requestBodyPtr + bytesRead, request_body_size - bytesRead);
		int bytesJustRead = requeststream.gcount();
		if (bytesJustRead == 0) {
			std::cout << " failed wellp\n";
			// If no bytes were read, and the expected bytes haven't been read yet, it's an error
			throw std::runtime_error("Incomplete request body");
		}
		bytesRead += bytesJustRead;
	}
	std::cout << " oi\n";
	std::cout << requestbody << "parserequestbody" << std::endl;
}

void Client::handletryfile(std::string path) {
    std::ifstream fileStream(path.c_str(), std::ios::binary);
    if (!fileStream.is_open()) {
        return;
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    std::string fileContent = buffer.str();
    std::string httpResponse = "HTTP/1.1 200 OK\r\n";
    httpResponse += "Content-Type: text/html\r\n";
    httpResponse += "Content-Length: " + intToString(fileContent.length()) + "\r\n\r\n";
    httpResponse += fileContent;
    send(client_socket_fd, httpResponse.c_str(), httpResponse.length(), 0);
	return;
}

void Client::handledirlist(std::string& rqdir, std::string& rquri) {
    (void)rquri;
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
                // Append filename to the response without creating a link
                response += "<li>" + filename + "</li>";
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
	std::string trypath = path + "/" + location.tryfile;
	if(route == "/cgi-bin")
	{
		if (!rquri.empty() || rquri == "/") {
			throw std::runtime_error("401 Unauthorized");
		}
		//cgi handler
	}
	if (!resourceexists(path))
		throw std::runtime_error("404 Not Found6");
	//remove trailing slash "/" if it exists
	if (path != "./" && path[path.length() - 1] == '/') {
		path = path.substr(0, path.length() - 1);
	}
	std::cout << path << "1" << std::endl;
	//check if its a directory
	if (isdirectory(path)){
		std::cout << trypath << std::endl;
		if (!location.tryfile.empty()
		&& resourceexists(trypath) && !isdirectory(trypath)){
			handletryfile(trypath);
			std::cout << "here1" << std::endl;
			return;
		}
		if (!location.auto_index){
			std::cout << "here2" << std::endl;
			throw std::runtime_error("403 Forbidden");
		}
		else {
			std::cout << "here3" << std::endl;
			handledirlist(path, rquri);
			return;
		}
	}
	else{
		std::cout << "here4" << std::endl;
		handletryfile(trypath);
	}
}

void Client::handlepost(std::string& rqdir, std::string& rquri, const Routes& location, const std::string& route){
	(void)rqdir;
	if(route == "/cgi-bin")
	{
		if (!rquri.empty() || rquri == "/") {
			throw std::runtime_error("401 Unauthorized");
		}
		//cgi handler
		return;
	}
	if (location.uploadpath.empty()){
		throw std::runtime_error("403 Forbidden");
	}
	if (!isdirectory(location.uploadpath)){
		throw std::runtime_error("403 Forbidden");
	}
	else{
	std::string filename;
	std::cout << requestbody << " not okay?" << std::endl;
	filename = extractfilename(requestbody);
	std::cout << filename << " = filename\n";
	if (filename == "")
		throw std::runtime_error("400 Bad Request9");
    std::string filePath = location.uploadpath + "/" + filename;
    std::ofstream outputFile(filePath.c_str(), std::ios::binary);
    if (!outputFile.is_open()) {
        throw std::runtime_error("500 Internal Server Error");
    }
    outputFile << requestbody;
    outputFile.close();
    std::string response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html\r\n"
                       "\r\n"
                       "<h1>File uploaded successfully!</h1>";
	std::cout << response << std::endl;
    send(client_socket_fd, response.c_str(), response.length(), 0);
	}
}

void Client::handledelete(std::string& rqdir, std::string& rquri, const Routes& location, const std::string& route){
	(void)location;
	(void)route;
	std::string path = rqdir + rquri;
	std::cout << path << " del test" << std::endl;
		if (!resourceexists(path)) {
		throw std::runtime_error("404 Not Found del1");
	}

	if (isdirectory(path)) {
		if (deletedirectory(path.c_str())) {
			std::stringstream responseStream;
			responseStream << "HTTP/1.1 204 No Content\r\n";
			responseStream << "Content-Length: 0\r\n";
			responseStream << "\r\n";
			std::string response = responseStream.str();
			send(client_socket_fd, response.c_str(), response.length(), 0);
    	} else {
			throw std::runtime_error("500 Internal Server Error");
    	}
	} else if (remove(path.c_str()) != 0) {
        throw std::runtime_error("500 Internal Server Error");
	} else {
        std::stringstream responseStream;
		responseStream << "HTTP/1.1 204 No Content\r\n";
		responseStream << "Content-Length: 0\r\n";
		responseStream << "\r\n";
		std::string response = responseStream.str();
		send(client_socket_fd, response.c_str(), response.length(), 0);
	}
}

void Client::sendget(std::string rquri){
   (void)rquri;
}

void Client::parseRoute(int exit, std::string requestdirectory){
	std::cout << requestbody << " here?" << std::endl;
	std::cout << requestURI << " = uri\n";
	std::cout << requestmethod << " = requestmethod\n";
	if (exit >= 10)
		throw std::runtime_error("508 Loop Detected");
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
		std::cout << requestdirectory + requestURI << " path" << std::endl;
		if (requestbody.length() > request_body_size)
			throw std::runtime_error("413 Payload Too Large");
		if (requestmethod.size()){
			if (requestmethod == "GET"){
				handleget(requestdirectory, requestURI, route->second, route->first);
				return;
			}
			else if (requestmethod == "POST"){
				handlepost(requestdirectory, requestURI, route->second, route->first);
				return;
			}
			else if (requestmethod == "DELETE"){
				std::cout << "here delete\n";
				handledelete(requestdirectory, requestURI, route->second, route->first);
				return;
			}
		}
	}
	throw std::runtime_error("404 Not Found7");
}

int Client::handleRequest(){
	sent = true;
	previous_request_time = std::time(NULL);
	try
	{
		if (!multiparts.empty()){
			for (size_t i = 0; i < multiparts.size(); i++){
				request = multiparts[i];
				std::cout << request << std::endl;
				std::cout << " = made it handle nr: "  << i << std::endl;
				std::string dir = target_server.getdirectory();
				parseRoutemulti(0, dir, multibody[i], i);
			}
		}
		else{
		parseRequest();
		std::string dir = target_server.getdirectory();
		parseRoute(0, dir);
		}
	}
	catch(const std::exception& e)
	{
		sendErrorResponse(e.what());
		return (1);
	}
	request.clear();
	requestbody.clear();
	if (!multiparts.empty())
		multiparts.clear();
	return (1);
}
