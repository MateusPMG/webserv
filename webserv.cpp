#include "webserv.hpp"


int main(int ac, char **av){

    if (ac == 1 || ac == 2){
        std::string configPath;
        configPath = (ac == 1 ? "conf_files/default.conf" : av[1]);
        
    }

    return (0);
}