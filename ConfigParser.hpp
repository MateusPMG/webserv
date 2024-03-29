#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <cstdlib>

class ServerConfig {
public:
	std::map<std::string, std::string> serverConfig;
	std::map<std::string, std::map<std::string, std::set<std::string> > > routes;
};

// this class will parse the config file and store the server directives
// in a vector of serverconfig objects
// each member of the vector is a different server with its own config
// no need for a destructor declaration since we dont need special cleanup
class ConfigParser {
private:
	int parseConfigFile(const std::string& filename);
	int parseNestedBlock(const std::string& blockContent, std::map<std::string, std::string>& block);
	int parseRoutesBlock(ServerConfig& currentServer, const std::string& nestedBlockContent);
public:
	std::vector<ServerConfig> configData;
	ConfigParser(const std::string& filename);
	int getNumServers() const;
	std::map<std::string, std::string> getServerConfig(int serverIndex);
	std::map<std::string, std::string> getErrorPages(int serverIndex);
	std::map<std::string, std::map<std::string, std::set<std::string> > > getRoutes();
	void printConfigData();
	void checkdata();
};


//important note: i accept anything as long as it has the min required but must always have {}
