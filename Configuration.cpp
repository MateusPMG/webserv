#include "Configuration.hpp"

ConfigParser::ConfigParser(const std::string& filename){
    parseConfigFile(filename);
}

void ConfigParser::parseConfigFile(const std::string& filename){
    std::ifstream configFile(filename);
    if (!configFile.is_open()) {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return;
    }
    std::string line;
    ServerConfig currentServer;
    while (std::getline(configFile, line)) {
        // Trim leading and trailing whitespace from the line
        line = trim(line);
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        // Check if the line contains a server block
        if (line == "server {") {
            // Start of a new server block, create a new ServerConfig object
            currentServer = ServerConfig();
            continue;
        } else if (line == "}") {
            // End of the current server block, add it to the vector
            configData.push_back(currentServer);
            continue;
        }
        // Parse server directives
        std::istringstream iss(line);
        std::string directive, value;
        iss >> directive;
        if (directive == "port") {
            iss >> value;
            currentServer.serverConfig["port"] = value;
        } else if (directive == "host") {
            iss >> value;
            currentServer.serverConfig["host"] = value;
        } else if (directive == "server_name") {
            iss >> value;
            currentServer.serverConfig["server_name"] = value;
        } else if (directive == "error_pages") {
            parseNestedBlock(iss, currentServer.errorPages);
        } else if (directive == "limits") {
            parseNestedBlock(iss, currentServer.limits);
        } else if (directive == "routes") {
            parseRoutesBlock(currentServer, iss);
        }
    }
    // Close the file
    configFile.close();
}

void ConfigParser::parseNestedBlock(std::istringstream& iss, std::map<std::string, std::string>& block){
    std::string line;
    while (std::getline(iss, line) && line != "}") {
        // Trim leading and trailing whitespace from the line
        line = trim(line);
        // Parse directive and value
        std::istringstream innerIss(line);
        std::string directive, value;
        innerIss >> directive >> value;
        block[directive] = value;
    }
}

void ConfigParser::parseRoutesBlock(ServerConfig& currentServer, std::istringstream& iss) {
    std::string line;
    while (std::getline(iss, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue; // Skip empty lines and comments
        } else if (line == "}") {
            return; // End of the "routes" block
        } else if (line.substr(0, 6) == "route ") {
            std::istringstream innerIss(line);
            std::string directive, value;
            innerIss >> directive >> value;
            currentServer.routes[value]; // Create a new entry for the route in routes map
            parseNestedBlock(iss, currentServer.routes[value]); // Parse nested block
        }
    }
}

std::string trim(const std::string& str){
    // Find the first non-whitespace character
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    // If the string is all whitespace, return an empty string
    if (start == std::string::npos) {
        return "";
    }
    // Find the last non-whitespace character
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    // Extract the trimmed substring
    return str.substr(start, end - start + 1);
}

std::map<std::string, std::string> ConfigParser::getServerConfig(int serverIndex){
    return configData[serverIndex].serverConfig;
}

std::map<std::string, std::string> ConfigParser::getErrorPages(int serverIndex){
    return configData[serverIndex].errorPages;
}

std::map<std::string, std::string> ConfigParser::getLimits(int serverIndex){
    return configData[serverIndex].limits;
}