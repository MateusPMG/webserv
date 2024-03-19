#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename){
    if (parseConfigFile(filename))
		throw std::runtime_error("");
}

std::string trim(const std::string& str){
    //Find the first non-whitespace character
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    //If the string is all whitespace, return an empty string
    if (start == std::string::npos) {
        return "";
    }
    //Find the last non-whitespace character
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    //Extract the trimmed substring
    return str.substr(start, end - start + 1);
}

int ConfigParser::parseConfigFile(const std::string& filename) {
    std::ifstream configFile(filename.c_str());
    if (!configFile.is_open()) {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return(1);
    }
	 // Check if the file is empty
    if (configFile.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "Error-> " << filename << " is empty" << std::endl;
        configFile.close();
        return (1); 
    }
    std::string line;
    ServerConfig currentServer;
    bool inErrorPagesBlock = false; //Flag to track if currently in an error pages block
    bool inRoutesBlock = false;     //Flag to track if currently in a routes block
    bool inServerBlock = false;     //Flag to track if currently in a server block
    bool serverBlockParsed = false; //Flag to track if server block has been parsed
    std::string nestedBlockContent; //Buffer to store nested block content
    std::string routesBlockContent; //Store routes block content as string
    int routesBlockNestingLevel = 0; //Track the nesting level of routes block
    while (std::getline(configFile, line)) {
        line = trim(line);
        //Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        if (inErrorPagesBlock || inRoutesBlock) {
            if (line == "}") {
                //End of the nested block
                if (inErrorPagesBlock) {
                    parseNestedBlock(nestedBlockContent, currentServer.errorPages);
                    inErrorPagesBlock = false;
                }
                else if (inRoutesBlock) {
                    if (routesBlockNestingLevel == 0) {
                        //End of the routes block, send the entire content to parseRoutesBlock
                        if (parseRoutesBlock(currentServer, routesBlockContent))
							return (1);
                        inRoutesBlock = false;
                        routesBlockContent.clear(); //Clear the buffer
                    } else {
                        //Append line to routes block content
                        routesBlockContent += line + '\n';
                    }
                }
                nestedBlockContent.clear();
            } else {
                //Append line to nested block content
                nestedBlockContent += line + '\n';
            }
        } else if (inServerBlock) {
            if (line == "}") {
                //End of the server block
                if (!serverBlockParsed) {
                    //Push back the current server configuration if not already done
                    configData.push_back(currentServer);
                    serverBlockParsed = true;
                }
                //Reset server configuration for the next server block
                currentServer = ServerConfig();
                inServerBlock = false;
            } else {
                //Parse server directives inside the server block
                std::istringstream iss(line);
                std::string directive, value;
                iss >> directive >> value;
                //remove trailing semicolon
                if (!value.empty() && value[value.length()-1] == ';') {
                    value.erase(value.length()-1);
                }
				//check empty values
				if (value.empty()){
					std::cerr << "Error: empty value for directive" << std::endl;
        			return(1);
				}
                if (directive == "port") {
					int intValue = atoi(value.c_str());
					if (intValue > 65535 || intValue < 1024 || (value.find_first_not_of("0123456789") != std::string::npos))
					{
						std::cerr << "Error: Invalid port number (valid 1024 - 65535)" << std::endl;
        				return(1);
					}
                    currentServer.serverConfig["port"] = value;
                } else if (directive == "host") {
                    currentServer.serverConfig["host"] = value;
                } else if (directive == "server_name") {
                    currentServer.serverConfig["server_name"] = value;
                } else if (directive == "index") {
                    currentServer.serverConfig["index"] = value;
                } else if (directive == "client_body_size") {
                    currentServer.serverConfig["client_body_size"] = value;
                } else if (directive == "error_pages") {
                    inErrorPagesBlock = true;
                    nestedBlockContent += line + '\n'; //Add the current line to the buffer
                } else if (directive == "routes") {
                    inRoutesBlock = true;
                    routesBlockNestingLevel = 0; //Reset nesting level
                }
            }
        } else {
            //Not inside a server block
            if (line == "server {") {
                //Start of a new server block, create a new ServerConfig object
                currentServer = ServerConfig();
                inServerBlock = true;
                serverBlockParsed = false; //Reset the flag for the new server block
            }
        }
        if (inRoutesBlock) {
            if (line == "}")
                routesBlockNestingLevel--;
            routesBlockContent += line + '\n';
            //Check if the line indicates the start of a nested block
            if (inRoutesBlock && line.find("route /") == 0) {
                //Increment nesting level
                routesBlockNestingLevel++;
            }
        }
    }
    //Push back the last server configuration if not already done
    if (!serverBlockParsed && !currentServer.serverConfig.empty()) {
        configData.push_back(currentServer);
    }
    //Close the file
    configFile.close();
	//Check if no server blocks were found
	if (configData.empty()) {
		std::cerr << "Error: no server blocks found" << std::endl;
        return(1);
	}
	return (0);
}

int ConfigParser::parseNestedBlock(const std::string& blockContent, std::map<std::string, std::string>& block){
    std::istringstream iss(blockContent);
    std::string line;
    std::getline(iss, line);
    while (std::getline(iss, line) && line != "}") {
        //Trim leading and trailing whitespace from the line
        line = trim(line);
        //Parse directive and value
        std::istringstream innerIss(line);
        std::string directive, value;
        innerIss >> directive >> value;
        //remove trailing semicolon
        if (!value.empty() && value[value.length()-1] == ';') {
            value.erase(value.length()-1);
        }
		if (value.empty()){
			std::cerr << "Error: empty value for directive" << std::endl;
        	return(1);
		}		
        block[directive] = value;
    }
	return (0);
}

int ConfigParser::parseRoutesBlock(ServerConfig& currentServer, const std::string& nestedBlockContent) {
    std::istringstream iss(nestedBlockContent);
    std::string line;
    std::string currentRoute;

    while (std::getline(iss, line)) {
        line = trim(line);

        if (line.empty()) {
            continue;
        }

        if (line.substr(0, 6) == "route ") {
            //Start of a new route block
            currentRoute = line.substr(6);
            currentRoute = trim(currentRoute);
            currentRoute = currentRoute.substr(0, currentRoute.size() - 1); //Remove trailing '{'
            currentRoute = trim(currentRoute);
        } else if (line == "}") {
            //End of the current route block
            currentRoute.clear();
        } else if (!currentRoute.empty()) {
            //Parse directive-value pairs within the route block
            size_t pos = line.find(' ');
            if (pos != std::string::npos) {
                std::string directive = line.substr(0, pos);
                std::string values = line.substr(pos + 1);
                directive = trim(directive);
                values = trim(values);
                std::istringstream valuesStream(values);
                std::string value;
                std::set<std::string> valueSet;
                while (valuesStream >> value) {
                    //remove trailing semicolon
                    if (!value.empty() && value[value.length()-1] == ';') {
                    value.erase(value.length()-1);
                    }
					if (value.empty()){
						std::cerr << "Error: empty value for directive" << std::endl;
        				return(1);
					}
                    valueSet.insert(value);
                }

                currentServer.routes[currentRoute][directive] = valueSet;
            }
        }
    }
	return (0);
}
    
std::map<std::string, std::string> ConfigParser::getServerConfig(int serverIndex){
    return configData[serverIndex].serverConfig;
}

std::map<std::string, std::string> ConfigParser::getErrorPages(int serverIndex){
    return configData[serverIndex].errorPages;
}

void ConfigParser::printConfigData() {
    std::cout << "Printing Config Data:" << std::endl;
    for (size_t i = 0; i < configData.size(); ++i) {
        std::cout << "Server " << i + 1 << ":" << std::endl;
        const ServerConfig& server = configData[i];
        
        // Print server config
        std::cout << "Server Config:" << std::endl;
        for (std::map<std::string, std::string>::const_iterator it = server.serverConfig.begin(); it != server.serverConfig.end(); ++it) {
            std::cout << it->first << " : " << it->second << "|" << std::endl;
        }
        
        // Print error pages
        std::cout << "Error Pages:" << std::endl;
        for (std::map<std::string, std::string>::const_iterator it = server.errorPages.begin(); it != server.errorPages.end(); ++it) {
            std::cout << it->first << " : " << it->second << "|" << std::endl;
        }
         
        // Print routes
        std::cout << "Routes:" << std::endl;
        for (std::map<std::string, std::map<std::string, std::set<std::string> > >::const_iterator it = server.routes.begin(); it != server.routes.end(); ++it) {
            const std::string& route = it->first;
            const std::map<std::string, std::set<std::string> >& routeData = it->second;
            std::cout << "Route: " << route << "|" << std::endl;
            for (std::map<std::string, std::set<std::string> >::const_iterator jt = routeData.begin(); jt != routeData.end(); ++jt) {
                const std::string& directive = jt->first;
                const std::set<std::string>& values = jt->second;
                std::cout << "    " << directive << " : ";
                for (std::set<std::string>::const_iterator vt = values.begin(); vt != values.end(); ++vt) {
                    std::cout << *vt << "|" << " ";
                }
                std::cout << std::endl;
            }
        }
        
        std::cout << std::endl;
    }
}
