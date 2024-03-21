#include "Networking.hpp"

Networking::Networking(const ConfigParser& parser){
    for (size_t i = 0; i < parser.configData.size(); ++i){
        this->servers.push_back(Server(parser.configData[i]));
    }
	
}

Networking::~Networking(){}

void Networking::booting(){
	
}