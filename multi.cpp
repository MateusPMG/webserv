#include "Client.hpp"

std::string Client::extractfilename(const std::string& body) {
	std::cout << body << "  ok?" << std::endl;
    // Find the start index of the filename in the request body
    size_t filenameStart = body.find("filename=\"");
    if (filenameStart == std::string::npos) {
        // Filename not found in the request body
		std::cout << "not foud in body\n";
        return "";
    }
    // Adjust the start index to point to the beginning of the filename
    filenameStart += 10; // Length of "filename=\""
    // Find the end index of the filename in the request body
    size_t filenameEnd = body.find('\"', filenameStart);
	if (filenameEnd == std::string::npos) {
		filenameEnd = body.find('\'', filenameStart); // Search for single quote if double quote not found
		if (filenameEnd == std::string::npos) {
			std::cout << "End quote not found\n";
			return "";
		}
	}
    // Extract the filename substring from the request body
    std::string filename = body.substr(filenameStart, filenameEnd - filenameStart);
    return filename;
}

void Client::parseRoutemulti(int exit, std::string requestdirectory, std::string mrequestbody, size_t i){
	if (exit >= 10)
		throw std::runtime_error("508 Loop Detected");
	size_t pos;
	std::map<std::string, Routes>::const_iterator route;
	for (route = target_server.getroutes().begin(); route != target_server.getroutes().end(); route++){
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
			parseRoutemulti(exit + 1, requestdirectory, requestbody, i);
			return;
		}
		if (mrequestbody.length() > target_server.getclientbodysize())
			throw std::runtime_error("413 Payload Too Large1");
		if (requestmethod.size()){
			if (requestmethod == "POST"){
				handlemultipost(requestdirectory, requestURI, mrequestbody, route->second, route->first, i);
				return;
			}
		}
	}
	throw std::runtime_error("404 Not Found7");
}

void Client::handlemultipost(std::string& rqdir, std::string& rquri ,std::string& rbody, const Routes& location, const std::string& route, size_t i){
	(void)rqdir;
	(void)rquri;
	if(route == "/cgi-bin")
	{
		cgipost(rbody);
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
	filename = extractfilename(multiparts[i]);
	std::cout << filename << " = filename\n";
	if (filename == "")
		throw std::runtime_error("400 Bad Request9");
    std::string filePath = location.uploadpath + "/" + filename;
    std::ofstream outputFile(filePath.c_str(), std::ios::binary);
    if (!outputFile.is_open()) {
        throw std::runtime_error("500 Internal Server Error");
    }
    outputFile << rbody;
    outputFile.close();
    std::string response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html\r\n"
                       "\r\n"
                       "<h1>File uploaded successfully!</h1>";
    send(client_socket_fd, response.c_str(), response.length(), 0);
	}
}