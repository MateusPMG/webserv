#include "webserv.hpp"
#include "ConfigParser.hpp"


int main(int ac, char **av){

    if (ac == 1 || ac == 2){
        std::string configPath;
        configPath = (ac == 1 ? "conf_files/default_config.txt" : av[1]);
        ConfigParser parse(configPath);
    }
    return (0);
}