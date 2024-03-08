#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
// this class will parse the config file and store the server directives
// no need for a destructor declaration since we dont need special cleanup
// each member of the vector that contains the maps is a separate server config
// the map will be divided into {section of config file, (directive name, directive value)}
class ConfigParser{
	private:
		std::vector<std::map<std::string, std::map<std::string, std::string> > >configData;
		void parseConfigFile(const std::string& filename);
	public:
		//constructor will call private parser
		ConfigParser(const std::string& filename);
		//getters to grant acess to the configs
		int getNumServers()const;// 3 would mean 3 servers indexed 0,1,2

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