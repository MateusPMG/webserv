#include "Client.hpp"

void Client::cgiget(){
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n\r\n"; // Changed content type to HTML
    
    // Execute the CGI script and get its output
   	FILE* pipe = popen("var/www/cgi-bin/timestamp.py", "r"); // Corrected path to CGI script
    if (pipe) {
        char temp_buffer[128];
        response += "<html><head><title>Timestamp</title></head><body><h1>Timestamp</h1><p>";
        std::string timestamp;
        while (!feof(pipe)) {
            if (fgets(temp_buffer, 128, pipe) != NULL)
                timestamp += temp_buffer;
        }
		std::cout << timestamp << "=timestamp" << std::endl;
        response += timestamp; // Include timestamp in the HTML response
        response += "</p></body></html>";
        pclose(pipe);
    }
    else {
    	throw (std::runtime_error("500 Internal Server Error"));
    }
    if (send(client_socket_fd, response.c_str(), response.size(), 0) <= 0)
    	throw (std::runtime_error("500 Internal Server Error"));
    return;
}
