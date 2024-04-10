#include "ConfigParser.hpp"
#include "Networking.hpp"

int main(int ac, char **av){

	if (ac == 1 || ac == 2){
		try
		{
			std::string configPath;
			configPath = (ac == 1 ? "conf_files/default_config.txt" : av[1]);
			ConfigParser parse(configPath);
			Networking net(parse);
			net.booting();
			net.runservers();
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what();
			return (1);
		}
	}
	return (0);
}
