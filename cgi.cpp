#include "Client.hpp"
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#include <sys/time.h>

std::string iToString(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

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

void Client::cgipost(std::string rbody) {
    int pipefd[2];
	int exitcode;
	char* body = strdup(rbody.c_str());
	std::cout << body << "=body" << std::endl;
	char* argv[] = {strdup("usr/bin/python3"), strdup("var/www/cgi-bin/storedata.py"), NULL};
    if (pipe(pipefd) == -1) {
        perror("pipe");
        throw std::runtime_error("500 Internal Server Error");
    }
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Error: Fork failed." << std::endl;
		throw std::runtime_error("500 Internal Server Error");
    } else if (pid == 0) {
        close(pipefd[1]);
		if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        exitcode = execv("/usr/bin/python3", argv);
        std::cerr << "Error: exec failed." << std::endl;
        exit(exitcode);
    } else {
		write(pipefd[1], body, strlen(body));
        close(pipefd[1]);
        struct timeval start, now;
        gettimeofday(&start, NULL);
        const int timeout_usec = 200000;//Timeout in microseconds (0.5 seconds)

        while (true) {
            gettimeofday(&now, NULL);
            int elapsed_usec = (now.tv_sec - start.tv_sec) * 1000000 + (now.tv_usec - start.tv_usec);
            if (elapsed_usec >= timeout_usec) {
                std::cerr << "Error: Script execution timed out." << std::endl;
                kill(pid, SIGKILL);
                throw std::runtime_error("500 Internal Server Error");
            }
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = timeout_usec - elapsed_usec;
            select(0, NULL, NULL, NULL, &tv); 
            int status;
            pid_t wpid = waitpid(pid, &status, WNOHANG);
            if (wpid == -1) {
                perror("waitpid");
                throw std::runtime_error("500 Internal Server Error");
            } else if (wpid > 0) {
                if (WIFEXITED(status)) {
                    std::string p = "Data stored successfully";
                    std::string response = "HTTP/1.1 200 OK\r\n";
                    response += "Content-Type: text/plain\r\n";
                    response += "Content-Length: " + iToString(p.length()) + "\r\n";
                    response += "\r\n";
                    response += p;
                    if (send(client_socket_fd, response.c_str(), response.size(), 0) <= 0)
                        throw std::runtime_error("500 Internal Server Error");
                    return;
                } else if (WIFSIGNALED(status)) {
                    int term_signal = WTERMSIG(status);
                    std::cout << "Child was terminated by signal: " << term_signal << std::endl;
                    throw std::runtime_error("500 Internal Server Error");
                }
            }
		}
	}
}