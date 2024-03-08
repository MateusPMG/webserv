#include "Configuration.hpp"

ConfigParser::ConfigParser(const std::string& filename){
    parseConfigFile(filename);
}

void ConfigParser::parseConfigFile(const std::string& filename){
    std::ifstream file(filename.c_str());
    std::string line, section;
    std::map<std::string, std::map<std::string, std::string> > currentServer;

    if (file.is_open()){
        while (std::getline(file, line)){
            if (line.find("server {") != std::string::npos){
                configData.push_back(currentServer);

            }
        }
    }
}