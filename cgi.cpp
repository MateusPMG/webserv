#include "Client.hpp"

void Client::cgiget(){
	std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/plain\r\n\r\n";
    // Execute the CGI script and get its output
    FILE* pipe = popen("/path/to/your/cgi/script.py", "r");
    if (pipe) {
        char temp_buffer[128];
        while (!feof(pipe)) {
            if (fgets(temp_buffer, 128, pipe) != NULL)
                response += temp_buffer;
        }
        pclose(pipe);
    }
	else
		return;
    // Send the response back to the client
    if (send(client_socket_fd, response.c_str(), response.size(), 0) <= 0)
		throw (std::runtime_error("500 Internal Server Error"));
	return;
}