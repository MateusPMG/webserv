#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>

class ServerConfig {
public:
    std::map<std::string, std::string> serverConfig;
    std::map<std::string, std::string> errorPages;
    std::map<std::string, std::string> limits;
    std::map<std::string, std::string> routes;
};
// this class will parse the config file and store the server directives
// no need for a destructor declaration since we dont need special cleanup
// each member of the vector is a different server with its own config
class ConfigParser {
private:
    std::vector<ServerConfig> configData;
    void parseConfigFile(const std::string& filename);
	void parseNestedBlock(std::istringstream& iss, std::map<std::string, std::string>& block);
public:
    ConfigParser(const std::string& filename);
    int getNumServers() const;
    std::string getDirective(int serverIndex, const std::string& directiveName);
    std::string getDirectory(int serverIndex, const std::string& route);
    std::string getIndex(int serverIndex, const std::string& route);
    std::string getCgi(int serverIndex, const std::string& route);
    std::string getUploadDir(int serverIndex, const std::string& route);
    std::map<std::string, std::string> getServerConfig(int serverIndex);
    std::map<std::string, std::string> getErrorPages(int serverIndex);
    std::map<std::string, std::string> getLimits(int serverIndex);
    std::vector<std::string> getMethods(int serverIndex, const std::string& route);
};