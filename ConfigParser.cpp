#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename){
    parseConfigFile(filename);
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

void ConfigParser::parseConfigFile(const std::string& filename) {
    std::ifstream configFile(filename.c_str());
    if (!configFile.is_open()) {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return;
    }
    std::string line;
    ServerConfig currentServer;
    bool inErrorPagesBlock = false; // Flag to track if currently in an error pages block
    bool inLimitsBlock = false;     // Flag to track if currently in a limits block
    bool inRoutesBlock = false;     // Flag to track if currently in a routes block
    bool inServerBlock = false;     // Flag to track if currently in a server block
    bool serverBlockParsed = false; // Flag to track if server block has been parsed
    std::string nestedBlockContent; // Buffer to store nested block content
    std::string routesBlockContent; // Store routes block content as string
    int routesBlockNestingLevel = 0; // Track the nesting level of routes block
    while (std::getline(configFile, line)) {
        line = trim(line);
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        if (inErrorPagesBlock || inLimitsBlock || inRoutesBlock) {
            if (line == "}") {
                // End of the nested block
                if (inErrorPagesBlock) {
                    parseNestedBlock(nestedBlockContent, currentServer.errorPages);
                    inErrorPagesBlock = false;
                } else if (inLimitsBlock) {
                    parseNestedBlock(nestedBlockContent, currentServer.limits);
                    inLimitsBlock = false;
                } else if (inRoutesBlock) {
                    if (routesBlockNestingLevel == 0) {
                        // End of the routes block, send the entire content to parseRoutesBlock
                        parseRoutesBlock(currentServer, routesBlockContent);
                        inRoutesBlock = false;
                        routesBlockContent.clear(); // Clear the buffer
                    } else {
                        // Append line to routes block content
                        routesBlockContent += line + '\n';
                    }
                }
                nestedBlockContent.clear();
            } else {
                // Append line to nested block content
                nestedBlockContent += line + '\n';
            }
        } else if (inServerBlock) {
            if (line == "}") {
                // End of the server block
                if (!serverBlockParsed) {
                    // Push back the current server configuration if not already done
                    configData.push_back(currentServer);
                    serverBlockParsed = true;
                }
                // Reset server configuration for the next server block
                currentServer = ServerConfig();
                inServerBlock = false;
            } else {
                // Parse server directives inside the server block
                std::istringstream iss(line);
                std::string directive, value;
                iss >> directive >> value;
                if (directive == "port") {
                    currentServer.serverConfig["port"] = value;
                } else if (directive == "host") {
                    currentServer.serverConfig["host"] = value;
                } else if (directive == "server_name") {
                    currentServer.serverConfig["server_name"] = value;
                } else if (directive == "error_pages") {
                    inErrorPagesBlock = true;
                    nestedBlockContent += line + '\n'; // Add the current line to the buffer
                } else if (directive == "limits") {
                    inLimitsBlock = true;
                    nestedBlockContent += line + '\n';
                } else if (directive == "routes") {
                    inRoutesBlock = true;
                    routesBlockNestingLevel = 0; // Reset nesting level
                }
            }
        } else {
            // Not inside a server block
            if (line == "server {") {
                // Start of a new server block, create a new ServerConfig object
                currentServer = ServerConfig();
                inServerBlock = true;
                serverBlockParsed = false; // Reset the flag for the new server block
            }
        }
        if (inRoutesBlock) {
            if (line == "}")
                routesBlockNestingLevel--;
            routesBlockContent += line + '\n';
            // Check if the line indicates the start of a nested block
            if (inRoutesBlock && line.find("route /") == 0) {
                // Increment nesting level
                routesBlockNestingLevel++;
            }
        }
    }
    // Push back the last server configuration if not already done
    if (!serverBlockParsed && !currentServer.serverConfig.empty()) {
        configData.push_back(currentServer);
    }
    // Close the file
    configFile.close();
}

void ConfigParser::parseNestedBlock(const std::string& blockContent, std::map<std::string, std::string>& block){
    std::istringstream iss(blockContent);
    std::string line;
    std::getline(iss, line);
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

void ConfigParser::parseRoutesBlock(ServerConfig& currentServer, const std::string& nestedBlockContent) {
    std::istringstream iss(nestedBlockContent);
    std::string line;
    std::string currentRoute;
    bool inRouteBlock = false; // Flag to track if currently parsing a route block

    // Split the nestedBlockContent into lines
    std::vector<std::string> lines;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    // Iterate over each line
    for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
        const std::string& currentLine = *it;
        std::istringstream lineStream(currentLine);
        std::string token;
        lineStream >> token;

        // Skip empty lines
        if (token.empty()) {
            continue;
        }

        if (token == "route") {
            // Start of a new route block
            currentRoute = currentLine.substr(6); // Extract route path
            currentRoute = trim(currentRoute);
            currentRoute = currentRoute.substr(0, currentRoute.size() - 1); // Remove trailing '{'
            inRouteBlock = true;
        } else if (token == "}") {
            // End of the current route block
            inRouteBlock = false;
            currentRoute.clear(); // Reset currentRoute
        } else if (inRouteBlock) {
            // Parse directive-value pairs within the route block
            std::string directive;
            lineStream >> directive;
            std::string value;
            std::getline(lineStream, value); // Get the rest of the line as value
            value = trim(value);
            currentServer.routes[currentRoute][directive].insert(value);
        }
    }
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
